#pragma once
#include <Arduino.h>
#include <EEPROM.h>

class ConfigFlash
{
private:
    static const size_t tamanhoMax = 1024; // tamanho máximo da área reservada
    static const int enderecoInicial = 0;  // endereço inicial de gravação

    String ultimaString_; // guarda última string salva

public:
    ConfigFlash()
    {
        // Inicializa a EEPROM emulada na flash do STM32
        EEPROM.begin();  

        // Lê a última configuração armazenada
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

    // 🔹 Salva todas variáveis persistentes (usando função auxiliar externa)
    bool SalvarStringConfigVariaveis(void)
    {
        return SalvarStringConfig(VariavelBase::todasPersistentesParaString());
    }

    // 🔹 Salva string (só se for diferente da última)
    bool SalvarStringConfig(const String& s)
    {
        if (s == ultimaString_) return false;

        size_t len = s.length();
        if (len > tamanhoMax - 1) len = tamanhoMax - 1; // reserva espaço para '\0'

        // Escreve a string na EEPROM
        for (size_t i = 0; i < len; i++)
        {
            if (EEPROM.read(enderecoInicial + i) != s[i])
                EEPROM.write(enderecoInicial + i, s[i]);
        }

        // Grava o terminador nulo
        EEPROM.write(enderecoInicial + len, 0);

        ultimaString_ = s;
        return true;
    }

    // 🔹 Retorna valor de variável pelo nome
    String obterValor(const String& nome)
    {
        String s = obterStringCompleta(); // lê string completa
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
