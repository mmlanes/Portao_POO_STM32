#pragma once

#include <Arduino.h>
#include "IChave.h"
#include <initializer_list>

/// Combina várias chaves e verifica se estão nos estados desejados
class ChavesCombinadas2
{
private:
    IChave** chaves_;
    bool* estadosDesejados_;
    size_t tamanho_;

public:
    // Construtor com arrays C
    ChavesCombinadas2(IChave* chaves[], bool estadosDesejados[], size_t tamanho)
        : chaves_(chaves), estadosDesejados_(estadosDesejados), tamanho_(tamanho)
    {
        if (tamanho_ == 0)
            Serial2.println("Erro: tamanho do array inválido!");
    }

    // Novo construtor que aceita lista de inicialização
    ChavesCombinadas2(std::initializer_list<IChave*> chaves, std::initializer_list<bool> estados)
    {
        tamanho_ = chaves.size();
        if (tamanho_ == 0)
        {
            Serial2.println("Erro: lista de chaves vazia!");
            chaves_ = nullptr;
            estadosDesejados_ = nullptr;
            return;
        }

        // Aloca arrays temporários
        chaves_ = new IChave*[tamanho_];
        estadosDesejados_ = new bool[tamanho_];

        size_t i = 0;
        for (IChave* c : chaves)
            chaves_[i++] = c;

        i = 0;
        for (bool e : estados)
            estadosDesejados_[i++] = e;
    }

    ~ChavesCombinadas2()
    {
        delete[] chaves_;
        delete[] estadosDesejados_;
    }

    /// Verifica se todas as chaves estão no estado esperado
    bool todasConformes() const
    {
        for (size_t i = 0; i < tamanho_; i++)
            if (chaves_[i]->estaAtiva() != estadosDesejados_[i])
                return false;
        return true;
    }
};
