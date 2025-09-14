#pragma once

#include <Arduino.h>
#include <vector>

class FastADC_PA0_STM32_S2
{
private:
    static std::vector<uint16_t> bufferLeituras_;               // Buffer principal
    static std::vector<uint16_t> bufferCircularCiclosLeituras_; // Buffer circular
    static uint8_t dimensaoBufferCircular_;
    static uint8_t posicaoBufferCircular_;
    static uint32_t periodoAmostragemUs_;
    static uint16_t periodoTotalUs_;
    static uint16_t capacidadeBuffer_;
    static uint16_t valorNuloAdc_;
    static bool bufferCheio_;
    static uint16_t valorMedio_;
    static uint16_t valorMaximo_;
    static HardwareTimer* timer2_;
    static FastADC_PA0_STM32_S2* instance_;

    // Construtor privado
    FastADC_PA0_STM32_S2(uint32_t periodoAmostragemUs, uint16_t periodoTotalUs, uint8_t dimensaoBufferCircular)
    {
        periodoAmostragemUs_ = periodoAmostragemUs;
        periodoTotalUs_ = periodoTotalUs;
        dimensaoBufferCircular_ = dimensaoBufferCircular;
        posicaoBufferCircular_ = 0;

        capacidadeBuffer_ = periodoTotalUs_ / periodoAmostragemUs_;
        bufferLeituras_.reserve(capacidadeBuffer_);
        bufferCircularCiclosLeituras_.resize(dimensaoBufferCircular_);

        instance_ = this;

        setupAdcPa0Fast();
        calcularValorNuloAdc();
        configuraTimer(periodoAmostragemUs_);
        anexarInterrupcao();
    }

    // Configuração de timer
    static void configuraTimer(uint32_t periodoUs)
    {
        if (!timer2_)
        {
            timer2_ = new HardwareTimer(TIM2);
            timer2_->setOverflow(periodoUs, MICROSEC_FORMAT);
            timer2_->resume();
        }
    }

    static void anexarInterrupcao()
    {
        if (timer2_)
            timer2_->attachInterrupt(isrTim2);
    }

    static void desanexarInterrupcao()
    {
        if (timer2_)
            timer2_->detachInterrupt();
    }

    // ISR: leitura do ADC + atualização buffer circular e cálculo médio/máximo
    static void isrTim2()
    {
        uint16_t leitura = leituraAdc_();

        // Armazena no buffer principal
        if (bufferLeituras_.size() < capacidadeBuffer_)
        {
            bufferLeituras_.push_back(leitura);
        }
        else
        {
            bufferCheio_ = true;
        }

        // Buffer circular
        bufferCircularCiclosLeituras_[posicaoBufferCircular_] = leitura;
        posicaoBufferCircular_ = (posicaoBufferCircular_ + 1) % dimensaoBufferCircular_;

        // Recalcula média e máximo do buffer circular
        uint32_t soma = 0;
        uint16_t maximo = 0;
        for (uint8_t i = 0; i < dimensaoBufferCircular_; i++)
        {
            uint16_t val = bufferCircularCiclosLeituras_[i];
            soma += val;
            if (val > maximo)
                maximo = val;
        }
        valorMedio_ = soma / dimensaoBufferCircular_;
        valorMaximo_ = maximo;
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

    static void calcularValorNuloAdc(uint16_t totalLeituras = 100)
    {
        uint32_t soma = 0;
        for (uint16_t i = 0; i < totalLeituras; i++)
        {
            soma += leituraAdc_();
            delay(1);
        }
        valorNuloAdc_ = (uint16_t)(soma / totalLeituras);
        Serial2.println("Valor nulo ADC recalibrado: " + String(valorNuloAdc_));
    }

    static void setupAdcPa0Fast()
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

    // Obter instância singleton
    static FastADC_PA0_STM32_S2& getInstance(uint32_t periodoAmostragemUs, uint16_t periodoTotalUs, uint8_t dimensaoBufferCircular = 100)
    {
        if (!instance_)
            instance_ = new FastADC_PA0_STM32_S2(periodoAmostragemUs, periodoTotalUs, dimensaoBufferCircular);
        return *instance_;
    }

    static FastADC_PA0_STM32_S2& getInstance()
    {
        return *instance_;
    }

    // Métodos para acessar valores
    uint16_t obterValorNuloAdc() { return valorNuloAdc_; }
    uint16_t obterValorMedio() { return valorMedio_; }
    uint16_t obterValorMaximo() { return valorMaximo_; }
    bool isBufferCheio() { return bufferCheio_; }
    const std::vector<uint16_t>& obterBuffer() { return bufferLeituras_; }

    void recalibrarValorNuloAdc(uint16_t totalLeituras = 100) { calcularValorNuloAdc(totalLeituras); }
};

// Inicialização dos membros estáticos
std::vector<uint16_t> FastADC_PA0_STM32_S2::bufferLeituras_;
std::vector<uint16_t> FastADC_PA0_STM32_S2::bufferCircularCiclosLeituras_;
uint8_t FastADC_PA0_STM32_S2::dimensaoBufferCircular_ = 0;
uint8_t FastADC_PA0_STM32_S2::posicaoBufferCircular_ = 0;
uint32_t FastADC_PA0_STM32_S2::periodoAmostragemUs_ = 0;
uint16_t FastADC_PA0_STM32_S2::periodoTotalUs_ = 0;
uint16_t FastADC_PA0_STM32_S2::capacidadeBuffer_ = 0;
uint16_t FastADC_PA0_STM32_S2::valorNuloAdc_ = 0;
bool FastADC_PA0_STM32_S2::bufferCheio_ = false;
uint16_t FastADC_PA0_STM32_S2::valorMedio_ = 0;
uint16_t FastADC_PA0_STM32_S2::valorMaximo_ = 0;
HardwareTimer* FastADC_PA0_STM32_S2::timer2_ = nullptr;
FastADC_PA0_STM32_S2* FastADC_PA0_STM32_S2::instance_ = nullptr;
