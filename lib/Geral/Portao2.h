#pragma once

#include <Arduino.h>
#include "Motor.h"
#include "Variavel.h"
#include "IChave.h"
#include "EncoderSTM32.h"
#include "PWM_PB1_STM32_S.h"

class Portao2
{
public:
    enum class Posicao { Fechado, Aberto, Intermediario, Erro };

private:
    IChave& fcS_;
    IChave& fcI_;
    EncoderSTM32& encAB_;
    Motor& motor_;
    Variavel<bool>& encAtivo_;
    Variavel<uint8_t>& encPosPartida0a100_;
    Variavel<uint8_t>& encPosParada0a100_;
    Variavel<uint8_t>& dPWMPartida_;
    Variavel<uint8_t>& dPWMParada_;
    Variavel<float>& rampaPWMPosicao_; // dPwm por porcentagem da posicao do encoder
    float multiplicadorPWM_; // Acelerar o motor nos modos sem encoder
    Posicao posicaoAtual_;

    void atualizarPosicao()
    {
        bool fcs = fcS_.estaAtiva();
        bool fci = fcI_.estaAtiva();

        if (fcs && !fci) 
            posicaoAtual_ = Posicao::Aberto;
        else if (!fcs && fci) 
            posicaoAtual_ = Posicao::Fechado;
        else if (!fcs && !fci) 
            posicaoAtual_ = Posicao::Intermediario;
        else 
            posicaoAtual_ = Posicao::Erro;

    }

public:
    Portao2(IChave& fcS, IChave& fcI, EncoderSTM32& encAB, Motor& motor, Variavel<bool>& encAtivo,
            Variavel<uint8_t>& encPosPartida0a100, Variavel<uint8_t>& encPosParada0a100, 
            Variavel<uint8_t>& dPWMPartida, Variavel<uint8_t>& dPWMParada, Variavel<float>& rampaPWMPosicao)
        : fcS_(fcS), fcI_(fcI), encAB_(encAB), motor_(motor), encAtivo_(encAtivo), 
          encPosPartida0a100_(encPosPartida0a100), encPosParada0a100_(encPosParada0a100),
          dPWMPartida_(dPWMPartida), dPWMParada_(dPWMParada), rampaPWMPosicao_(rampaPWMPosicao), multiplicadorPWM_(1),
          posicaoAtual_(Posicao::Intermediario)
        {}

    Posicao obterPosicao() const { return posicaoAtual_; }

    EncoderSTM32& obterEncoder() { return encAB_; }
    Motor& obterMotor() { return motor_; }
    
    String obterPosicaoAtualString()
    {
        switch (posicaoAtual_)
        {
            case Posicao::Fechado: return "Fc";
            case Posicao::Aberto: return "Ab";
            case Posicao::Intermediario: return "I.";
            case Posicao::Erro: return "E.";
            default: return "P?";
        }
    }

    Posicao obterPosicaoAtual()
    {
        return posicaoAtual_;
    }

    void monitorar()
    {
        atualizarPosicao();
    }

    friend class ControladorPortao;
};
