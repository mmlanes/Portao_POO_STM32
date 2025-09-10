#pragma once

#include <Arduino.h>

class ModosOperacao 
{
private:
    String nomeModo;
    static int8_t modoAtual_;
    static std::vector<ModosOperacao*>& obterListaModos() 
    {
        static std::vector<ModosOperacao*> lista;
        return lista;
    }
public:
    ModosOperacao(String nome)
    : nomeModo(nome)
    {
        obterListaModos().push_back(this);
        if (modoAtual_ == -1) 
            modoAtual_ = 0; // inicia no primeiro modo criado
    }
    String obterNome(void) const
    {
        return nomeModo;
    }
    static ModosOperacao* obterModoAtual(void)
    {
        auto& lista = obterListaModos();
        if (modoAtual_ < 0 || modoAtual_ >= (int8_t)lista.size())
            return nullptr;
        return lista[modoAtual_];
    }
    static void definirModoAtual(ModosOperacao* modo)
    {
        auto& lista = obterListaModos();
        for (size_t i = 0; i < lista.size(); i++)
        {
            if (lista[i] == modo)
            {
                modoAtual_ = i;
                return;
            }
        }
    }
    static void definirModoAtualPorPosicao(uint8_t PosModo)
    {
        auto& lista = obterListaModos();
        if (PosModo < lista.size())
            modoAtual_ = PosModo;
    }
    static int8_t obterPosicaoModoAtual(void)
    {
        return modoAtual_;
    }
    static void modoSeguinte(void)
    {
        auto& lista = obterListaModos();
        if (lista.size() == 0)
            modoAtual_ = -1;
        else if (modoAtual_ < (int8_t)(lista.size()-1))
            modoAtual_++;
        else
            modoAtual_ = 0;
    }
    static void modoAnterior(void)
    {
        auto& lista = obterListaModos();
        if (lista.size() == 0)
            modoAtual_ = -1;
        else if (modoAtual_ > 0)
            modoAtual_--;
        else
            modoAtual_ = lista.size()-1;
    }
};

int8_t ModosOperacao::modoAtual_ = -1;