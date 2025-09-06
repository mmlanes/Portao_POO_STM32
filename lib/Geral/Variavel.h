#pragma once
#include <Arduino.h>
#include <type_traits>
#include <math.h>
#include <vector>
#include <algorithm>

// =========================
// Classe base
// =========================
class VariavelBase 
{
public:
    virtual ~VariavelBase() = default;

    virtual String obterNome() const = 0;
    virtual bool ehPersistente() const = 0;
    virtual String paraString() const = 0;

    // Lista global única
    static std::vector<VariavelBase*> _todas;

    static void registrar(VariavelBase* v) 
    {
        _todas.push_back(v);
    }

    static void remover(VariavelBase* v) 
    {
        _todas.erase(std::remove(_todas.begin(), _todas.end(), v), _todas.end());
    }

    static String todasPersistentesParaString() 
    {
        String resultado = "";
        for (auto v : _todas) 
        {
            if (v->ehPersistente()) 
            {
                if (resultado.length() > 0) 
                    resultado += ";";
                resultado += v->obterNome() + "=" + v->paraString();
            }
        }
        return resultado;
    }

    // 🔹 Novo método: substitui $nome$ por valor
static String substituirVariaveis(const String& texto) 
{
    String resultado = texto;
    for (auto v : _todas) 
    {
        String padrao = "$" + v->obterNome() + "$";
        int pos = resultado.indexOf(padrao);
        while (pos != -1) 
        {
            // Divide a string em antes + substituição + depois
            resultado = resultado.substring(0, pos) 
                        + v->paraString() 
                        + resultado.substring(pos + padrao.length());
            // Procura novamente a partir do final da substituição
            pos = resultado.indexOf(padrao, pos + v->paraString().length());
        }
    }
    return resultado;
}

};

inline std::vector<VariavelBase*> VariavelBase::_todas;

// =========================
// Classe template genérica
// =========================
template<typename T>
class Variavel : public VariavelBase {
private:
    String _nome;
    T _valor;
    T _min;
    T _max;
    T _passo;
    bool _persistente;

public:
    Variavel(String nome, T valorInicial, T minimo, T maximo, T passo, bool persistente = false)
        : _nome(nome), _valor(valorInicial), _min(minimo), _max(maximo), _passo(passo), _persistente(persistente)
    {
        if (_valor < _min) _valor = _min;
        if (_valor > _max) _valor = _max;
        VariavelBase::registrar(this);
    }

    ~Variavel() 
    {
        VariavelBase::remover(this);
    }

    // Métodos de manipulação
    void incrementar(uint8_t acelerador = 1) 
    {
        if constexpr (std::is_same<T, bool>::value) 
            _valor = true;
        else 
        {
            T incremento = acelerador * _passo;
            if (_valor + incremento <= _max)
                _valor += incremento;
            else if (_valor + _passo <= _max)
                _valor += _passo;
        }
    }

    void decrementar(uint8_t acelerador = 1) 
    {
        if constexpr (std::is_same<T, bool>::value) 
            _valor = false;
        else 
        {
            T incremento = acelerador * _passo;
            if (_valor - incremento >= _min)
                _valor -= incremento;
            else if (_valor - _passo >= _min)
                _valor -= _passo;
        }
    }

    T obterValor() const 
    { 
        return _valor; 
    }

    void definirValor(T v) 
    {
        if (v < _min) v = _min;
        if (v > _max) v = _max;
        _valor = v;
    }

    T obterMinimo() const 
    { 
        return _min; 
    }

    T obterMaximo() const 
    { 
        return _max; 
    }

    String obterNome() const override 
    { 
        return _nome; 
    }

    bool ehPersistente() const override 
    { 
        return _persistente; 
    }

    // String paraString2() const override 
    // {
    //     if constexpr (std::is_same<T, float>::value) 
    //         return floatParaCientifico(_valor);
    //     else if constexpr (std::is_same<T, bool>::value) 
    //         return _valor ? "true" : "false";
    //     else 
    //         return String(_valor);
    // }

    String paraString() const override 
    {
        if constexpr (std::is_same<T, float>::value) {
            return String(_valor, 5);  // 3 casas decimais
        } else if constexpr (std::is_same<T, bool>::value) {
            return _valor ? "1" : "0"; //return _valor ? "true" : "false";
        } else {
            return String(_valor);
        }
    }

    // // Converter float para notação científica (3 algarismos significativos)
    // static String floatParaCientifico2(float valor) 
    // {
    //     if (valor == 0.0f) 
    //         return "0.00e+0";
    //     int expoente = (int)floor(log10(fabs(valor)));
    //     float mantissa = valor / pow(10, expoente);
    //     char buffer[16];
    //     snprintf(buffer, sizeof(buffer), "%.2fe%d", mantissa, expoente);
    //     return String(buffer);
    // }

    // static String floatParaCientifico1(float valor) 
    // {
    //     if (valor == 0.0f) return "0.00e+0";

    //     int expoente = (int)floor(log10(fabs(valor)));
    //     float mantissa = valor / pow(10, expoente);

    //     // garante 2 casas decimais e expoente sempre positivo ou negativo
    //     char buffer[20];
    //     snprintf(buffer, sizeof(buffer), "%.2fe%d", mantissa, expoente);
    //     return String(buffer);
    // }
    // static String floatParaCientifico(float valor) 
    // {
    //     if (valor == 0.0f) return "0.00e+0";

    //     int expoente = (int)floor(log10(fabs(valor)));
    //     float mantissa = valor / pow(10, expoente);

    //     char buffer[20];
    //     // dtostrf(valor, larguraMinima, casasDecimais, buffer)
    //     // Aqui: 3 casas decimais para mantissa, sempre positivo
    //     snprintf(buffer, sizeof(buffer), "%.3fe%d", mantissa, expoente);
    //     return String(buffer);
    // }
};
