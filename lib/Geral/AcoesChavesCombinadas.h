#pragma once

#include <Arduino.h>
#include "ModosOperacao.h"
#include "ChavesCombinadas.h"
#include <vector>

class AcoesChavesCombinadas
{
private:
    ChavesCombinadas combinacao_;     // <-- Agora usamos a classe combinadora
    ModosOperacao* modoAtuar_;
    void (*funcaoAcao_)(uint8_t);
    uint16_t tempoAtivacaoMs_;
    bool acaoRepetir_;
    uint16_t tempoAcelerador1Ms_;
    uint8_t acelerador1_;
    uint16_t tempoAcelerador2Ms_;
    uint8_t acelerador2_;
    unsigned long tempoInicioAtuacaoMs_;
    unsigned long tempoUltimaAtuacaoMs_;
    bool jaExecutou_;   
    static std::vector<AcoesChavesCombinadas*> instancias_;

public:
    AcoesChavesCombinadas(
        const ChavesCombinadas& combinacao, 
        ModosOperacao* modoAtuar = nullptr,
        void (*funcaoAcao)(uint8_t) = nullptr, uint16_t tempoAtivacaoMs = 0, bool repetir = false,
        uint16_t tempoAcelerador1Ms = 5000, uint8_t acelerador1 = 10,
        uint16_t tempoAcelerador2Ms = 10000, uint8_t acelerador2 = 100)
        : combinacao_(combinacao), modoAtuar_(modoAtuar), funcaoAcao_(funcaoAcao), 
          tempoAtivacaoMs_(tempoAtivacaoMs), acaoRepetir_(repetir),
          tempoAcelerador1Ms_(tempoAcelerador1Ms), acelerador1_(acelerador1),
          tempoAcelerador2Ms_(tempoAcelerador2Ms), acelerador2_(acelerador2),
          tempoInicioAtuacaoMs_(0), tempoUltimaAtuacaoMs_(0),
          jaExecutou_(false)
    {
        instancias_.push_back(this);
    }

    void atuar(ModosOperacao* modo = nullptr) 
    { 
        if (modoAtuar_ != modo && modoAtuar_ != nullptr)
            return; // não está no modo correto

        if (combinacao_.todasConformes()) 
        { 
            if (tempoUltimaAtuacaoMs_ == 0) 
            { 
                tempoInicioAtuacaoMs_ = millis();
                tempoUltimaAtuacaoMs_ = tempoInicioAtuacaoMs_;
                jaExecutou_ = false;
            } 
            else if (!jaExecutou_ || acaoRepetir_) 
            { 
                unsigned long decorrido_ultima = millis() - tempoUltimaAtuacaoMs_; 
                unsigned long decorrido_total = millis() - tempoInicioAtuacaoMs_;
                if (decorrido_ultima >= tempoAtivacaoMs_) 
                { 
                    if (funcaoAcao_) 
                    { 
                        uint8_t acelerador = 1;
                        if (decorrido_total >= tempoAcelerador2Ms_) 
                            acelerador = acelerador2_; 
                        else if (decorrido_total >= tempoAcelerador1Ms_) 
                            acelerador = acelerador1_; 
                        funcaoAcao_(acelerador);
                    } 
                    if (acaoRepetir_) 
                        tempoUltimaAtuacaoMs_ = millis(); 
                    else 
                        jaExecutou_ = true;
                } 
            } 
        } 
        else 
        { 
            tempoUltimaAtuacaoMs_ = 0;
            jaExecutou_ = false; 
        } 
    }

    static void atuarTodas(ModosOperacao* modo = nullptr) 
    {
        for (auto* instancia : instancias_)
            instancia->atuar(modo);
    }
};

// 🔹 Definição da lista estática
std::vector<AcoesChavesCombinadas*> AcoesChavesCombinadas::instancias_;
