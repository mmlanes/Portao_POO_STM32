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
    PWM_PB1_STM32_S& pwm_;
    FastADC_PA0_STM32_S2& adc_;
    Variavel<float>& kAjuste_;
    Variavel<float>& Imedio_;
    uint16_t tempoEsperaRele_;
    uint16_t tempoAtualizacaoAutomaticaImedioMs_;
    uint8_t dpwmAlvo_;

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
    Motor(uint8_t pinKD, 
          uint8_t pinKE, 
          PWM_PB1_STM32_S& pwm, 
          FastADC_PA0_STM32_S2& adc,
          Variavel<float>& imedio,
          Variavel<float>& kADCAjusteValorReal,
          uint16_t tempoEsperaAcionarReles_ms = 1000,
          uint16_t tempoAtualizacaoAutomaticaImedio_ms = 500)
        : pinKD_(pinKD), 
          pinKE_(pinKE), 
          pwm_(pwm), 
          adc_(adc),
          Imedio_(imedio),
          kAjuste_(kADCAjusteValorReal),
          tempoEsperaRele_(tempoEsperaAcionarReles_ms), 
          tempoAtualizacaoAutomaticaImedioMs_(tempoAtualizacaoAutomaticaImedio_ms),
          dpwmAlvo_(0)
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

    void definirkADCAjusteValorReal(float kAjuste = 1.7e-3f) { kAjuste_.definirValor(kAjuste); }

    float obterkADCAjusteValorReal(void) { return kAjuste_.obterValor(); }

    float obterUltimoImedio(void)
    {
        return Imedio_.obterValor();
    }

    float obterImedio(float valorMinino = 0.1f)
    {
        int16_t MediaADC = adc_.obterMediaMovel();
        float Im = (float)MediaADC * kAjuste_.obterValor();
        //Serial2.println("MediaADC=" + String(MediaADC) + " kAjuste=" + String(kAjuste_.obterValor()) + " Imedio=" + String(Im));
        if (Im > -valorMinino && Im < valorMinino) 
            Im = 0.0f;  
        Imedio_.definirValor(Im);
        return Im;
    }

    void atualizacaoPeriodicaImedio(uint16_t intervaloMs)
    {
        static unsigned long ultimoUpdate_ = 0;  
        static float ultimaMedia_ = 0.0f;         

        if (intervaloMs == 0)
            return;

        unsigned long agora = millis();
        if (agora - ultimoUpdate_ >= intervaloMs)
        {
            obterImedio();
            //Serial2.println("Imedio atualizado: " + String(Imedio_.obterValor()) + " A");
            ultimoUpdate_ = agora;
        }
    }

    void monitorar()
    {
        atualizarPwmZero();
        //atualizacaoPeriodicaImedio(tempoAtualizacaoAutomaticaImedioMs_);
        atualizacaoPeriodicaImedio(3000);

         // Transições de estado
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
