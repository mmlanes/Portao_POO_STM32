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
    enum class Operacao { Nenhuma, Abrir, Fechar, Parar };

private:
    float rampaPWMPosicao_ = 2; // dPwm por porcentagem da posicao do encoder
    ChaveSTM32& fcS_;
    ChaveSTM32& fcI_;
    EncoderSTM32& encAB_;
    Motor& motor_;
    Variavel<bool>& encAtivo_;
    Variavel<uint8_t>& encPosPartida0a100_;
    Variavel<uint8_t>& encPosParada0a100_;
    Variavel<uint8_t>& dPWMPartida_;
    Variavel<uint8_t>& dPWMParada_;
    Posicao posicaoAtual_ = Posicao::Intermediario;
    Operacao operacaoAtual_ = Operacao::Nenhuma;

    // Métodos privados para cada tipo de operação
    void moverComEncoder(int8_t pos)
    {
        float dPWM = 0;
        uint8_t posStart = encPosPartida0a100_.obterValor();
        uint8_t posStop = encPosParada0a100_.obterValor();
        uint8_t pwmStart = dPWMPartida_.obterValor();
        uint8_t pwmStop = dPWMParada_.obterValor();
        if (pos < posStart)
            dPWM = (float)pwmStart;
        else if (pos > posStop)
            dPWM = (float)pwmStop;
        else
        {
            float pwm_pos50 = (float)dPWMPartida_.obterValor() + (50 - posStart) * (float)rampaPWMPosicao_;
            if (pos < 50)
                dPWM = (float)pwmStart + (float)(pos - posStart) * (float)rampaPWMPosicao_;
            else
                dPWM = (float)pwm_pos50 - (float)(pos - 50) * (float)rampaPWMPosicao_;
            if (dPWM > 100)
                dPWM = 100;
            else if (dPWM < 0)
                dPWM = 0;
        }
    }

    void abrirComEncoder()
    {
        if (motor_.obterSentido() == Motor::Sentido::Antihorario)
        {
            motor_.desligar();
            return;
        }
        int8_t pos = encAB_.obterPosicao_N100aP100();
        moverComEncoder( pos );
    }

    void fecharComEncoder()
    {
        if (motor_.obterSentido() == Motor::Sentido::Horario)
        {
            motor_.desligar();
            return;
        }
        int8_t pos = 0;
        if (operacaoAtual_ == Operacao::Fechar)
            pos = 100 - encAB_.obterPosicao_N100aP100();
        moverComEncoder( pos );
    }

    void abrirSemEncoder()
    {

    }

    void fecharSemEncoder()
    {

    }

    void desligarMotor()
    {
        motor_.desligar();
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

public:
    Portao(ChaveSTM32& fcS, ChaveSTM32& fcI, EncoderSTM32& encAB, Motor& motor, Variavel<bool>& encAtivo,
            Variavel<uint8_t>& encPosPartida0a100, Variavel<uint8_t>& encPosParada0a100, 
            Variavel<uint8_t> dPWMPartida, Variavel<uint8_t> dPWMParada)
        : fcS_(fcS), fcI_(fcI), encAB_(encAB), motor_(motor), encAtivo_(encAtivo), 
          encPosPartida0a100_(encPosPartida0a100), encPosParada0a100_(encPosParada0a100),
          dPWMPartida_(dPWMPartida), dPWMParada_(dPWMParada) {}

    Posicao obterPosicao() const { return posicaoAtual_; }

    void abrir()
    {
        if (encAtivo_.obterValor())
        {
            if (posicaoAtual_ == Posicao::Fechado || posicaoAtual_ == Posicao::Intermediario)
                operacaoAtual_ = Operacao::Abrir;
            else
                desligarMotor();
        }
        else
        {
            if (posicaoAtual_ == Posicao::Fechado || posicaoAtual_ == Posicao::Intermediario)
                operacaoAtual_ = Operacao::Abrir;
            else
                desligarMotor();
        }
    }

    void fechar()
    {
        if (encAtivo_.obterValor())
        {
            if (posicaoAtual_ == Posicao::Aberto || posicaoAtual_ == Posicao::Intermediario)
                operacaoAtual_ = Operacao::Fechar;
            else
                desligarMotor();
        }
        else
        {
            if (posicaoAtual_ == Posicao::Aberto || posicaoAtual_ == Posicao::Intermediario)
                operacaoAtual_ = Operacao::Fechar;
            else
                desligarMotor();
        }
    }

    void parar()
    {
        operacaoAtual_ = Operacao::Parar;
        desligarMotor();
    }

    void monitorar()
    {
        atualizarPosicao();

        switch (operacaoAtual_)
        {
            case Operacao::Abrir:
                if (encAtivo_.obterValor()) 
                    abrirComEncoder();
                else 
                    abrirSemEncoder();
                break;
            case Operacao::Fechar:
                if (encAtivo_.obterValor()) 
                    fecharComEncoder();
                else 
                    fecharSemEncoder();
                break;
            case Operacao::Parar:
            case Operacao::Nenhuma:
                desligarMotor();
                break;
        }
    }
};
