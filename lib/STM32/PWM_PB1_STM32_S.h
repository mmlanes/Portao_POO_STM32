// Fechar (dobrar) o bloco de código atual:
// Ctrl + Shift + [
// Abrir (expandir) o bloco de código atual:
// Ctrl + Shift + ]
// Fechar (dobrar) todos os blocos de código:
// Ctrl + K, Ctrl + 0
// Abrir (expandir) todos os blocos de código:
// Ctrl + K, Ctrl + J

#pragma once
#include <Arduino.h>
#include "Variavel.h"

class PWM_PB1_STM32_S
{
private:
    // Atributos do PWM
    Variavel<uint16_t>& freqHz_;
    Variavel<uint8_t>& dpwmAtual_;
    Variavel<uint8_t>& dpwmMaximo_;
    Variavel<float>& aceleracao_; // uma porcentagem por segundo
    uint8_t dpwmAlvo_;
    unsigned long tempoInicioRampa_;
    static PWM_PB1_STM32_S* instance_; // Ponteiro estático para a instância única Singleton

    // Construtor privado
    PWM_PB1_STM32_S(Variavel<uint16_t>& freqHz, 
                    Variavel<uint8_t>& dpwmAtual,
                    Variavel<uint8_t>& dpwmMaximo, 
                    Variavel<float>& aceleracao, 
                    void (*staticLerAdcA0)(void) = nullptr)
        :   freqHz_(freqHz), 
            dpwmAtual_(dpwmAtual), 
            dpwmMaximo_(dpwmMaximo),
            aceleracao_(aceleracao),  
            dpwmAlvo_(0), 
            tempoInicioRampa_(millis())
    {
        instance_ = this;
        defineFrequencia(freqHz.obterValor());
    }

public:
     // Deleta cópia e atribuição
    PWM_PB1_STM32_S(const PWM_PB1_STM32_S&) = delete;
    PWM_PB1_STM32_S& operator=(const PWM_PB1_STM32_S&) = delete;

    // Função pública para obter a instância
    static PWM_PB1_STM32_S& getInstance(Variavel<uint16_t>& freqHz, 
                                        Variavel<uint8_t>& dpwmAtual, 
                                        Variavel<uint8_t>& dpwmMaximo, 
                                        Variavel<float>& aceleracao, 
                                        void (*staticLerAdcA0)(void) = nullptr)
    {
        static PWM_PB1_STM32_S instance(freqHz, dpwmAtual, dpwmMaximo, aceleracao, staticLerAdcA0);
        return instance;
    }

    // // Chamadas posteriores: usa instância existente
    static PWM_PB1_STM32_S& getInstance() { return *instance_; }
    // Métodos de controle do PWM (não estáticos agora)
 
    // Método para definir a frequência
    void defineFrequencia(uint32_t freqHz_100a10k)
    {
        static uint16_t lastFreq = 0;

        if (freqHz_100a10k == lastFreq)
            return; // Mesma frequência, nada a fazer
        
        // Limites de frequência
        if (freqHz_100a10k < 100 || freqHz_100a10k > 10000)
            return; // Frequência fora do intervalo permitido
        if (freqHz_100a10k == lastFreq)
            return; // Mesma frequência, nada a fazer
        lastFreq = freqHz_100a10k;
        Serial2.println("Definindo freq: " + String(freqHz_100a10k));
        freqHz_.definirValor(freqHz_100a10k);
        
        float d0a100 = 0;
        RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

        // PB1 - TIM3_CH4 - saída alternativa push-pull 50MHz
        GPIOB->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1);
        GPIOB->CRL |= (GPIO_CRL_MODE1_1 | GPIO_CRL_MODE1_0);
        GPIOB->CRL |= GPIO_CRL_CNF1_1;

        uint32_t timerClk = 72000000; // Frequência do clock do timer (72 MHz)

        // Calcular prescaler e ARR dinamicamente
        uint32_t psc = 0;
        uint32_t arr = 0;

        // Ajustar prescaler para manter ARR dentro do limite de 16 bits
        for (psc = 0; psc <= 0xFFFF; ++psc) {
            arr = (timerClk / ((psc + 1) * freqHz_100a10k)) - 1;
            if (arr <= 0xFFFF) {
                break; // Encontrou um valor válido para PSC e ARR
            }
        }
        TIM3->PSC = psc;  // Configurar prescaler
        TIM3->ARR = arr;  // Configurar ARR

        uint32_t ccr4 = (arr + 1) * d0a100 / 100.0;
        TIM3->CCR4 = ccr4;
        TIM3->CCR1 = ccr4 / 2;  // meio do HIGH

        // PWM canal 4 modo 1
        TIM3->CCMR2 &= ~TIM_CCMR2_OC4M;
        TIM3->CCMR2 |= (6 << TIM_CCMR2_OC4M_Pos);
        TIM3->CCMR2 |= TIM_CCMR2_OC4PE;
        TIM3->CCER |= TIM_CCER_CC4E;

        definirDpwmImediato(0);
    }

    void definirDpwmImediato(uint8_t d0a100)
    {
        if (d0a100 > dpwmMaximo_.obterValor()) 
            d0a100 = dpwmMaximo_.obterValor();
        dpwmAtual_.definirValor(d0a100);
        uint32_t ccr = (TIM3->ARR + 1) * d0a100 / 100.0;
        TIM3->CCR4 = ccr;
        TIM3->CCR1 = ccr / 2;
    }

    void definirDpwmRampa(uint8_t d0a100_alvo)
    {
        if (d0a100_alvo > dpwmMaximo_.obterValor()) 
            d0a100_alvo = dpwmMaximo_.obterValor();
        dpwmAlvo_ = d0a100_alvo;
    }

    uint8_t obterDpwmAtual() const { return dpwmAtual_.obterValor(); }

    void definirAceleracao(float dPwmPorSeg) { aceleracao_.definirValor( (dPwmPorSeg <= 0) ? 0.1f : dPwmPorSeg ); }
    
    float obterAceleracao(void) {return aceleracao_.obterValor();}

    void definirDpwmMaximo(uint8_t d0a100)
    {
        if (d0a100 > 100) 
            d0a100 = 100;
        dpwmMaximo_.definirValor(d0a100);
        if (dpwmAtual_.obterValor() > dpwmMaximo_.obterValor())
            definirDpwmImediato(dpwmMaximo_.obterValor());
        if (dpwmAlvo_ > dpwmMaximo_.obterValor())
            definirDpwmRampa(dpwmMaximo_.obterValor());
    }

    uint8_t obterDpwmMaximo() const { return dpwmMaximo_.obterValor(); }

    uint32_t obterFreqHz(void) {return freqHz_.obterValor();}

    void atualizaRampa(void)
    {
        if (dpwmAtual_.obterValor() == dpwmAlvo_)
        {
            tempoInicioRampa_ = millis();
            return;
        }
        unsigned long agora = millis();
        float dPwmPorMs = aceleracao_.obterValor() / 1000.0f;
        float dPwmNecessario = (dpwmAlvo_ > dpwmAtual_.obterValor()) ? (dpwmAlvo_ - dpwmAtual_.obterValor()) : (dpwmAtual_.obterValor() - dpwmAlvo_);
        float dPwmPossivel = dPwmPorMs * (agora - tempoInicioRampa_);
        if (dPwmPossivel < 1)
            return; // nada a fazer
        if (dPwmPossivel >= dPwmNecessario)
            dpwmAtual_.definirValor(dpwmAlvo_);
        else
        {
            if (dpwmAlvo_ > dpwmAtual_.obterValor())
                dpwmAtual_.definirValor(dpwmAtual_.obterValor() + dPwmPossivel);
            else
            {
                if (dpwmAtual_.obterValor() < dPwmPossivel)
                    dpwmAtual_.definirValor(0);
                else
                    dpwmAtual_.definirValor(dpwmAtual_.obterValor() - dPwmPossivel);
            }
        }
        //Serial2.println(String(tempoInicioRampa_) + "/" + String(agora));
        tempoInicioRampa_ = agora;
        definirDpwmImediato(dpwmAtual_.obterValor());
    }

    void monitorar(void)
    {
        // Chamar periodicamente em loop()
        atualizaRampa();
        defineFrequencia(freqHz_.obterValor());
    }
};

PWM_PB1_STM32_S* PWM_PB1_STM32_S::instance_ = nullptr;