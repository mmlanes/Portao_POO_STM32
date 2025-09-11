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

public:
    Protecao(Portao& portao, 
             Variavel<float>& iProtecao,
             Variavel<bool>& protecaoEncoderParadoAtuada,
             Variavel<bool>& protecaoSobrecorrenteAtuada)
        : portao_(portao), iProtecao_(iProtecao),
          protecaoEncoderParadoAtuada_(protecaoEncoderParadoAtuada),
          protecaoSobrecorrenteAtuada_(protecaoSobrecorrenteAtuada)
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
        if (portao_.obterMotor().obterImedio() >= iProtecao_.obterValor())
        {
            portao_.protecaoSobrecorrente();
            protecaoSobrecorrenteAtuada_.definirValor(true);
        }
        // Proteção por encoder parado
        if (portao_.obterEncoder().estaParado())
        {
            portao_.protecaoEncoderParado();
            protecaoEncoderParadoAtuada_.definirValor(true);
        }
    }
};
