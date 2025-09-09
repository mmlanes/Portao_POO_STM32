#pragma once

#include <Arduino.h>
#include "Motor.h"
#include "Variavel.h"
#include "ChaveSTM32.h"
#include "EncoderSTM32.h"
#include "PWM_PB1_STM32_S.h"

class Portao
{
public:
    enum class Posicao { Fechado, Aberto, Intermediario, Erro };
    enum class Operacao { Nenhuma, AbrirComEncoder, FecharComEncoder, AbrirSemEncoder, FecharSemEncoder, Parar };

private:
    ChaveSTM32& fcS_;
    ChaveSTM32& fcI_;
    EncoderSTM32& encAB_;
    Motor& motor_;
    Variavel<bool>& encAtivo_;
    Variavel<uint8_t>& encPosPartida0a100_;
    Variavel<uint8_t>& encPosParada0a100_;
    Variavel<uint8_t>& dPWMPartida_;
    Variavel<uint8_t>& dPWMParada_;
    float rampaPWMPosicao_; // dPwm por porcentagem da posicao do encoder
    float multiplicadorPWM_; // Acelerar o motor nos modos sem encoder
    Posicao posicaoAtual_;
    Operacao operacaoAtual_;

    // Métodos privados para cada tipo de operação
    uint8_t dPWMComEncoder(uint8_t pos0a100)
    {
        float dPWM = 0;
        uint8_t posStart = encPosPartida0a100_.obterValor();
        uint8_t posStop = encPosParada0a100_.obterValor();
        uint8_t pwmStart = dPWMPartida_.obterValor();
        uint8_t pwmStop = dPWMParada_.obterValor();
        if (pos0a100 < posStart)
            dPWM = (float)pwmStart;
        else if (pos0a100 > posStop)
            dPWM = (float)pwmStop;
        else
        {
            float pwm_pos50 = (float)pwmStart + (50 - posStart) * (float)rampaPWMPosicao_;
            if (pos0a100 < 50)
                dPWM = (float)pwmStart + (float)(pos0a100 - posStart) * (float)rampaPWMPosicao_;
            else
                dPWM = (float)pwm_pos50 - (float)(pos0a100 - 50) * (float)rampaPWMPosicao_;
            
            if (dPWM > 100) dPWM = 100; else if (dPWM < 0) dPWM = 0;
        }
        return (uint8_t)dPWM;
    }

    void abrirComEncoder()
    {
        if (encAB_.obterPosicao() < encAB_.obterPosicaoMaximaAbs()) 
        {
            if (motor_.obterEstadoAtual() != Motor::Estado::Horario && fcI_.estaAtiva())
                encAB_.zerarPosicao();
            int8_t pos = encAB_.obterPosicao_N100aP100();
            uint8_t dPWM = dPWMComEncoder(pos);
            motor_.mover(Motor::Estado::Horario, dPWM);
        }
        else
        {
            operacaoAtual_ == Operacao::Parar;
        }
    }

    void fecharComEncoder()
    {
        if (encAB_.obterPosicao() > 0) 
        {
            if (motor_.obterEstadoAtual() != Motor::Estado::Antihorario && fcS_.estaAtiva())
                encAB_.setarPosicao();
            int8_t pos = encAB_.obterPosicao_N100aP100();
            if (operacaoAtual_ == Operacao::FecharComEncoder)
                pos = 100 - encAB_.obterPosicao_N100aP100();
            uint8_t dPWM = dPWMComEncoder(pos);
            motor_.mover(Motor::Estado::Antihorario, dPWM);
        }
        else
        {
            operacaoAtual_ == Operacao::Parar;
        }
    }

    void abrirSemEncoder()
    {
        float dPWM = (float)dPWMPartida_.obterValor() * multiplicadorPWM_;
        motor_.mover(Motor::Estado::Horario, (uint8_t)dPWM);
    }

    void fecharSemEncoder()
    {
        float dPWM = (float)dPWMPartida_.obterValor() * multiplicadorPWM_;
        motor_.mover(Motor::Estado::Antihorario, (uint8_t)dPWM);
    }

    void desligarMotor()
    {
        motor_.desligar();
        operacaoAtual_ = Operacao::Nenhuma;
    }

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

    void atualizarOperacao()
    {
        bool fcs = fcS_.estaAtiva();
        bool fci = fcI_.estaAtiva();

        if ( (operacaoAtual_==Operacao::AbrirComEncoder || 
              operacaoAtual_==Operacao::AbrirSemEncoder) && fcs) 
            operacaoAtual_ = Operacao::Parar;
        else if ( (operacaoAtual_==Operacao::FecharComEncoder || 
                   operacaoAtual_==Operacao::FecharSemEncoder) && fci) 
            operacaoAtual_ = Operacao::Parar;
        else if (posicaoAtual_ == Posicao::Erro)
            operacaoAtual_ = Operacao::Parar;
    }

public:
    Portao(ChaveSTM32& fcS, ChaveSTM32& fcI, EncoderSTM32& encAB, Motor& motor, Variavel<bool>& encAtivo,
            Variavel<uint8_t>& encPosPartida0a100, Variavel<uint8_t>& encPosParada0a100, 
            Variavel<uint8_t>& dPWMPartida, Variavel<uint8_t>& dPWMParada)
        : fcS_(fcS), fcI_(fcI), encAB_(encAB), motor_(motor), encAtivo_(encAtivo), 
          encPosPartida0a100_(encPosPartida0a100), encPosParada0a100_(encPosParada0a100),
          dPWMPartida_(dPWMPartida), dPWMParada_(dPWMParada), rampaPWMPosicao_(2), multiplicadorPWM_(1),
          posicaoAtual_(Posicao::Intermediario), operacaoAtual_(Operacao::Nenhuma)
        {}

    Posicao obterPosicao() const { return posicaoAtual_; }

    void abrir()
    {
        if (posicaoAtual_ == Posicao::Fechado || posicaoAtual_ == Posicao::Intermediario)
        {
            if (encAtivo_.obterValor())
                operacaoAtual_ = Operacao::AbrirComEncoder;
            else
                operacaoAtual_ = Operacao::AbrirSemEncoder;
        }
        else
            operacaoAtual_ = Operacao::Parar;
    }

    void fechar()
    {
        if (posicaoAtual_ == Posicao::Aberto || posicaoAtual_ == Posicao::Intermediario)
        {
            if (encAtivo_.obterValor())
                operacaoAtual_ = Operacao::FecharComEncoder;
            else
                operacaoAtual_ = Operacao::FecharSemEncoder;
        }
        else
            operacaoAtual_ = Operacao::Parar;
    }

    void parar()
    {
        operacaoAtual_ = Operacao::Parar;
    }

    void abrirFecharAceleradoSemEncoder()
    {
        if (operacaoAtual_==Operacao::AbrirSemEncoder || operacaoAtual_==Operacao::FecharSemEncoder )
            multiplicadorPWM_ = 3;
    }

    void abrirFecharDesaceleradoSemEncoder()
    {
        if (operacaoAtual_==Operacao::AbrirSemEncoder || operacaoAtual_==Operacao::FecharSemEncoder )
            multiplicadorPWM_ = 0.5;
    }

    void velocidadeNormalSemEncoder()
    {
        multiplicadorPWM_ = 1;
    }

    float obterMultiplicadorSemEncoder()
    {
        return multiplicadorPWM_;
    }

    String obterOperacaoAtualString()
    {
        switch (operacaoAtual_)
        {
            case Operacao::Nenhuma: return "N.";
            case Operacao::AbrirComEncoder: return "AC";
            case Operacao::FecharComEncoder: return "FC";
            case Operacao::AbrirSemEncoder: return "AS";
            case Operacao::FecharSemEncoder: return "FS";
            case Operacao::Parar: return "P.";
            default: return "O?";
        }
    }

    Operacao obterOperacaoAtual()
    {
        return operacaoAtual_;
    }

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
        atualizarOperacao();
        switch (operacaoAtual_)
        {
            case Operacao::Parar:
                desligarMotor();
                break;
            case Operacao::AbrirComEncoder:
                abrirComEncoder();
                break;
            case Operacao::FecharComEncoder:
                fecharComEncoder();
                break;
            case Operacao::AbrirSemEncoder:
                abrirSemEncoder();
                break;
            case Operacao::FecharSemEncoder:
                fecharSemEncoder();
                break;
            case Operacao::Nenhuma:
                desligarMotor();
                break;
        }
    }
};
