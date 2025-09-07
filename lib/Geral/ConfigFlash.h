#pragma once
#include <Arduino.h>
#include <EEPROM.h>

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

        // Escreve a string na EEPROM
        for (size_t i = 0; i < len; i++)
        {
            if (EEPROM.read(enderecoInicial + i) != s[i])
                EEPROM.write(enderecoInicial + i, s[i]);
        }

        // Preenche o restante com zeros
        for (size_t i = len; i < tamanhoMax; i++)
        {
            if (EEPROM.read(enderecoInicial + i) != 0)
                EEPROM.write(enderecoInicial + i, 0);
        }

        ultimaString_ = s;
        return true;
    }


    // 🔹 Retorna valor de variável pelo nome
    String obterValor(const String& nome)
    {
        String s = obterStringCompleta(); // lê toda a flash
        if (s.startsWith("CFG=")) s = s.substring(4);

        int start = s.indexOf(nome + "=");
        if (start == -1) return "";
        start += nome.length() + 1;
        int end = s.indexOf(";", start);
        if (end == -1) end = s.length();
        return s.substring(start, end);
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
