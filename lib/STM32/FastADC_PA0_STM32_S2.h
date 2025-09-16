#pragma once

#include <Arduino.h>
#include "MediaComposta.h"

class FastADC_PA0_STM32_S2
{
private:
    Variavel<float>& iMedio_;
    Variavel<float>& iPico_;
    MediaComposta* mediaComposta_;
    uint32_t periodoAmostragemUs_;
    uint32_t periodoTotalUs_;
    uint16_t amostrasPorPeriodo_;
    uint16_t tamanhoMediaMovel_;
    uint16_t valorNuloAdc_;
    HardwareTimer* timer_;
    static FastADC_PA0_STM32_S2* instance_;

    // Construtor privado
    FastADC_PA0_STM32_S2(Variavel<float>& iMedio,
                         Variavel<float>& iPico,       
                         MediaComposta* mediaComposta, 
                         uint32_t periodoAmostragemUs, 
                         uint32_t periodoTotalUs,
                         uint16_t amostrasPorPeriodo,
                         uint16_t tamanhoMediaMovel,
                         HardwareTimer* timer = nullptr)
        : iMedio_(iMedio),
          iPico_(iPico),
          mediaComposta_(nullptr),
          periodoAmostragemUs_(periodoAmostragemUs),
          periodoTotalUs_(periodoTotalUs),
          amostrasPorPeriodo_(amostrasPorPeriodo),
          tamanhoMediaMovel_(tamanhoMediaMovel),
          valorNuloAdc_(0),
          timer_(timer)
    {
        instance_ = this;
        mediaComposta_ = new MediaComposta(amostrasPorPeriodo, tamanhoMediaMovel);
        setupAdcPa0Fast();
        calcularValorNuloAdc();
        configuraTimer(periodoAmostragemUs_);
        anexarInterrupcao();
    }

    ~FastADC_PA0_STM32_S2()
    {
        desanexarInterrupcao();       // Evita que ISR acesse ponteiros deletados
        if (mediaComposta_)
        {
            delete mediaComposta_;    // Libera MediaComposta alocado dinamicamente
            mediaComposta_ = nullptr;
        }
        instance_ = nullptr;
    }

    // ISR do timer
    static void isrTimer_()
    {
        GPIOC->BSRR = (1 << 13);
        if (instance_ && instance_->mediaComposta_)
            instance_->mediaComposta_->adicionar((float)instance_->leituraAdc_());
        GPIOC->BSRR = (1 << (13 + 16));
    }

    void configuraTimer(uint32_t periodoUs)
    {
        if (timer_)
        {
            timer_->setOverflow(periodoUs, MICROSEC_FORMAT);
            timer_->resume();
        }
    }

    uint16_t leituraAdc_()
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
            delay(10);
        }
        valorNuloAdc_ = (uint16_t)(soma / totalLeituras);
        Serial2.println("ADC0: " + String(valorNuloAdc_));
    }

    void setupAdcPa0Fast()
    {
        pinMode(PA0, INPUT_ANALOG);
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
        ADC1->CR2 = 0;
        ADC1->SQR3 = 0;
        ADC1->CR2 |= ADC_CR2_ADON;
        delay(1);
        ADC1->CR2 |= ADC_CR2_RSTCAL;
        while (ADC1->CR2 & ADC_CR2_RSTCAL);
        ADC1->CR2 |= ADC_CR2_CAL;
        while (ADC1->CR2 & ADC_CR2_CAL);
    }

public:
    FastADC_PA0_STM32_S2(const FastADC_PA0_STM32_S2&) = delete;
    FastADC_PA0_STM32_S2& operator=(const FastADC_PA0_STM32_S2&) = delete;

    // Factory singleton usando pontos por ciclo
    static FastADC_PA0_STM32_S2* PontosPorCiclo(Variavel<float>& iMedio,
                                                Variavel<float>& iPico,
                                                uint32_t periodoAmostragemUs,
                                                uint16_t amostrasPorPeriodo, 
                                                uint16_t tamanhoMediaMovel = 100, 
                                                HardwareTimer* timer = nullptr)
    {
        if (!instance_)
        {
            uint32_t periodoTotalUs = periodoAmostragemUs * amostrasPorPeriodo;
            instance_ = new FastADC_PA0_STM32_S2(iMedio, iPico, nullptr, periodoAmostragemUs, periodoTotalUs, amostrasPorPeriodo, tamanhoMediaMovel, timer);
        }
        return instance_;
    }

    // Factory singleton usando período total
    static FastADC_PA0_STM32_S2* PeriodoTotal(Variavel<float>& iMedio,
                                              Variavel<float>& iPico,
                                              uint32_t periodoAmostragemUs,
                                              uint32_t periodoTotalUs, 
                                              uint16_t tamanhoMediaMovel = 100, 
                                              HardwareTimer* timer = nullptr)
    {
        if (!instance_)
        {
            uint16_t amostrasPorPeriodo = periodoTotalUs / periodoAmostragemUs;
            instance_ = new FastADC_PA0_STM32_S2(iMedio, iPico, nullptr, periodoAmostragemUs, periodoTotalUs, amostrasPorPeriodo, tamanhoMediaMovel, timer);
        }
        return instance_;
    }

    static FastADC_PA0_STM32_S2& getInstance()
    {
        return *instance_;
    }

    // Destruir singleton
    static void destruirSingleton()
    {
        if (instance_)
        {
            delete instance_;
            instance_ = nullptr;
        }
    }

    // Métodos de acesso
    uint16_t obterValorNuloAdc() { return valorNuloAdc_; }
    float obterMediaMovel() { return mediaComposta_->obterMediaMovel(); }
    float obterUltimaMediaSimples() { return mediaComposta_->obterUltimaMediaSimples(); }
    float obterMaiorMediaSimples() { return mediaComposta_->obterMaiorMediaSimples(); }
    float obterMaiorAmostra() { return mediaComposta_->obterMaiorAmostra(); }
    void recalibrarValorNuloAdc(uint16_t totalLeituras = 100) { calcularValorNuloAdc(totalLeituras); }

    void anexarInterrupcao()
    {
        if (timer_)
            timer_->attachInterrupt(isrTimer_);
    }

    void desanexarInterrupcao()
    {
        if (timer_)
            timer_->detachInterrupt();
    }

    void pausarTimer()
    {
        if (timer_)
            timer_->pause();
    }

    void resumirTimer()
    {
        if (timer_)
            timer_->resume();
    }

    void imprimirMediaMovel()
    {
        if (mediaComposta_)
            mediaComposta_->imprimirSerialBufferMediaMovel();
    }

    void atualizarImedioIpico()
    {
        if (mediaComposta_)
        {
            float media = mediaComposta_->obterMediaMovel();
            iMedio_.definirValor(media);
            //float pico = mediaComposta_->obterMaiorMediaMovel();
            float pico = mediaComposta_->obterMaiorMediaSimples();
            //float pico = mediaComposta_->obterMaiorAmostra();
            iPico_.definirValor(pico);
        }
    }
};

// Inicialização do singleton
FastADC_PA0_STM32_S2* FastADC_PA0_STM32_S2::instance_ = nullptr;
