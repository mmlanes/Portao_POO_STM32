#pragma once
#include <Arduino.h>

// ----------------------- Média Simples -----------------------
class MediaSimples 
{
private:
    float soma_;
    uint16_t quantidade_;
    uint16_t capacidade_;
    float ultimaMedia_;

public:
    MediaSimples(uint16_t capacidade)
        : soma_(0), quantidade_(0), capacidade_(capacidade), ultimaMedia_(0) 
    {}

    void adicionar(float valor) 
    {
        soma_ += valor;
        quantidade_++;
        if (quantidade_ == capacidade_) 
            ultimaMedia_ = soma_ / quantidade_;
    }

    bool completa() const 
    {
        return quantidade_ >= capacidade_;
    }

    float obterMedia() const 
    {
        if (quantidade_ == 0) 
            return 0.0f;
        return soma_ / quantidade_;
    }

    float obterUltimaMedia() const 
    {
        return ultimaMedia_;
    }

    void zerar() 
    {
        soma_ = 0;
        quantidade_ = 0;
    }
};

// ----------------------- Média Móvel -----------------------
class MediaMovel 
{
private:
    float* buffer_;
    uint16_t capacidade_;
    uint16_t quantidade_;
    uint16_t indice_;
    float soma_;

public:
    MediaMovel(uint16_t capacidade)
        : capacidade_(capacidade), quantidade_(0), indice_(0), soma_(0.0f) 
    {
        buffer_ = new float[capacidade_];
        for (uint16_t i = 0; i < capacidade_; i++) 
            buffer_[i] = 0.0f;
    }

    ~MediaMovel() 
    {
        delete[] buffer_;
    }

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
            soma_ -= buffer_[indice_];
            soma_ += valor;
            buffer_[indice_] = valor;
            indice_ = (indice_ + 1) % capacidade_;
        }
    }

    float obterMedia() const 
    {
        if (quantidade_ == 0) 
            return 0.0f;
        return soma_ / quantidade_;
    }

    void zerar() 
    {
        for (uint16_t i = 0; i < capacidade_; i++) 
            buffer_[i] = 0.0f;
        soma_ = 0.0f;
        quantidade_ = 0;
        indice_ = 0;
    }
};

// ----------------------- Classe Composta -----------------------
class MediaComposta 
{
private:
    MediaSimples mediaSimples_;
    MediaMovel mediaMovel_;
    float maiorLeitura_;

public:
    MediaComposta(uint16_t qtdSimples, uint16_t qtdMovel)
        : mediaSimples_(qtdSimples), mediaMovel_(qtdMovel), maiorLeitura_(0.0f) 
    {}

    // Adiciona novo valor
    void adicionar(float valor) 
    {
        mediaSimples_.adicionar(valor);
        if (valor > maiorLeitura_) 
            maiorLeitura_ = valor;

        if (mediaSimples_.completa()) 
        {
            float media = mediaSimples_.obterMedia();
            mediaMovel_.adicionar(media);
            mediaSimples_.zerar();
        }
    }

    // Última média simples
    float obterMediaSimples() const 
    {
        return mediaSimples_.obterMedia();
    }

    // Média móvel
    float obterMediaMovel() const 
    {
        return mediaMovel_.obterMedia();
    }

    // Última média simples completa
    float obterUltimaMediaSimples() const 
    {
        return mediaSimples_.obterUltimaMedia();
    }

    // Maior valor inserido
    float obterMaiorLeitura() const 
    {
        return maiorLeitura_;
    }

    // Zera tudo
    void zerar() 
    {
        mediaSimples_.zerar();
        mediaMovel_.zerar();
        maiorLeitura_ = 0.0f;
    }

    // Reseta apenas o maior leitura
    void resetMaiorLeitura(float valorReset = 0.0f) 
    {
        maiorLeitura_ = valorReset;
    }
};
