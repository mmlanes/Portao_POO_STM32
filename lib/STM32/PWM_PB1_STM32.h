#pragma once

#include <Arduino.h>
#include "Variavel.h"

class PWM_PB1_STM32
{
private:
    inline static float aceleracao_ = 1.0f; // em unidades de dPwm/seg
    inline static uint8_t dpwmAtual_ = 0; // valor atual do dPwm 0 a 100
    inline static uint8_t dpwmAlvo_ = 0; // valor alvo do dPwm 0 a 100
    inline static uint8_t dpwmMaximo_ = 100; // valor alvo do dPwm 0 a 100
    inline static unsigned long tempoInicioRampa_ = millis();
    inline static uint32_t freqHz_ = 500;
    inline static HardwareTimer* timer3_ = nullptr;
    inline static void (*staticLerAdcA0_)(void) = nullptr;
    static void atualizaRampa(void)
    {
        if (dpwmAtual_ == dpwmAlvo_)
            return;

        unsigned long agora = millis();
        float dPwmPorMs = aceleracao_ / 1000.0f;
        float dPwmNecessario = (dpwmAlvo_ > dpwmAtual_) ? (dpwmAlvo_ - dpwmAtual_) : (dpwmAtual_ - dpwmAlvo_);
        float dPwmPossivel = dPwmPorMs * (agora - tempoInicioRampa_);
        if (dPwmPossivel >= dPwmNecessario)
            dpwmAtual_ = dpwmAlvo_;
        else
        {
            if (dpwmAlvo_ > dpwmAtual_)
                dpwmAtual_ += dPwmPossivel;
            else
                dpwmAtual_ -= dPwmPossivel;
        }
        tempoInicioRampa_ = agora;
        definirDpwmImediato(dpwmAtual_);
    }
    static void isrTim3_()
    {
        if (staticLerAdcA0_)
            staticLerAdcA0_();
    }

public:
    static void setupPwmUpDown(uint32_t freqHz_100a10k, void (*staticLerAdcA0)(void) = nullptr)
    {
        // Limites de frequência
        if (freqHz_100a10k < 100 || freqHz_100a10k > 10000)
            return; // Frequência fora do intervalo permitido

        freqHz_ = freqHz_100a10k;
        if (!timer3_) 
            timer3_ = new HardwareTimer(TIM3);
        staticLerAdcA0_ = staticLerAdcA0;
        timer3_->attachInterrupt(1, PWM_PB1_STM32::isrTim3_);
        timer3_->resume();

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

        // Canal 1 modo timing para interrupção
        TIM3->CCMR1 &= ~(TIM_CCMR1_OC1M);
        TIM3->CCMR1 |= TIM_CCMR1_OC1PE;  // preload enable

        TIM3->CCER &= ~TIM_CCER_CC1E;   // saída desabilitada no canal 1

        TIM3->DIER |= TIM_DIER_CC1IE;   // habilita interrupção canal 1

        TIM3->CR1 |= TIM_CR1_ARPE;
        TIM3->EGR |= TIM_EGR_UG;

        NVIC_EnableIRQ(TIM3_IRQn);

        TIM3->CR1 |= TIM_CR1_CEN;

        PWM_PB1_STM32::definirDpwmImediato(0);
    }
    static void definirAceleracao(float dPwmPorSeg)
    {
        if (dPwmPorSeg <= 0)
            dPwmPorSeg = 0.1f;
        aceleracao_ = dPwmPorSeg;
    }
    static float obterAceleracao(void)
    {
        return aceleracao_;
    }
    static void definirDpwmMaximo(uint8_t d0a100)
    {
        if (d0a100 > 100) 
            d0a100 = 100;
        dpwmMaximo_ = d0a100;
        if (dpwmAtual_ > dpwmMaximo_)
            definirDpwmImediato(dpwmMaximo_);
        if (dpwmAlvo_ > dpwmMaximo_)
            definirDpwmRampa(dpwmMaximo_);
    }
    static void definirDpwmImediato(uint8_t d0a100)
    {
        if (d0a100 > dpwmMaximo_) 
            d0a100 = dpwmMaximo_;

        dpwmAtual_ = d0a100;
        uint32_t ccr = (TIM3->ARR + 1) * d0a100 / 100.0;
        TIM3->CCR4 = ccr;
        TIM3->CCR1 = ccr / 2;
    }
    static void definirDpwmRampa(uint8_t d0a100_alvo)
    {
        if (d0a100_alvo > dpwmMaximo_) 
            d0a100_alvo = dpwmMaximo_;
        dpwmAlvo_ = d0a100_alvo;
    }
    static uint8_t obterDpwmAtual(void)
    {
        return dpwmAtual_;
    }
    static void definirFreqHz(uint32_t freqHz_100a10k)
    {
        if (freqHz_100a10k == freqHz_)
            return;

        // Limites de frequência
        if (freqHz_100a10k < 100 || freqHz_100a10k > 10000)
            return; // Frequência fora do intervalo permitido

        freqHz_ = freqHz_100a10k;
        uint32_t timerClk = 72000000; // Frequência do clock do timer (72 MHz)
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

        // Atualizar CCR4 e CCR1 com base no novo ARR
        uint32_t ccr4 = (arr + 1) * dpwmAtual_ / 100.0;
        TIM3->CCR4 = ccr4;
        TIM3->CCR1 = ccr4 / 2;  // meio do HIGH
    }
    static uint32_t obterFreqHz(void)
    {
        return freqHz_;
    }

};

