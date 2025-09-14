#pragma once
#include <Arduino.h>

class MediaMovel
{
private:
    float* buffer_;          // vetor para armazenar amostras
    uint16_t capacidade_;    // número máximo de amostras (tamanho da janela)
    uint16_t quantidade_;    // quantidade atual de amostras preenchidas
    uint16_t indice_;        // posição do próximo valor a ser escrito
    float soma_;             // soma atual dos valores (para não recalcular toda vez)

public:
    // Construtor: define a quantidade de amostras da média
    MediaMovel(uint16_t capacidade) 
        : capacidade_(capacidade), quantidade_(0), indice_(0), soma_(0.0f)
    {
        buffer_ = new float[capacidade_];
        for (uint16_t i = 0; i < capacidade_; i++) 
            buffer_[i] = 0.0f;
    }

    // Destrutor: libera memória
    ~MediaMovel() 
    {
        delete[] buffer_;
    }

    // Adiciona uma nova amostra
    void adicionar(float valor) 
    {
        if (quantidade_ < capacidade_) 
        {
            soma_ += valor;
            buffer_[indice_++] = valor;
            quantidade_++;
        } 
        else 
        {
            // remove a mais antiga e adiciona a nova
            soma_ -= buffer_[indice_];
            soma_ += valor;
            buffer_[indice_] = valor;
            indice_ = (indice_ + 1) % capacidade_;
        }
    }

    // Retorna a média atual
    float obterMedia() const 
    {
        if (quantidade_ == 0) 
            return 0.0f;
        return soma_ / quantidade_;
    }

    // Retorna a quantidade de amostras já inseridas
    uint16_t obterQuantidade() const 
    {
        return quantidade_;
    }

    // Zera todo o histórico
    void zerar() 
    {
        for (uint16_t i = 0; i < capacidade_; i++) 
            buffer_[i] = 0.0f;
        soma_ = 0.0f;
        quantidade_ = 0;
        indice_ = 0;
    }
};
