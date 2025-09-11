#pragma once

#include <Arduino.h>
#include "ModosOperacao.h"
#include "IChave.h"
#include "Portao.h"
#include "Variavel.h"

// Estrutura de uma ação combinada
struct AcaoCombinada {
    IChave* chaves[3];   // ponteiros para as chaves
    uint8_t estados;     // cada bit = estado desejado da chave (LSB = chaves[0])
    ModosOperacao* modo;
    void (*funcao)(uint8_t);
    uint16_t tempoAtivacaoMs;
    bool repetir;

    unsigned long tempoInicioMs;
    unsigned long tempoUltimaMs;
    bool jaExecutou;

    bool todasConformes() {
        for (uint8_t i = 0; i < 3; i++) {
            bool estadoDesejado = (estados >> i) & 0x01;
            if (chaves[i]->estaAtiva() != estadoDesejado) return false;
        }
        return true;
    }
};

// Gerenciador de todas as ações combinadas
class GerenciadorAcoes {
public:
    static const size_t MAX_ACOES = 50;
    AcaoCombinada acoes[MAX_ACOES];
    size_t numAcoes = 0;

    void adicionarAcao(const AcaoCombinada& acao) {
        if (numAcoes < MAX_ACOES) {
            acoes[numAcoes++] = acao;
        } else {
            Serial2.println("Erro: máximo de ações atingido!");
        }
    }

    void atuar(ModosOperacao* modo = nullptr) {
        unsigned long agora = millis();
        for (size_t i = 0; i < numAcoes; i++) {
            AcaoCombinada& a = acoes[i];

            if (a.modo != nullptr && a.modo != modo) continue;

            if (a.todasConformes()) {
                if (a.tempoUltimaMs == 0) {
                    a.tempoInicioMs = agora;
                    a.tempoUltimaMs = agora;
                    a.jaExecutou = false;
                } else if (!a.jaExecutou || a.repetir) {
                    unsigned long decorrido = agora - a.tempoUltimaMs;
                    if (decorrido >= a.tempoAtivacaoMs) {
                        if (a.funcao) a.funcao(1); // parâmetro acelerador genérico
                        if (a.repetir) a.tempoUltimaMs = agora;
                        else a.jaExecutou = true;
                    }
                }
            } else {
                a.tempoUltimaMs = 0;
                a.jaExecutou = false;
            }
        }
    }
};
