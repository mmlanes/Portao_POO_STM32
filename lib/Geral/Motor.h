#pragma once

#include <Arduino.h>
#include "PWM_PB1_STM32_S.h"
#include "Variavel.h"

class Motor
{
public:
    enum class Sentido { Indefinido, Horario, Antihorario };

private:
    uint8_t pinKD_;
    uint8_t pinKE_;
    PWM_PB1_STM32_S& pwm_;

    Sentido sentidoAtual_ = Sentido::Indefinido;
    Sentido sentidoAlvo_ = Sentido::Indefinido;

    unsigned long tempoPwmZero_ = 0;
    bool pwmZerado_ = false;

    enum class Estado { Parado, MudandoParaIndefinido, AtuandoSentido };
    Estado estado_ = Estado::Parado;

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
        return pwmZerado_ && (millis() - tempoPwmZero_ >= 1000);
    }

    void aplicarReles(Sentido sentido)
    {
        switch (sentido)
        {
            case Sentido::Horario:
                digitalWrite(pinKD_, LOW);
                digitalWrite(pinKE_, LOW);
                break;
            case Sentido::Antihorario:
                digitalWrite(pinKD_, HIGH);
                digitalWrite(pinKE_, HIGH);
                break;
            case Sentido::Indefinido:
                digitalWrite(pinKD_, LOW);
                digitalWrite(pinKE_, HIGH);
                break;
        }
        sentidoAtual_ = sentido;
    }

public:
    Motor(uint8_t pinKD, uint8_t pinKE, PWM_PB1_STM32_S& pwm)
        : pinKD_(pinKD), pinKE_(pinKE), pwm_(pwm)
    {
        pinMode(pinKD_, OUTPUT);
        pinMode(pinKE_, OUTPUT);
        aplicarReles(Sentido::Indefinido);
    }

    Sentido obterSentido() const { return sentidoAtual_; }

    void ligar(Sentido sentido)
    {
        if (sentido == Sentido::Indefinido) 
            return;
        if (sentidoAtual_ == Sentido::Indefinido)
        {
            if (podeAtuarRele())
            {
                aplicarReles(sentido);
                estado_ = Estado::Parado;
            }
        }
        else if (sentidoAtual_ != sentido)
        {
            sentidoAlvo_ = sentido;
            aplicarReles(Sentido::Indefinido);
            pwm_.definirDpwmRampa(0);
            estado_ = Estado::MudandoParaIndefinido;
        }
    }

    void desligar()
    {
        if (sentidoAtual_ != Sentido::Indefinido)
        {
            sentidoAlvo_ = Sentido::Indefinido;
            pwm_.definirDpwmRampa(0);
            estado_ = Estado::MudandoParaIndefinido;
        }
    }

    void monitorar()
    {
        atualizarPwmZero();
        switch (estado_)
        {
            case Estado::MudandoParaIndefinido:
                if (podeAtuarRele() && pwm_.obterDpwmAtual() == 0)
                {   // Aplicar sentido alvo
                    if (sentidoAlvo_ != Sentido::Indefinido)
                        aplicarReles(sentidoAlvo_);
                    estado_ = Estado::Parado;
                    sentidoAlvo_ = Sentido::Indefinido;
                }
                break;
            case Estado::Parado:
            case Estado::AtuandoSentido:
                // Nada a fazer aqui
                break;
        }
    }
};
