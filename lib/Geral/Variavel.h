#pragma once
#include <Arduino.h>
#include <type_traits>
#include <math.h>
#include <vector>
#include "IVariavel.h"

// Classe template para variáveis genéricas
template<typename T>
class Variavel : public IVariavel 
{
private:
    String _nome;
    T _valor;
    T _min;
    T _max;
    T _passo;
    bool _persistente;

    // lista global de todas as variáveis
    static std::vector<IVariavel*> _todas;

public:
    Variavel(String nome, T valorInicial, T minimo, T maximo, T passo, bool persistente = false)
        : _nome(nome), _valor(valorInicial), _min(minimo), _max(maximo), _passo(passo), _persistente(persistente)
    {
        if (_valor < _min) _valor = _min;
        if (_valor > _max) _valor = _max;
        _todas.push_back(this);
    }

    ~Variavel() 
    {
        // remove da lista global ao destruir
        _todas.erase(std::remove(_todas.begin(), _todas.end(), this), _todas.end());
    }

    // Métodos de manipulação
    void incrementar() 
    {
        if constexpr (std::is_same<T, bool>::value) 
        {
            _valor = true;
        } 
        else 
        {
            if (_valor + _passo <= _max)
                _valor += _passo;
        }
    }

    void decrementar() 
    {
        if constexpr (std::is_same<T, bool>::value) 
        {
            _valor = false;
        } 
        else 
        {
            if (_valor - _passo >= _min)
                _valor -= _passo;
        }
    }

    T obterValor() const { return _valor; }

    void definirValor(T v) 
    {
        if (v < _min) 
            v = _min;
        if (v > _max) 
            v = _max;
        _valor = v;
    }

    T obterMinimo() const { return _min; }
    T obterMaximo() const { return _max; }
    String obterNome() const override { return _nome; }
    bool ehPersistente() const override { return _persistente; }

    String paraString() const override 
    {
        if constexpr (std::is_same<T, float>::value)
            return floatParaCientifico(_valor);
        else if constexpr (std::is_same<T, bool>::value)
            return _valor ? "true" : "false";
        else
            return String(_valor);
    }

    // Converter float para notação científica (3 algarismos significativos)
    static String floatParaCientifico(float valor) 
    {
        if (valor == 0.0f) return "0.00e+0";
        int expoente = (int)floor(log10(fabs(valor)));
        float mantissa = valor / pow(10, expoente);
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%.2fe%d", mantissa, expoente);
        return String(buffer);
    }

    // Lista global de todas as variáveis (independente do tipo)
    static String todasPersistentesParaString() 
    {
        String resultado;
        for (auto v : _todas) 
        {
            if (v->ehPersistente()) 
            {
                if (resultado.length() > 0) resultado += ";";
                resultado += v->obterNome() + "=" + v->paraString();
            }
        }
        return resultado;
    }
};

// definição do membro estático
template<typename T>
std::vector<IVariavel*> Variavel<T>::_todas;
