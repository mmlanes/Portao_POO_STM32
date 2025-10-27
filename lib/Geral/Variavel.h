#pragma once
#include <Arduino.h>
#include <vector>
#include <algorithm>

// =========================
// Enum de tipos suportados
// =========================
enum class TipoVariavel {
    INT32,
    UINT32,
    UINT16,
    FLOAT,
    BOOL,
    STRING,
    DESCONHECIDO
};

// =========================
// Classe base
// =========================
class VariavelBase {
public:
    virtual ~VariavelBase() = default;
    virtual TipoVariavel tipo() const = 0;
    virtual String obterNome() const = 0;
    virtual bool ehPersistente() const = 0;
    virtual String paraString() const = 0;

    static std::vector<VariavelBase*> _todas;

    static void registrar(VariavelBase* v) {
        _todas.push_back(v);
    }

    static void remover(VariavelBase* v) 
    {
        _todas.erase(std::remove(_todas.begin(), _todas.end(), v), _todas.end());
    }

    static String todasPersistentesParaString() {
        String resultado = "";
        for (auto v : _todas) {
            if (v->ehPersistente()) {
                if (resultado.length() > 0)
                    resultado += ";";
                resultado += v->obterNome() + "=" + v->paraString();
            }
        }
        return resultado;
    }

    static String substituirVariaveis(const String& texto) {
        if (texto.length() == 0) return texto;
        String resultado = texto;
        for (auto v : _todas) {
            String padrao = "$" + v->obterNome() + "$";
            int pos = resultado.indexOf(padrao);
            while (pos >= 0) {
                String valor = v->paraString();
                resultado = resultado.substring(0, pos) + valor +
                            resultado.substring(pos + padrao.length());
                pos = resultado.indexOf(padrao, pos + valor.length());
            }
        }
        return resultado;
    }
};

std::vector<VariavelBase*> VariavelBase::_todas;

// =========================
// Template genérico → aritméticos
// =========================
template<typename T>
class Variavel : public VariavelBase 
{
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

    void incrementar(uint8_t acelerador = 1) 
    {
        T incremento = acelerador * _passo;
        if (_valor + incremento <= _max)
            _valor += incremento;
        else if (_valor + _passo <= _max)
            _valor += _passo;
    }

    void decrementar(uint8_t acelerador = 1) 
    {
        T incremento = acelerador * _passo;
        if (_valor - incremento >= _min)
            _valor -= incremento;
        else if (_valor - _passo >= _min)
            _valor -= _passo;
    }

    T obterValor() const { return _valor; }

    void definirValor(T v) 
    {
        if (v < _min) v = _min;
        if (v > _max) v = _max;
        _valor = v;
    }

    T obterMinimo() const { return _min; }
    T obterMaximo() const { return _max; }

    String obterNome() const override { return _nome; }
    bool ehPersistente() const override { return _persistente; }

    // String paraStringCientifico() const override {
    //     if (std::is_same<T, float>::value) {
    //         if (_valor == 0.0f) return "0.00e+0";
    //         if (_valor >= 0.1f && _valor < 100.0f) return String(_valor, 1);
    //         float val = _valor;
    //         int expoente = 0;
    //         bool negativo = false;
    //         if (val < 0.0f) {negativo = true; val = -val;}
    //         if (val >= 10.0f)
    //             while (val >= 10.0f) {val /= 10.0f; expoente++;}
    //         else if (val < 1.0f)
    //             while (val < 1.0f) {val *= 10.0f; expoente--;}
    //         if (negativo) val = -val;
    //         char buf[32];
    //         int pos = 0;
    //         if (val < 0.0f) {buf[pos++] = '-'; val = -val;}
    //         int intPart = (int)val;
    //         int fracPart = (int)((val - intPart) * 100);
    //         pos += sprintf(buf + pos, "%d.%02d", intPart, fracPart);
    //         if (expoente != 0)
    //             pos += sprintf(buf + pos, "e%+d", expoente);
    //         return String(buf);
    //     }
    //     else {
    //         return String(_valor);
    //     }
    // }

    String paraString() const override 
    {
        if (std::is_same<T, float>::value) 
        {
            float val = _valor;
            String sign = (val < 0.0f) ? "-" : "";
            val = fabs(val);

            if (val == 0.0f) return "0.0";

            struct Prefix { float threshold; const char* suffix; int decimals; };
            const Prefix prefixes[] = {
                {1.0e6f, "M", 0},
                {1.0e3f, "k", 0},
                {1.0f, "", 1},
                {1.0e-3f, "m", 1},
                {1.0e-6f, "u", 1}
            };

            for (const auto& p : prefixes)
            {
                if (val >= p.threshold) 
                {
                    float displayVal = val;
                    if (p.suffix[0] == 'k') displayVal /= 1.0e3f;
                    if (p.suffix[0] == 'M') displayVal /= 1.0e6f;
                    if (p.suffix[0] == 'm') displayVal *= 1000.0f;
                    if (p.suffix[0] == 'u') displayVal *= 1000000.0f;

                    return sign + String(displayVal, p.decimals) + p.suffix;
                }
            }

            // Para valores menores que 1u
            return sign + String(val, 3);
        } 
        else 
        {
            return String(_valor);
        }
    }

    TipoVariavel tipo() const override 
    {
        if (std::is_same<T, int32_t>::value) return TipoVariavel::INT32;
        else if (std::is_same<T, uint32_t>::value) return TipoVariavel::UINT32;
        else if (std::is_same<T, uint16_t>::value) return TipoVariavel::UINT16;
        else if (std::is_same<T, float>::value) return TipoVariavel::FLOAT;
        else return TipoVariavel::DESCONHECIDO;
    }
};

// =========================
// Especialização para String
// =========================
template<>
class Variavel<String> : public VariavelBase 
{
private:
    String _nome;
    String _valor;
    bool _persistente;

public:
    Variavel(String nome, String valorInicial, bool persistente = false)
        : _nome(nome), _valor(valorInicial), _persistente(persistente)
    {
        VariavelBase::registrar(this);
    }

    ~Variavel() 
    {
        VariavelBase::remover(this);
    }

    void incrementar(uint8_t = 1) { /* ignorado */ }
    void decrementar(uint8_t = 1) { /* ignorado */ }

    String obterValor() const { return _valor; }
    void definirValor(const String& v) { _valor = v; }

    String obterNome() const override { return _nome; }
    bool ehPersistente() const override { return _persistente; }

    String paraString() const override { return _valor; }

    TipoVariavel tipo() const override { return TipoVariavel::STRING; }
};

// =========================
// Especialização para bool
// =========================
template<>
class Variavel<bool> : public VariavelBase 
{
private:
    String _nome;
    bool _valor;
    bool _persistente;

public:
    Variavel(String nome, bool valorInicial, bool persistente = false)
        : _nome(nome), _valor(valorInicial), _persistente(persistente)
    {
        VariavelBase::registrar(this);
    }

    ~Variavel() 
    {
        VariavelBase::remover(this);
    }

    void incrementar(uint8_t = 1) { _valor = true; }
    void decrementar(uint8_t = 1) { _valor = false; }

    bool obterValor() const { return _valor; }
    void definirValor(bool v) { _valor = v; }

    String obterNome() const override { return _nome; }
    bool ehPersistente() const override { return _persistente; }

    String paraString() const override { return _valor ? "1" : "0"; }

    TipoVariavel tipo() const override { return TipoVariavel::BOOL; }
};