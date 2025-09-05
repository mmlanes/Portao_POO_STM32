#pragma once

#include <Arduino.h>

class EncoderSTM32 
{
    private:
    uint8_t pinA_;
    uint8_t pinB_;
    bool pinA_is_Isr_;
    bool pinB_is_Isr_;
    bool reverso_;
    int32_t posicao_;
    uint32_t posicaoMaxima_;
    void atualizarPosicaoIsr(void)
    {
        bool a = digitalRead(pinA_);
        bool b = digitalRead(pinB_);
        if (a == b)
        {
            if (reverso_)
                posicao_--;
            else
                posicao_++;
        }
        else
        {
            if (reverso_)
                posicao_++;
            else
                posicao_--;
        }
    }

public:
    EncoderSTM32(uint8_t pinA, uint8_t pinB, bool pinA_is_Isr = true, bool pinB_is_Isr = false, bool contagemReversa = false)
    : pinA_(pinA), pinB_(pinB), pinA_is_Isr_(pinA_is_Isr), pinB_is_Isr_(pinB_is_Isr), reverso_(contagemReversa), posicao_(0), posicaoMaxima_(1)
    {
        pinMode(pinA_, INPUT_PULLUP);
        pinMode(pinB_, INPUT_PULLUP);
        if (pinA_is_Isr_)
            attachInterrupt(digitalPinToInterrupt(pinA_), std::bind(&EncoderSTM32::atualizarPosicaoIsr, this), RISING);
        if (!pinA_is_Isr_ && pinB_is_Isr_)
            attachInterrupt(digitalPinToInterrupt(pinB_), std::bind(&EncoderSTM32::atualizarPosicaoIsr, this), RISING);
    }
    void zerarPosicao(void)
    {
        posicao_ = 0;
    }
    int32_t obterPosicao(void) const
    {
        return posicao_;
    }
    void definirPosicao(int32_t p)
    {
        posicao_ = p;
    }
    int8_t obterPosicao_N100aP100(void) const
    {
        int32_t p = (posicao_ * 100) / (int32_t)posicaoMaxima_;
        // if (p < -100)
        //     p = -100;
        // else if (p > 100)
        //     p = 100;
        return static_cast<int8_t>(p);
    }
    void definirPosicao_N100aP100(int8_t p)
    {
        if (p < -100)
            p = -100;
        else if (p > 100)
            p = 100;
        posicao_ = (p * posicaoMaxima_) / 100;
    }
    uint32_t obterPosicaoMaximaAbs(void)
    {
        return posicaoMaxima_;
    }
    void definirPosicaoMaximaAbs(int32_t pMaxAbs)
    {
        if (pMaxAbs < 1)
            pMaxAbs = 1;
        posicaoMaxima_ = pMaxAbs;
    }
    int8_t obterPosicaoMaxima_0a100(void)
    {
        return 100 * (float)posicao_ / posicaoMaxima_;
    }
    void definirSentidoEncoder(bool reverso = false)
    {
        reverso_ = reverso;
    }
    bool obterSentidoEncoder(void)
    {
        return reverso_;
    }
};

