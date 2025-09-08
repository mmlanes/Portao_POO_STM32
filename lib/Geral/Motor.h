#pragma once

#include <Arduino.h>
#include "PWM_PB1_STM32_S.h"
#include "Variavel.h"

class Motor
{
public:
    enum class Estado { Parado, Parando, Horario, Antihorario };

private:
    uint8_t pinKD_;
    uint8_t pinKE_;
    uint8_t dpwmAlvo_;
    unsigned long tempoEsperaRele_;
    PWM_PB1_STM32_S& pwm_;

    Estado estadoAtual_ = Estado::Parado;
    Estado estadoAlvo_ = Estado::Parado;

    unsigned long tempoPwmZero_ = 0;
    bool pwmZerado_ = false;

    void atualizarPwmZero()
    {
        if (pwm_.obterDpwmAtual() == 0)
        {
            if (!pwmZerado_)
            {
                tempoPwmZero_ = millis();
                pwmZerado_ = true;
            }
        }
        else
            pwmZerado_ = false;
    }

    bool podeAtuarRele() const
    {
        return pwmZerado_ && ((millis() - tempoPwmZero_) >= tempoEsperaRele_);
    }

    void aplicarRelesImediato(Estado estado)
    {
        switch (estado)
        {
            case Estado::Horario:
                digitalWrite(pinKD_, LOW);
                digitalWrite(pinKE_, LOW);
                break;
            case Estado::Antihorario:
                digitalWrite(pinKD_, HIGH);
                digitalWrite(pinKE_, HIGH);
                break;
            case Estado::Parando:
                break;
            case Estado::Parado:
                digitalWrite(pinKD_, LOW);
                digitalWrite(pinKE_, HIGH);
                break;
        }
        estadoAtual_ = estado;
    }

public:
    Motor(uint8_t pinKD, uint8_t pinKE, PWM_PB1_STM32_S& pwm, unsigned long tempoEspera = 1000)
        : pinKD_(pinKD), pinKE_(pinKE), dpwmAlvo_(0), tempoEsperaRele_(tempoEspera), pwm_(pwm)
    {
        pinMode(pinKD_, OUTPUT);
        pinMode(pinKE_, OUTPUT);
        aplicarRelesImediato(Estado::Parado);
    }

    Estado obterEstadoAtual() const { return estadoAtual_; }
    Estado obterEstadoAlvo() const { return estadoAlvo_; }

    void definirTempoEspera(unsigned long tempo) { tempoEsperaRele_ = tempo; }

    void mover(Estado estado, uint8_t dpwm)
    {
        // Validação de parâmetros
        if (estado == Estado::Parado) 
            return;

        if (dpwm > 100) // Assumindo PWM 0-100%
            dpwm = 100;

        if (estadoAtual_ == Estado::Parado) 
        {   // Está parado e vai para Horario ou Antihorario
            if (podeAtuarRele())
            {
                aplicarRelesImediato(estado);
                pwm_.definirDpwmRampa(dpwm);
                estadoAtual_ = estado;
                dpwmAlvo_ = dpwm;
            }
        }
        else if ( (estadoAtual_ == Estado::Horario && estado == Estado::Antihorario) ||
                  (estadoAtual_ == Estado::Antihorario && estado == Estado::Horario) )
        {   // Quer ir para estado oposto (precisa parar primeiro)
            estadoAlvo_ = estado;
            //aplicarRelesImediato(Estado::Parado);
            pwm_.definirDpwmRampa(0);
            dpwmAlvo_ = 0;
            estadoAtual_ = Estado::Parando;
        }
        else if (estadoAtual_ == estado) // Já está no próprio estado (altera dpwm)
        {
            estadoAlvo_ = estado;
            pwm_.definirDpwmRampa(dpwm);
            dpwmAlvo_ = dpwm;
        }
        else if (estadoAtual_ == Estado::Parando && pwm_.obterDpwmAtual()==0) // Já está parando (altera dpwmAlvo)
        {
            estadoAlvo_ = Estado::Parado;
            estadoAtual_ = Estado::Parado;
            //dpwmAlvo_ = 0;
        }
    }

    void desligar()
    {
        if (estadoAtual_ == Estado::Horario || estadoAtual_ == Estado::Antihorario)
        {
            estadoAlvo_ = Estado::Parado;
            pwm_.definirDpwmRampa(0);
            dpwmAlvo_ = 0;
            estadoAtual_ = Estado::Parando;
        }
    }

    void monitorar()
    {
        atualizarPwmZero();
        if (estadoAtual_ == Estado::Parando && estadoAlvo_ == Estado::Parado )
        {   // Vai de parando para parado
            if (podeAtuarRele())
            {
                aplicarRelesImediato(estadoAlvo_);
                pwm_.definirDpwmRampa(0); 
                dpwmAlvo_ = 0;
                estadoAtual_ = estadoAlvo_;
            }
        }
        else if ( (estadoAtual_ == Estado::Parado || estadoAtual_ == Estado::Parando) && 
                   dpwmAlvo_ > 0 &&
                  (estadoAlvo_ == Estado::Horario || estadoAlvo_ == Estado::Antihorario) )
        {
            if (pwm_.obterDpwmAtual() == dpwmAlvo_)
                estadoAtual_ = estadoAlvo_;
            else
                pwm_.definirDpwmRampa(dpwmAlvo_);
        }
    }
};
