#pragma once

#include <Arduino.h>
#include "ChavesSTM32.h"

class MonitorChave
{
private:
    ChavesSTM32& chaveMonitorada_;
    uint16_t tempoAtivacaoMs_;
    bool repetir_;
    unsigned long tempoUltimaAtuacaoMs_;

public:
    MonitorChave(ChavesSTM32& chave, uint16_t tempoAtivacaoMs, bool repetir = false)
    : chaveMonitorada_(chave), tempoAtivacaoMs_(tempoAtivacaoMs), repetir_(repetir)
    {}

    bool atuar(void)
    {
        if (chaveMonitorada_.estaAtiva())
        {
            if (tempoUltimaAtuacaoMs_ == 0)
                tempoUltimaAtuacaoMs_ = millis();
            else if ((millis() - tempoUltimaAtuacaoMs_) >= tempoAtivacaoMs_)
            {
                if (repetir_)
                    tempoUltimaAtuacaoMs_ = millis(); // Reinicia o temporizador para repetição
                return true; // Ação deve ser tomada
            }
        }
        else
            tempoUltimaAtuacaoMs_ = 0; // Reseta o temporizador se a chave não estiver ativa
        return false; // Nenhuma ação necessária
    }
};
