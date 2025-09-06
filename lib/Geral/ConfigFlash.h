#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include "Variavel.h"

class ConfigFlash
{
private:
    static const size_t tamanhoMax = 1024; // ou 500 bytes
    static const int enderecoInicial = 0;  // endereço de gravação na EEPROM/flash

    String ultimaString_; // guarda última string salva

public:
    ConfigFlash()
    {
        // Inicializa EEPROM (sem tamanho)
        EEPROM.begin();  

        // Lê última configuração armazenada
        char buffer[tamanhoMax + 1];
        for (size_t i = 0; i < tamanhoMax; i++)
        {
            buffer[i] = EEPROM.read(enderecoInicial + i);
            if (buffer[i] == 0xFF) buffer[i] = 0; // memória não programada
        }
        buffer[tamanhoMax] = 0;
        ultimaString_ = String(buffer);
    }

    ~ConfigFlash()
    {
        EEPROM.end();
    }

    bool SalvarStringConfigVariaveis(void)
    {
        return SalvarStringConfig(VariavelBase::todasPersistentesParaString());
    }
    // 🔹 Salva apenas se for diferente
    bool SalvarStringConfig(const String& s)
    {
        if (s == ultimaString_) return false;

        size_t len = s.length();
        if (len > tamanhoMax) len = tamanhoMax;

        for (size_t i = 0; i < len; i++)
        {
            EEPROM.write(enderecoInicial + i, s[i]);
        }
        // Preenche o restante com zeros
        for (size_t i = len; i < tamanhoMax; i++)
        {
            EEPROM.write(enderecoInicial + i, 0);
        }

        ultimaString_ = s;
        return true;
    }

    // 🔹 Retorna valor de variável pelo nome
    String obterValor(const String& nome)
    {
        int pos = ultimaString_.indexOf(nome + "=");
        if (pos == -1) return "";

        int inicio = pos + nome.length() + 1;
        int fim = ultimaString_.indexOf(';', inicio);
        if (fim == -1) fim = ultimaString_.length();

        return ultimaString_.substring(inicio, fim);
    }

    int obterValorInt(const String& nome)
    {
        return obterValor(nome).toInt();
    }

    float obterValorFloat(const String& nome)
    {
        return obterValor(nome).toFloat();
    }

    bool obterValorBool(const String& nome)
    {
        String val = obterValor(nome);
        return (val == "1" || val.equalsIgnoreCase("true"));
    }

    String obterStringCompleta() const
    {
        return ultimaString_;
    }
};
