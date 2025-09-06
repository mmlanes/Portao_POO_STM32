#pragma once

#include <Arduino.h>

class ModosOperacao 
{
private:
    String nomeModo;
    inline static int8_t modoAtual_ = -1;
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
    static ModosOperacao* modoAtual(void)
    {
        auto& lista = obterListaModos();
        if (modoAtual_ < 0 || modoAtual_ >= lista.size())
            return nullptr;
        return lista[modoAtual_];
    }
    static void modoSeguinte(void)
    {
        auto& lista = obterListaModos();
        if (lista.size() == 0)
            modoAtual_ = -1;
        else if (modoAtual_ < (lista.size()-1))
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
