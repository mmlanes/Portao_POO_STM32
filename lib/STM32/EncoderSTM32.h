#pragma once

#include <Arduino.h>
#include "Variavel.h"

class EncoderSTM32 
{
private:
    uint8_t pinA_;
    uint8_t pinB_;
    bool pinA_is_Isr_;
    bool pinB_is_Isr_;
    Variavel<bool>& reverso_;
    Variavel<int32_t>& posicao_;
    Variavel<uint32_t>& posicaoMaxima_;

    // Para verificação de movimento
    int32_t ultimaPosicao_;
    uint32_t ultimoTempo_;

    void atualizarPosicaoIsr(void)
    {
        bool a = digitalRead(pinA_);
        bool b = digitalRead(pinB_);
        if (a == b)
        {
            if (reverso_.obterValor())
                posicao_.decrementar();
            else
                posicao_.incrementar();
        }
        else
        {
            if (reverso_.obterValor())
                posicao_.incrementar();
            else
                posicao_.decrementar();
        }
    }

public:
    EncoderSTM32(Variavel<int32_t>& pos, Variavel<uint32_t>& max, Variavel<bool>& rev, 
                 uint8_t pinA, uint8_t pinB, 
                 bool pinA_is_Isr = true, bool pinB_is_Isr = false, bool contagemReversa = false)
        : posicao_(pos), posicaoMaxima_(max), reverso_(rev), 
          pinA_(pinA), pinB_(pinB), pinA_is_Isr_(pinA_is_Isr), pinB_is_Isr_(pinB_is_Isr),
          ultimaPosicao_(0), ultimoTempo_(0)
    {
        pinMode(pinA_, INPUT_PULLUP);
        pinMode(pinB_, INPUT_PULLUP);

        if (pinA_is_Isr_)
            attachInterrupt(digitalPinToInterrupt(pinA_), std::bind(&EncoderSTM32::atualizarPosicaoIsr, this), RISING);
        if (!pinA_is_Isr_ && pinB_is_Isr_)
            attachInterrupt(digitalPinToInterrupt(pinB_), std::bind(&EncoderSTM32::atualizarPosicaoIsr, this), RISING);

        ultimaPosicao_ = posicao_.obterValor();
        ultimoTempo_ = millis();
    }

    void zerarPosicao(void) { posicao_.definirValor(0); }
    int32_t obterPosicao(void) const { return posicao_.obterValor(); }
    void definirPosicao(int32_t p) { posicao_.definirValor(p); }

    int8_t obterPosicao_N100aP100(void) const
    {
        int32_t p = (posicao_.obterValor() * 100) / (int32_t)posicaoMaxima_.obterValor();
        return static_cast<int8_t>(p);
    }

    void definirPosicao_N100aP100(int8_t p)
    {
        if (p < -100) p = -100;
        else if (p > 100) p = 100;
        int32_t pos = (p * posicaoMaxima_.obterValor()) / 100;
        posicao_.definirValor(pos);
    }

    uint32_t obterPosicaoMaximaAbs(void) { return posicaoMaxima_.obterValor(); }
    void definirPosicaoMaximaAbs(int32_t pMaxAbs)
    {
        if (pMaxAbs < 1) pMaxAbs = 1;
        posicaoMaxima_.definirValor(pMaxAbs);
    }

    int8_t obterPosicaoMaxima_0a100(void)
    {
        return 100 * (float)posicao_.obterValor() / posicaoMaxima_.obterValor();
    }

    void definirSentidoEncoder(bool reverso = false) { reverso_.definirValor(reverso); }
    bool obterSentidoEncoder(void) { return reverso_.obterValor(); }

    /// 🔹 Novo método: verifica se o encoder está parado
    bool estaParado(int32_t delta = 50, uint32_t intervaloMs = 3000)
    {
        uint32_t agora = millis();
        if (agora - ultimoTempo_ >= intervaloMs)
        {
            int32_t posAtual = posicao_.obterValor();
            int32_t diferenca = abs(posAtual - ultimaPosicao_);

            // Atualiza referência
            ultimaPosicao_ = posAtual;
            ultimoTempo_ = agora;

            // Se não se moveu mais que delta → parado
            return (diferenca < delta);
        }
        return false; // ainda não passou tempo suficiente
    }
};
