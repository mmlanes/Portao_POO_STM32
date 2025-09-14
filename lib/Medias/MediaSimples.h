#pragma once
#include <Arduino.h>

class MediaSimples
{
private:
    uint32_t soma_;        // acumula os valores
    uint16_t quantidade_;  // número de amostras

public:
    MediaSimples() : soma_(0), quantidade_(0) {}

    // Adiciona um valor ao acumulador
    void adicionar(float valor) 
    {
        soma_ += valor;
        quantidade_++;
    }

    // Retorna a média (como float)
    float obterMedia() const 
    {
        if (quantidade_ == 0) return 0.0f;
        return soma_ / quantidade_;
    }

    // Retorna a quantidade de amostras
    uint16_t obterQuantidade() const 
    {
        return quantidade_;
    }

    // Retorna a média e zera acumulador e contador
    float obterMediaEZerar() 
    {
        float media = obterMedia();
        zerar();
        return media;
    }

    // Zera sem calcular
    void zerar() 
    {
        soma_ = 0;
        quantidade_ = 0;
    }
};
