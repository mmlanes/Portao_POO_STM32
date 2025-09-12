#pragma once

#include <Arduino.h>
#include <functional>
#include "Variavel.h"
#include "Portao.h"

class Protecao
{
private:
    Portao& portao_;
    Variavel<float>& iProtecao_;
    Variavel<bool>& protecaoEncoderParadoAtuada_;
    Variavel<bool>& protecaoSobrecorrenteAtuada_;
    Variavel<bool>& usarProtecaoEncoderParado_;
    Variavel<bool>& usarProtecaoSobrecorrente_;

public:
    Protecao(Portao& portao, 
             Variavel<float>& iProtecao,
             Variavel<bool>& protecaoEncoderParadoAtuada,
             Variavel<bool>& protecaoSobrecorrenteAtuada,
             Variavel<bool>& usarProtecaoEncoderParado,
             Variavel<bool>& usarProtecaoSobrecorrente)
        : portao_(portao), iProtecao_(iProtecao),
          protecaoEncoderParadoAtuada_(protecaoEncoderParadoAtuada),
          protecaoSobrecorrenteAtuada_(protecaoSobrecorrenteAtuada),
          usarProtecaoEncoderParado_(usarProtecaoEncoderParado),
          usarProtecaoSobrecorrente_(usarProtecaoSobrecorrente)
    {
        protecaoEncoderParadoAtuada_.definirValor(false);
        protecaoSobrecorrenteAtuada_.definirValor(false);
    }

    void liberarProtecaoEncoderParado() { protecaoEncoderParadoAtuada_.definirValor(false); }
    void liberarProtecaoSobrecorrente() { protecaoSobrecorrenteAtuada_.definirValor(false); }
    bool protecaoEncoderParadoAtuada() const { return protecaoEncoderParadoAtuada_.obterValor(); }
    bool protecaoSobrecorrenteAtuada() const { return protecaoSobrecorrenteAtuada_.obterValor(); }

    void monitorar()
    {
        // Proteção por sobrecorrente
        if (usarProtecaoEncoderParado_.obterValor())
        {
            if (portao_.obterMotor().obterUltimoImedio() >= iProtecao_.obterValor())
                protecaoSobrecorrenteAtuada_.definirValor(true);
        }

        // Proteção por encoder parado
        if (usarProtecaoEncoderParado_.obterValor())
        {
            if (portao_.obterEncoder().estaParado())
                protecaoEncoderParadoAtuada_.definirValor(true);
            else
                protecaoEncoderParadoAtuada_.definirValor(false);        
        }

    }
};
