#pragma once

#include <Arduino.h>

class FastADC_PA0_STM32
{

private:
    inline static float kAjuste_ = 1.7e-3f; // Valor real = Valor lido * kAjuste
    inline static const uint8_t dimensaoAmostrasMediaMovel_ = 100;
    inline static uint16_t vetorADC_[dimensaoAmostrasMediaMovel_] = {0};
    inline static uint16_t valorNuloAdc_ = 0;
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

public:
    static void setupAdcPa0Fast(void)
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
    static float obterGrandezaMedia(void)
    {
        uint32_t soma = 0;
        for (uint8_t i = 0; i < dimensaoAmostrasMediaMovel_; i++)
        {
            soma += vetorADC_[i];
        }
        float media = (float)soma / (float)dimensaoAmostrasMediaMovel_;
        // Converte para valor real
        media = (media - (float)valorNuloAdc_) * kAjuste_;
        return media;
    }
    static uint16_t obterValorNuloAdc(void)
    {
        return valorNuloAdc_;
    }
    static void calcularValorNuloAdc(uint16_t totalLeituras = 100)
    {
        uint32_t soma = 0;
        for (uint16_t i = 0; i < totalLeituras; i++)
        {
            soma += leituraAdc_();
            delay(1);
        }
        valorNuloAdc_ = (float)soma / (float)totalLeituras;
    }
    static void definirKValorReal(float kAjuste = 1.7e-3)
    {
        kAjuste_ = kAjuste;
    }
    static float obterKValorReal(void)
    {
        return kAjuste_;
    }

};

