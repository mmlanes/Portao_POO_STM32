#pragma once
#include <Arduino.h>

// ----------------------- Média Simples -----------------------
class MediaSimples 
{
private:
    uint16_t capacidade_;
    float soma_;
    uint16_t quantidade_;
    float ultimaMedia_;
    float maiorAmostra_;
    float menorAmostra_;
    const float epsilon_ = 1.0e-10f; // Pequeno valor para comparação

public:
    MediaSimples(uint16_t capacidade)
        : capacidade_(capacidade), soma_(0), quantidade_(0), ultimaMedia_(0), maiorAmostra_(-epsilon_), menorAmostra_(epsilon_) 
    {}

    void adicionar(float valor) 
    {
        if (valor > maiorAmostra_) 
            maiorAmostra_ = valor;
        if (valor < menorAmostra_) 
            menorAmostra_ = valor;
        soma_ += valor;
        quantidade_++;
        if (quantidade_ == capacidade_) 
            ultimaMedia_ = soma_ / quantidade_;
    }

    void resetMaiorAmostra() { maiorAmostra_ = -epsilon_; }

    void resetMenorAmostra() { menorAmostra_ = epsilon_; }

    float obterMaiorAmostra() const { return maiorAmostra_; }

    float obterMenorAmostra() const { return menorAmostra_; }    

    bool completa() const { return quantidade_ >= capacidade_; }

    float obterMediaSimples() const 
    {
        if (quantidade_ == 0) 
            return 0.0f;
        return soma_ / quantidade_;
    }

    float obterUltimaMediaSimples() const 
    {
        return ultimaMedia_;
    }

    void zerarMediaSimples() 
    {
        soma_ = 0;
        quantidade_ = 0;
        ultimaMedia_ = 0;
        maiorAmostra_ = -epsilon_;
        menorAmostra_ = epsilon_;
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
    float maiorMediaSimples_;
    float menorMediaSimples_;
    const float epsilon_ = 1.0e-10f; // Pequeno valor para comparação

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
        if (valor > maiorMediaSimples_) 
            maiorMediaSimples_ = valor;
        if (valor < menorMediaSimples_) 
            menorMediaSimples_ = valor;
        if (quantidade_ < capacidade_) 
        {
            soma_ += valor;
            buffer_[indice_] = valor;
            indice_ = (indice_ + 1) % capacidade_;
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

    void resetMaiorMediaSimples() { maiorMediaSimples_ = -epsilon_; }

    void resetMenorMediaSimples() { menorMediaSimples_ = epsilon_; }

    float obterMaiorMediaSimples() const { return maiorMediaSimples_; }

    float obterMediaMovel() const 
    {
        if (quantidade_ == 0) 
            return 0.0f;
        return soma_ / quantidade_;
    }

    void zerarMediaMovel() 
    {
        for (uint16_t i = 0; i < capacidade_; i++) 
            buffer_[i] = 0.0f;
        soma_ = 0.0f;
        quantidade_ = 0;
        indice_ = 0;
        maiorMediaSimples_ = -epsilon_;
        menorMediaSimples_ = epsilon_;
    }

    uint16_t obterDimensaoMediaMovel() const { return quantidade_; }

    // Acesso ao buffer (índice 0 é o mais antigo)
    float operator[](uint16_t index) const 
    {
        if (index >= quantidade_) 
            return 0.0f;
        uint16_t realIndex = (indice_ + capacidade_ - quantidade_ + index) % capacidade_;
        return buffer_[realIndex];
    }
};

// ----------------------- Classe Composta -----------------------
class MediaComposta 
{
private:
    MediaSimples* mediaSimples_;
    MediaMovel* mediaMovel_;
    float maiorMediaMovel_;
    float menorMediaMovel_;
    float epsilon_ = 1.0e-10f; // Pequeno valor para comparação

public:
    MediaComposta(uint16_t qtdSimples, uint16_t qtdMovel)
        : mediaSimples_(new MediaSimples(qtdSimples)), mediaMovel_(new MediaMovel(qtdMovel)),
          maiorMediaMovel_(-epsilon_), menorMediaMovel_(epsilon_)
    {}

    // Adiciona novo valor
    void adicionar(float valor) 
    {
        mediaSimples_->adicionar(valor);

        if (mediaSimples_->completa()) 
        {
            float media = mediaSimples_->obterMediaSimples();
            if (media > maiorMediaMovel_) 
                maiorMediaMovel_ = media;
            if (media < menorMediaMovel_) 
                menorMediaMovel_ = media;
            mediaMovel_->adicionar(media);
            mediaSimples_->zerarMediaSimples();
        }
    }

    // Dá acesso aos objetos internos (se necessário) 
    MediaSimples obterObjetoMediaSimples() const { return *mediaSimples_; }
    MediaMovel obterObjetoMediaMovel() const { return *mediaMovel_; }

    // Média móvel
    float obterMediaMovel() const { return mediaMovel_->obterMediaMovel(); }
    float obterMaiorMediaMovel() const { return maiorMediaMovel_; }
    float obterMenorMediaMovel() const { return menorMediaMovel_; }
    void resetMaiorMediaMovel() { maiorMediaMovel_ = -epsilon_; }
    void resetMenorMediaMovel() { menorMediaMovel_ = epsilon_; }

    // Média simples
    float obterUltimaMediaSimples() const { return mediaSimples_->obterUltimaMediaSimples(); }
    float obterMaiorMediaSimples() const { return mediaSimples_->obterMaiorAmostra(); }
    float obterMenorMediaSimples() const { return mediaSimples_->obterMenorAmostra(); }
    void resetMaiorMediaSimples() { mediaSimples_->resetMaiorAmostra(); }
    void resetMenorMediaSimples() { mediaSimples_->resetMenorAmostra(); }

    // Amostras limites
    float obterMaiorAmostra() const { return mediaSimples_->obterMaiorAmostra(); }
    float obterMenorAmostra() const { return mediaSimples_->obterMenorAmostra(); }
    void resetMaiorAmostra() { mediaSimples_->resetMaiorAmostra(); }
    void resetMenorAmostra() { mediaSimples_->resetMenorAmostra(); }

    // Zera tudo
    void zerarMediaComposta() 
    {
        mediaSimples_->zerarMediaSimples();
        mediaMovel_->zerarMediaMovel();
        maiorMediaMovel_ = 0.0f;
        menorMediaMovel_ = 0.0f;
    }

    void imprimirSerialBufferMediaMovel() const 
    {
        uint16_t qtd = mediaMovel_->obterDimensaoMediaMovel();
        Serial2.println("qtd: " + String(qtd));
        for (uint16_t i = 0; i < qtd; i++)
            Serial2.print(String((*mediaMovel_)[i], 4) + " ");
            //Serial2.print(String(i) + ": " + String((*mediaMovel_)[i], 4) + " ");
    }
};
