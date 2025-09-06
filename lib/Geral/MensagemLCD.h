#pragma once
#include <Arduino.h>
#include <vector>
#include <algorithm>
#include "ModosOperacao.h"

class MensagemLCD 
{
private:
    ModosOperacao* modoAtual_;   // ponteiro para o modo associado
    String L1_;
    String L2_;
    String L3_;
    String L4_;

    // 🔹 Lista global de todas as instâncias
    static std::vector<MensagemLCD*>& obterListaMensagens() 
    {
        static std::vector<MensagemLCD*> lista;
        return lista;
    }

    // 🔹 Função auxiliar para substituir $modo$
    String substituirModo(const String& texto) const 
    {
        if (!modoAtual_) return texto; // se não tiver modo associado
        String resultado = texto;
        String padrao = "$modo$";
        int pos = resultado.indexOf(padrao);

        while (pos != -1)
        {
            resultado = resultado.substring(0, pos) 
                      + modoAtual_->obterNome() 
                      + resultado.substring(pos + padrao.length());

            // Procura outras ocorrências depois da substituição
            pos = resultado.indexOf(padrao, pos + modoAtual_->obterNome().length());
        }

        return resultado;
    }

public:
    // ✅ Construtor agora recebe ponteiro
    MensagemLCD(ModosOperacao* modo, String L1, String L2 = "", String L3 = "", String L4 = "")
        : modoAtual_(modo), L1_(L1), L2_(L2), L3_(L3), L4_(L4)
    {
        // Substitui $modo$ nas linhas já no construtor
        L1_ = substituirModo(L1_); 
        L2_ = substituirModo(L2_);
        L3_ = substituirModo(L3_);
        L4_ = substituirModo(L4_);

        // 🔹 Registra instância
        obterListaMensagens().push_back(this);
    }

    ~MensagemLCD() 
    {
        // 🔹 Remove instância ao destruir
        auto& lista = obterListaMensagens();
        lista.erase(std::remove(lista.begin(), lista.end(), this), lista.end());
    }

    // ✅ Agora retorna ponteiro, consistente com o resto do código
    ModosOperacao* modoAtual() const { return modoAtual_; }

    // 🔹 Métodos de acesso
    String obterL1() const { return L1_; }
    String obterL2() const { return L2_; }
    String obterL3() const { return L3_; }
    String obterL4() const { return L4_; }

    // 🔹 Acesso à lista de todas as instâncias
    static const std::vector<MensagemLCD*>& todas() 
    {
        return obterListaMensagens();
    }
};
