#pragma once

#include <Arduino.h>
#include "IChave.h"
#include <vector>

/// Combina várias chaves e verifica se estão nos estados desejados
class ChavesCombinadas 
{
private:
    std::vector<IChave*> chaves_;
    std::vector<bool> estadosDesejados_;

public:
    ChavesCombinadas(const std::vector<IChave*>& chaves, const std::vector<bool>& estadosDesejados)
        : chaves_(chaves), estadosDesejados_(estadosDesejados)
    {
        if (chaves_.size() != estadosDesejados_.size())
            Serial2.println("Erro: número de chaves e estados não coincide!");
    }

    /// Verifica se todas as chaves estão no estado esperado
    bool todasConformes() {
        for (size_t i = 0; i < chaves_.size(); i++)
            if (chaves_[i]->estaAtiva() != estadosDesejados_[i])
                return false;
        return true;
    }
};
