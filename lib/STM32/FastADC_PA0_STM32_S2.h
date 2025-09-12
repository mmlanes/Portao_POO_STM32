#pragma once

#include <Arduino.h>
#include "Variavel.h"

class FastADC_PA0_STM32_S2
{
private:
    static const uint8_t dimensaoAmostrasMediaMovel_ = 100;
    static uint16_t vetorADC_[dimensaoAmostrasMediaMovel_];
    static uint16_t valorNuloAdc_;
    HardwareTimer* timer2_;

    FastADC_PA0_STM32_S2() 
    { 
        setupAdcPa0Fast();  // executa setup automaticamente
        calcularValorNuloAdc(); // calcula valor nulo automaticamente
        configuraTimer();
    }

    void configuraTimer(uint32_t periodoUs)
    {
        if (!timer2_)
        {
            // Cria o objeto para TIM2
            timer2_ = new HardwareTimer(TIM2);
            // Define a frequência ou o período
            timer2_->setOverflow(periodoUs, MICROSEC_FORMAT);  
            // Anexa a ISR
            //timer2_->attachInterrupt(isrTim2);
            // Inicia o timer
            timer2_->resume();
        }
    }

    void anexarInterrupcao()
    {
        if (timer2_)
            timer2_->attachInterrupt(isrTim2);
    }

    void desanexarInterrupcao()
    {
        if (timer2_)
            timer2_->detachInterrupt();
    }

    static void isrTim2() 
    {
        digitalToggle(LED_BUILTIN);
    }

    static uint16_t leituraAdc_()
    {
        ADC1->CR2 &= ~ADC_CR2_CONT;
        ADC1->CR2 &= ~ADC_CR2_EXTTRIG;

        ADC1->SQR3 = 0;
        ADC1->CR2 |= ADC_CR2_ADON;
        ADC1->CR2 |= ADC_CR2_ADON;
        while (!(ADC1->SR & ADC_SR_EOC));

        return ADC1->DR;
    }

    void calcularValorNuloAdc(uint16_t totalLeituras = 100)
    {
        uint32_t soma = 0;
        for (uint16_t i = 0; i < totalLeituras; i++)
        {
            soma += leituraAdc_();
            delay(1);
        }
        valorNuloAdc_ = (float)soma / (float)totalLeituras;
        Serial2.println("Valor nulo ADC recalibrado: " + String(valorNuloAdc_));
    }

    void setupAdcPa0Fast(void)
    {
        pinMode(PA0, INPUT_ANALOG);  // Configura PA0 como entrada analógica
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;  // Habilita clock do ADC1
        // Reseta ADC e prepara
        ADC1->CR2 = 0;
        ADC1->SQR3 = 0;
        // Liga ADC
        ADC1->CR2 |= ADC_CR2_ADON;
        delay(1);  // Espera estabilizar
        // Calibração recomendada
        ADC1->CR2 |= ADC_CR2_RSTCAL;
        while (ADC1->CR2 & ADC_CR2_RSTCAL);
        ADC1->CR2 |= ADC_CR2_CAL;
        while (ADC1->CR2 & ADC_CR2_CAL);
    }

    void leituraADCBuferizada(uint32_t periodoAmostragemUs, uint16_t periodoTotalUs)
    {
        static const uint8_t totalAmostras = 100;
        static uint16_t bufferLeituras[totalAmostras] = {0};
        static uint8_t indiceBuffer = 0;

        // Leitura ADC
        uint16_t leitura = leituraAdc_();
        // Desloca todos os elementos do vetor para a esquerda
        for (uint8_t i = 1; i < dimensaoAmostrasMediaMovel_; i++)
            vetorADC_[i - 1] = vetorADC_[i];
        // Adiciona a nova leitura no final do vetor
        vetorADC_[dimensaoAmostrasMediaMovel_ - 1] = leitura;
    }
public:
    // Deleta cópia e atribuição
    FastADC_PA0_STM32_S2(const FastADC_PA0_STM32_S2&) = delete;
    FastADC_PA0_STM32_S2& operator=(const FastADC_PA0_STM32_S2&) = delete;
        // 🔹 Método para obter a única instância
    static FastADC_PA0_STM32_S2& getInstance()
    {
        static FastADC_PA0_STM32_S2 instance;
        return instance;
    }

    // Função pública para obter a instância
    static FastADC_PA0_STM32_S2& getInstance(uint16_t periodoUs)
    {
        static PWM_PB1_STM32_S instance(freqHz, dpwmAtual, dpwmMaximo, aceleracao, staticLerAdcA0);
        return instance;
    }

    // // Chamadas posteriores: usa instância existente
    static PWM_PB1_STM32_S& getInstance() { return *instance_; }

    static void leituraSincronizadaPWM(void)
    {
        static uint8_t indiceCircular_ = 0;
        // Leitura ADC
        uint16_t leitura = leituraAdc_();
        // Sobrescreve o elemento mais antigo no vetor
        vetorADC_[indiceCircular_] = leitura;
        // Atualiza o índice circular
        indiceCircular_ = (indiceCircular_ + 1) % dimensaoAmostrasMediaMovel_;
    }

    int16_t obterGrandezaMedia(void)
    {
        uint32_t soma = 0;
        for (uint8_t i = 0; i < dimensaoAmostrasMediaMovel_; i++)
            soma += vetorADC_[i];
        uint16_t media = soma / dimensaoAmostrasMediaMovel_;
        media = media - valorNuloAdc_; // Remove valor nulo
        return media;
    }

    uint16_t obterValorNuloAdc(void) { return valorNuloAdc_; }

    void recalibrarValorNuloAdc(uint16_t totalLeituras = 100) { calcularValorNuloAdc(totalLeituras); }
};

uint16_t FastADC_PA0_STM32_S::vetorADC_[dimensaoAmostrasMediaMovel_] = {0};
uint16_t FastADC_PA0_STM32_S::valorNuloAdc_ = 0;