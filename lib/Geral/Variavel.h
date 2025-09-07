#pragma once
#include <Arduino.h>
#include <type_traits>
#include <math.h>
#include <vector>
#include <algorithm>

enum class TipoVariavel {
    INT32,
    UINT32,
    UINT16,
    FLOAT,
    BOOL,
    DESCONHECIDO
};
// =========================
// Classe base
// =========================
class VariavelBase 
{
public:
    virtual ~VariavelBase() = default;
    virtual TipoVariavel tipo() const = 0; 
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
        if (texto.length() == 0) return texto;  // nada a substituir
        String resultado = texto;
        for (auto v : _todas) 
        {
            String padrao = "$" + v->obterNome() + "$";
            int pos = resultado.indexOf(padrao);
            while (pos >= 0) 
            {
                // Salva o valor da variável para evitar chamar toString() duas vezes
                String valor = v->paraString();
                // Substitui o padrão pela string da variável
                resultado = resultado.substring(0, pos) + valor + resultado.substring(pos + padrao.length());
                // Continua procurando a partir do fim da substituição
                pos = resultado.indexOf(padrao, pos + valor.length());
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

    // String paraString() const override 
    // {
    //     if constexpr (std::is_same<T, float>::value) 
    //     {
    //         if (_valor == 0.0f) return "0.00e+0";
    //         int expoente = (int)floor(log10(fabs(_valor)));
    //         float mantissa = _valor / pow(10, expoente);
    //         char buf[16];
    //         dtostrf(mantissa, 0, 2, buf);  // Arduino tem dtostrf para floats
    //         String s(buf);
    //         s += "e";
    //         if (expoente >= 0) s += "+";
    //         s += String(expoente);
    //         return s;
    //     } 
    //     else if constexpr (std::is_same<T, bool>::value) 
    //         return _valor ? "1" : "0";
    //     else 
    //         return String(_valor);
    // }

    // String paraString() const override //78,3%
    // {
    //     if constexpr (std::is_same<T, float>::value)
    //     {
    //         if (_valor == 0.0f)
    //             return "0.00e+0";
    //         float val = _valor;
    //         int expoente = 0;
    //         bool negativo = false;
    //         if (val < 0.0f)
    //         {
    //             negativo = true;
    //             val = -val;
    //         }
    //         if (val >= 10.0f)
    //         {
    //             while (val >= 10.0f)
    //             {
    //                 val /= 10.0f;
    //                 expoente++;
    //             }
    //         }
    //         else if (val < 1.0f)
    //         {
    //             while (val < 1.0f)
    //             {
    //                 val *= 10.0f;
    //                 expoente--;
    //             }
    //         }
    //         if (negativo)
    //             val = -val;
    //         char buf[16];
    //         dtostrf(val, 0, 2, buf);
    //         String s(buf);
    //         s += "e";
    //         if (expoente >= 0) 
    //             s += "+";
    //         s += String(expoente);
    //         return s;
    //     }
    //     else if constexpr (std::is_same<T, bool>::value)
    //         return _valor ? "1" : "0";
    //     else
    //         return String(_valor);
    // }


String paraString() const override //78,0%
{
    if constexpr (std::is_same<T, float>::value)
    {
        if (_valor == 0.0f) return "0.00e+0";
        float val = _valor;
        int expoente = 0;
        bool negativo = false;
        if (val < 0.0f) {negativo = true; val = -val;}
        if (val >= 10.0f)
            while (val >= 10.0f) {val /= 10.0f; expoente++;}
        else if (val < 1.0f)
            while (val < 1.0f) {val *= 10.0f; expoente--;}
        if (negativo) val = -val;
        char buf[32];
        int pos = 0;
        if (val < 0.0f) {buf[pos++] = '-'; val = -val;}
        int intPart = (int)val;
        int fracPart = (int)((val - intPart) * 100); // 2 casas decimais
        pos += sprintf(buf + pos, "%d.%02d", intPart, fracPart);
        if (expoente != 0)
            pos += sprintf(buf + pos, "e%+d", expoente);
        return String(buf);
    }
    else if constexpr (std::is_same<T, bool>::value)
        return _valor ? "1" : "0";
    else
        return String(_valor);
}

// String paraString() const override //77.3%
// {
//     if constexpr (std::is_same<T, float>::value)
//     {
//         if (_valor == 0.0f)
//             return "0";

//         float val = _valor;
//         int expoente = 0;

//         // Ajusta sinal
//         bool negativo = false;
//         if (val < 0.0f)
//         {
//             negativo = true;
//             val = -val;
//         }

//         // Normaliza mantissa entre 1 e 10
//         if (val >= 10.0f)
//         {
//             while (val >= 10.0f)
//             {
//                 val /= 10.0f;
//                 expoente++;
//             }
//         }
//         else if (val < 1.0f)
//         {
//             while (val < 1.0f)
//             {
//                 val *= 10.0f;
//                 expoente--;
//             }
//         }

//         if (negativo)
//             val = -val;

//         char buf[32];
//         int pos = 0;

//         // Sinal negativo
//         if (negativo)
//             buf[pos++] = '-';

//         // Parte inteira
//         int intPart = (int)val;
//         float fracPartF = val - intPart;

//         // Converte parte inteira
//         char intBuf[12];
//         int intLen = 0;
//         int tempInt = intPart;
//         if (tempInt == 0) intBuf[intLen++] = '0';
//         while (tempInt > 0)
//         {
//             intBuf[intLen++] = '0' + (tempInt % 10);
//             tempInt /= 10;
//         }
//         // Inverte
//         for (int i = intLen - 1; i >= 0; i--)
//             buf[pos++] = intBuf[i];

//         // Parte fracionária (5 casas decimais)
//         int fracPart = (int)(fracPartF * 100000 + 0.5f);
//         if (fracPart > 0)
//         {
//             buf[pos++] = '.';

//             int digits[5] = {0};
//             for (int i = 4; i >= 0; i--)
//             {
//                 digits[i] = fracPart % 10;
//                 fracPart /= 10;
//             }

//             // Remove zeros à direita
//             int lastDigit = 4;
//             while (lastDigit >= 0 && digits[lastDigit] == 0) lastDigit--;

//             for (int i = 0; i <= lastDigit; i++)
//                 buf[pos++] = '0' + digits[i];
//         }

//         // Adiciona expoente se diferente de 0
//         if (expoente != 0)
//         {
//             buf[pos++] = 'e';
//             if (expoente >= 0) buf[pos++] = '+';
//             else { buf[pos++] = '-'; expoente = -expoente; }

//             // expoente <= 3 dígitos
//             if (expoente >= 100)
//             {
//                 buf[pos++] = '0' + (expoente / 100);
//                 expoente %= 100;
//                 buf[pos++] = '0' + (expoente / 10);
//                 buf[pos++] = '0' + (expoente % 10);
//             }
//             else if (expoente >= 10)
//             {
//                 buf[pos++] = '0' + (expoente / 10);
//                 buf[pos++] = '0' + (expoente % 10);
//             }
//             else
//                 buf[pos++] = '0' + expoente;
//         }

//         buf[pos] = '\0';
//         return String(buf);
//     }
//     else if constexpr (std::is_same<T, bool>::value)
//         return _valor ? "1" : "0";
//     else
//         return String(_valor);
// }

// String paraString() const override //77.3% 
// {
//     if constexpr (std::is_same<T, float>::value)
//     {
//         if (_valor == 0.0f)
//             return "0";

//         float val = _valor;
//         int expoente = 0;

//         // Ajusta sinal
//         bool negativo = false;
//         if (val < 0.0f)
//         {
//             negativo = true;
//             val = -val;
//         }

//         // Normaliza mantissa entre 1 e 10
//         if (val >= 10.0f)
//         {
//             while (val >= 10.0f)
//             {
//                 val /= 10.0f;
//                 expoente++;
//             }
//         }
//         else if (val < 1.0f)
//         {
//             while (val < 1.0f)
//             {
//                 val *= 10.0f;
//                 expoente--;
//             }
//         }

//         if (negativo)
//             val = -val;

//         char buf[32];
//         int pos = 0;

//         // Sinal negativo
//         if (negativo)
//             buf[pos++] = '-';

//         // Parte inteira
//         int intPart = (int)val;
//         float fracPartF = val - intPart;

//         // Converte parte inteira direto no buffer
//         if (intPart == 0)
//         {
//             buf[pos++] = '0';
//         }
//         else
//         {
//             int div = 1000000000; // máximo 10 dígitos
//             bool started = false;
//             while (div > 0)
//             {
//                 int digit = intPart / div;
//                 if (digit != 0 || started)
//                 {
//                     buf[pos++] = '0' + digit;
//                     started = true;
//                 }
//                 intPart %= div;
//                 div /= 10;
//             }
//         }

//         // Parte fracionária (5 casas decimais)
//         int fracPart = (int)(fracPartF * 100000 + 0.5f);
//         if (fracPart > 0)
//         {
//             buf[pos++] = '.';
//             int div = 10000; // 5 dígitos
//             bool started = false;
//             for (int i = 0; i < 5; i++)
//             {
//                 int digit = fracPart / div;
//                 if (digit != 0 || started)
//                 {
//                     buf[pos++] = '0' + digit;
//                     started = true;
//                 }
//                 fracPart %= div;
//                 div /= 10;
//             }
//         }

//         // Expoente
//         if (expoente != 0)
//         {
//             buf[pos++] = 'e';
//             if (expoente >= 0) buf[pos++] = '+';
//             else { buf[pos++] = '-'; expoente = -expoente; }

//             if (expoente >= 100)
//             {
//                 buf[pos++] = '0' + (expoente / 100);
//                 expoente %= 100;
//                 buf[pos++] = '0' + (expoente / 10);
//                 buf[pos++] = '0' + (expoente % 10);
//             }
//             else if (expoente >= 10)
//             {
//                 buf[pos++] = '0' + (expoente / 10);
//                 buf[pos++] = '0' + (expoente % 10);
//             }
//             else
//                 buf[pos++] = '0' + expoente;
//         }

//         buf[pos] = '\0';
//         return String(buf);
//     }
//     else if constexpr (std::is_same<T, bool>::value)
//         return _valor ? "1" : "0";
//     else
//         return String(_valor);
// }

// String paraString() const override //77.1%
// {
//     if constexpr (std::is_same<T, float>::value)
//     {
//         if (_valor == 0.0f)
//             return "0";

//         float val = _valor;
//         int expoente = 0;

//         // Ajusta sinal
//         bool negativo = false;
//         if (val < 0.0f)
//         {
//             negativo = true;
//             val = -val;
//         }

//         // Normaliza mantissa entre 1 e 10
//         while (val >= 10.0f)
//         {
//             val /= 10.0f;
//             expoente++;
//         }
//         while (val < 1.0f)
//         {
//             val *= 10.0f;
//             expoente--;
//         }

//         if (negativo)
//             val = -val;

//         char buf[32];
//         int pos = 0;

//         // Sinal negativo
//         if (negativo)
//             buf[pos++] = '-';

//         // Parte inteira da mantissa (sempre 1 ou -1)
//         int intPart = (int)val;
//         buf[pos++] = '0' + abs(intPart);

//         // Parte fracionária (5 casas decimais)
//         float fracPartF = fabs(val - intPart);
//         int fracPart = (int)(fracPartF * 100000 + 0.5f);
//         if (fracPart > 0)
//         {
//             buf[pos++] = '.';
//             int div = 10000;
//             bool started = false;
//             for (int i = 0; i < 5; i++)
//             {
//                 int digit = fracPart / div;
//                 if (digit != 0 || started)
//                 {
//                     buf[pos++] = '0' + digit;
//                     started = true;
//                 }
//                 fracPart %= div;
//                 div /= 10;
//             }
//         }

//         // Expoente
//         buf[pos++] = 'e';
//         if (expoente >= 0) buf[pos++] = '+';
//         else { buf[pos++] = '-'; expoente = -expoente; }

//         if (expoente >= 100)
//         {
//             buf[pos++] = '0' + (expoente / 100);
//             expoente %= 100;
//             buf[pos++] = '0' + (expoente / 10);
//             buf[pos++] = '0' + (expoente % 10);
//         }
//         else if (expoente >= 10)
//         {
//             buf[pos++] = '0' + (expoente / 10);
//             buf[pos++] = '0' + (expoente % 10);
//         }
//         else
//             buf[pos++] = '0' + expoente;

//         buf[pos] = '\0';
//         return String(buf);
//     }
//     else if constexpr (std::is_same<T, bool>::value)
//         return _valor ? "1" : "0";
//     else
//         return String(_valor);
// }

// String paraString() const override //76.9%
// {
//     if constexpr (std::is_same<T, float>::value)
//     {
//         if (_valor == 0.0f) return "0";

//         float val = _valor;
//         int expoente = 0;
//         bool negativo = val < 0.0f;
//         if (negativo) val = -val;

//         // Normaliza mantissa entre 1 e 10
//         while (val >= 10.0f) { val /= 10.0f; expoente++; }
//         while (val < 1.0f)  { val *= 10.0f; expoente--; }

//         char buf[32];
//         int pos = 0;
//         if (negativo) buf[pos++] = '-';

//         // Parte inteira da mantissa
//         int intPart = (int)val;
//         buf[pos++] = '0' + intPart;

//         // Parte fracionária (5 dígitos)
//         int fracPart = (int)((val - intPart) * 100000 + 0.5f);

//         if (fracPart > 0)
//         {
//             buf[pos++] = '.';
//             int div = 10000;
//             bool started = false;
//             for (int i = 0; i < 5; i++)
//             {
//                 int digit = fracPart / div;
//                 if (digit != 0 || started)
//                 {
//                     buf[pos++] = '0' + digit;
//                     started = true;
//                 }
//                 fracPart %= div;
//                 div /= 10;
//             }
//         }

//         // Expoente
//         buf[pos++] = 'e';
//         buf[pos++] = (expoente >= 0 ? '+' : '-');
//         int e = (expoente >= 0 ? expoente : -expoente);
//         buf[pos++] = '0' + e / 10;
//         buf[pos++] = '0' + e % 10;

//         buf[pos] = '\0';
//         return String(buf);
//     }
//     else if constexpr (std::is_same<T, bool>::value)
//         return _valor ? "1" : "0";
//     else
//         return String(_valor);
// }


    TipoVariavel tipo() const override 
    {
        if constexpr (std::is_same<T, int32_t>::value) return TipoVariavel::INT32;
        else if constexpr (std::is_same<T, uint32_t>::value) return TipoVariavel::UINT32;
        else if constexpr (std::is_same<T, uint16_t>::value) return TipoVariavel::UINT16;
        else if constexpr (std::is_same<T, float>::value) return TipoVariavel::FLOAT;
        else if constexpr (std::is_same<T, bool>::value) return TipoVariavel::BOOL;
        else return TipoVariavel::DESCONHECIDO;
    }

};
