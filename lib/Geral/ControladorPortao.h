#pragma once

#include <Arduino.h>
#include "Motor.h"
#include "Variavel.h"
#include "Portao2.h"
#include "Protecao.h"

class ControladorPortao
{
public:
    enum class Operacao { Nenhuma, AbrirComEncoder, FecharComEncoder, AbrirSemEncoder, FecharSemEncoder, Parar };

private:
    Portao2 portao_;
    Protecao protecao_;
    Operacao operacaoAtual_;

    // Métodos privados para cada tipo de operação
    uint8_t dPWMComEncoder(uint8_t pos0a100)
    {
        float dPWM = 0;
        uint8_t posStart = portao_.encPosPartida0a100_.obterValor();
        uint8_t posStop = portao_.encPosParada0a100_.obterValor();
        uint8_t pwmStart = portao_.dPWMPartida_.obterValor();
        uint8_t pwmStop = portao_.dPWMParada_.obterValor();
        if (pos0a100 < posStart)
            dPWM = (float)pwmStart;
        else if (pos0a100 > posStop)
            dPWM = (float)pwmStop;
        else
        {
            float pwm_pos50 = (float)pwmStart + (50 - posStart) * portao_.rampaPWMPosicao_.obterValor();
            if (pos0a100 < 50)
                dPWM = (float)pwmStart + (float)(pos0a100 - posStart) * portao_.rampaPWMPosicao_.obterValor();
            else
                dPWM = (float)pwm_pos50 - (float)(pos0a100 - 50) * portao_.rampaPWMPosicao_.obterValor();
            
            if (dPWM > 100) dPWM = 100; else if (dPWM < 0) dPWM = 0;
        }
        return (uint8_t)dPWM;
    }

    void abrirComEncoder()
    {
        if (portao_.fcI_.estaAtiva()) //motor_.obterEstadoAtual() != Motor::Estado::Horario && 
            portao_.encAB_.zerarPosicao();
        if (portao_.encAB_.obterPosicao() < (int32_t)portao_.encAB_.obterPosicaoMaximaAbs()) 
        {
            int8_t pos = portao_.encAB_.obterPosicao_N100aP100();
            uint8_t dPWM = dPWMComEncoder(pos);
            portao_.motor_.mover(Motor::Estado::Horario, dPWM);
        }
        else
            operacaoAtual_ = Operacao::Parar;
    }

    void fecharComEncoder()
    {
        if (portao_.fcS_.estaAtiva()) // motor_.obterEstadoAtual() != Motor::Estado::Antihorario && 
            portao_.encAB_.setarPosicao();
        if (portao_.encAB_.obterPosicao() > 0) 
        {
            int8_t pos = portao_.encAB_.obterPosicao_N100aP100();
            if (operacaoAtual_ == Operacao::FecharComEncoder)
                pos = 100 - portao_.encAB_.obterPosicao_N100aP100();
            uint8_t dPWM = dPWMComEncoder(pos);
            portao_.motor_.mover(Motor::Estado::Antihorario, dPWM);
        }
        else
            operacaoAtual_ = Operacao::Parar;
    }

    void abrirSemEncoder()
    {
        //protecao_.liberarProtecaoSobrecorrente();
        float dPWM = (float)portao_.dPWMPartida_.obterValor() * portao_.multiplicadorPWM_;
        portao_.motor_.mover(Motor::Estado::Horario, (uint8_t)dPWM);
    }

    void fecharSemEncoder()
    {
        //protecao_.liberarProtecaoSobrecorrente();
        float dPWM = (float)portao_.dPWMPartida_.obterValor() * portao_.multiplicadorPWM_;
        portao_.motor_.mover(Motor::Estado::Antihorario, (uint8_t)dPWM);
    }

    void desligarMotor()
    {
        portao_.motor_.desligar();
        operacaoAtual_ = Operacao::Nenhuma;
    }

    void atualizarOperacao()
    {
        bool fcs = portao_.fcS_.estaAtiva();
        bool fci = portao_.fcI_.estaAtiva();

        if ( (operacaoAtual_==Operacao::AbrirComEncoder || 
              operacaoAtual_==Operacao::AbrirSemEncoder) && fcs) 
            operacaoAtual_ = Operacao::Parar;
        else if ( (operacaoAtual_==Operacao::FecharComEncoder || 
                   operacaoAtual_==Operacao::FecharSemEncoder) && fci) 
            operacaoAtual_ = Operacao::Parar;
        else if (portao_.posicaoAtual_ == Portao2::Posicao::Erro)
            operacaoAtual_ = Operacao::Parar;
    }

public:
    ControladorPortao(Portao2 portao, Protecao protecao)
        : portao_(portao), protecao_(protecao), operacaoAtual_(Operacao::Nenhuma)
        {}
  
    void abrir()
    {
        if (portao_.posicaoAtual_ == Portao2::Posicao::Fechado || 
            portao_.posicaoAtual_ == Portao2::Posicao::Intermediario)
        {
            if (portao_.encAtivo_.obterValor())
            {
                protecao_.liberarProtecaoEncoderParado();
                portao_.encAB_.resetParado();
                operacaoAtual_ = Operacao::AbrirComEncoder;
            }
            else
                operacaoAtual_ = Operacao::AbrirSemEncoder;
        }
        else
            operacaoAtual_ = Operacao::Parar;
    }

    void fechar()
    {
        if (portao_.posicaoAtual_ == Portao2::Posicao::Aberto || 
            portao_.posicaoAtual_ == Portao2::Posicao::Intermediario)
        {
            if (portao_.encAtivo_.obterValor())
            {
                protecao_.liberarProtecaoEncoderParado();
                portao_.encAB_.resetParado();
                operacaoAtual_ = Operacao::FecharComEncoder;
            }
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
            portao_.multiplicadorPWM_ = 3;
    }

    void abrirFecharDesaceleradoSemEncoder()
    {
        if (operacaoAtual_==Operacao::AbrirSemEncoder || operacaoAtual_==Operacao::FecharSemEncoder )
            portao_.multiplicadorPWM_ = 0.5;
    }

    void velocidadeNormalSemEncoder()
    {
        portao_.multiplicadorPWM_ = 1;
    }

    float obterMultiplicadorSemEncoder()
    {
        return portao_.multiplicadorPWM_;
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

    void protecaoEncoderParado()
    {
        if (portao_.encAtivo_.obterValor())
        {
            if (operacaoAtual_==Operacao::AbrirComEncoder || operacaoAtual_==Operacao::FecharComEncoder)
                operacaoAtual_ = Operacao::Parar;
        }
    }

    void protecaoSobrecorrente()
    {
        operacaoAtual_ = Operacao::Parar;
    }

    void verificarProtecao()
    {
        if (protecao_.protecaoEncoderParadoAtuada())
            operacaoAtual_ = Operacao::Parar;
        if (protecao_.protecaoSobrecorrenteAtuada())
            operacaoAtual_ = Operacao::Parar;
    }
    void monitorar()
    {
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
        verificarProtecao();
    }
};
