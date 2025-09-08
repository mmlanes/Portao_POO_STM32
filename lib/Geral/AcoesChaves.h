#pragma once

#include <Arduino.h>
#include "ModosOperacao.h"

class AcoesChaves
{
private:
    ChaveSTM32& chave1_;
    ChaveSTM32& chave2_;
    ChaveSTM32& chave3_;
    bool estadoNecessarioChave2_;
    bool estadoNecessarioChave3_;
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
    bool jaExecutou_;   // <--- NOVO
    static std::vector<AcoesChaves*> instancias_;

public:
AcoesChaves(
    ChaveSTM32& chave1, ChaveSTM32& chave2, ChaveSTM32& chave3, 
    bool estadoNecessarioChave2 = false, bool estadoNecessarioChave3 = false, ModosOperacao* modoAtuar = nullptr,
    void (*funcaoAcao)(uint8_t) = nullptr, uint16_t tempoAtivacaoMs = 0, bool repetir = false,
    uint16_t tempoAcelerador1Ms = 5000, uint8_t acelerador1 = 10,
    uint16_t tempoAcelerador2Ms = 10000,
    uint8_t acelerador2 = 100)
    : chave1_(chave1), chave2_(chave2), chave3_(chave3),
      estadoNecessarioChave2_(estadoNecessarioChave2), estadoNecessarioChave3_(estadoNecessarioChave3),
      modoAtuar_(modoAtuar), funcaoAcao_(funcaoAcao), 
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

         // Verifica se a chave1 está ativa e as outras duas estão nos estados necessários
        if (chave1_.estaAtiva() && chave2_.estaAtiva()==estadoNecessarioChave2_ && chave3_.estaAtiva()==estadoNecessarioChave3_) 
        { 
            if (tempoUltimaAtuacaoMs_ == 0) 
            { 
                tempoInicioAtuacaoMs_ = millis();
                tempoUltimaAtuacaoMs_ = tempoInicioAtuacaoMs_;
                jaExecutou_ = false; // reset ao pressionar 
            } 
            else if (!jaExecutou_ || acaoRepetir_) // só executa se não tiver rodado ou se é pra repetir 
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
                        tempoUltimaAtuacaoMs_ = millis(); // reinicia pra repetir 
                    else 
                        jaExecutou_ = true; // trava até soltar 
                } 
            } 
        } 
        else 
        { 
            tempoUltimaAtuacaoMs_ = 0; // libera na próxima pressão 
            jaExecutou_ = false; 
        } 
    }

    // 🔹 Método estático: chama atuar() em todas as instâncias registradas
    static void atuarTodas(ModosOperacao* modo = nullptr)
    {
        for (auto* instancia : instancias_)
            instancia->atuar(modo);
    }
};

// 🔹 Definição da lista estática
std::vector<AcoesChaves*> AcoesChaves::instancias_;