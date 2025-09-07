#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Variavel.h"
#include "MensagemLCD.h"

class Mensagem 
{
private:
    HardwareSerial& serial;
    LiquidCrystal_I2C* lcd;
    String linhasOld[4];
    int colLCD;
    int linLCD;
    uint8_t scanI2C(void)
    {
        serial.println("Escaneando I2C...");
        uint8_t foundAddress = 0;
        
        for (uint8_t address = 1; address < 127; address++) 
        {
            Wire.beginTransmission(address);
            if (Wire.endTransmission() == 0) 
            {
                foundAddress = address;
                serial.println("LCD encontrado: 0x" + String(address, HEX));
                break; // Retorna o primeiro endereço encontrado
            }
        }
        
        if (foundAddress == 0) {
            serial.println("ERRO: LCD não encontrado!");
        }
        
        return foundAddress;
    }

public:
    Mensagem(HardwareSerial& serialHardware, uint32_t serialBaudRate = 115200, int colunas = 20, int linhas = 4)
    : serial(serialHardware), colLCD(colunas), linLCD(linhas)
    {
        serial.begin(115200);
        serial.println("Serial iniciada..");

        Wire.begin();
        delay(100);
        uint8_t endereco = scanI2C();
        if (endereco == 0)
        {
            serial.println("LCD nao achado em i2c.");
            lcd = nullptr;
            return;
        }
        else
            lcd = new LiquidCrystal_I2C(endereco, colLCD, linLCD);
        
        if (lcd == nullptr)
        {
            serial.println("LCD nao inicializado.");
            return;
        }

        lcd->init();
        lcd->backlight();
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->print("LCD Iniciado");
        for (int i = 0; i < 4; i++)
            linhasOld[i] = "";
    }
    void enviarMensagem(MensagemLCD* mensagem)
    {
        if (mensagem->modoAtual() != ModosOperacao::modoAtual())
            return;
        enviarMensagem(mensagem->obterL1(), mensagem->obterL2(), mensagem->obterL3(), mensagem->obterL4());
    }
    void enviarMensagem(String L1, String L2 = "", String L3 = "", String L4 = "")
    {
        String L[4] = {L1, L2, L3, L4};
        bool flagMudanca = false;

        for (int i = 0; i < linLCD; i++)
        {
            unsigned int colunas = static_cast<unsigned int>(colLCD);
            // Linhas vazias signfica que o conteúdo anterior não deve ser alterado
            if (L[i] == "")
                continue;
            // Linha = " " signfica que a linha deve ser apagada
            if (L[i] == " ")
                while (L[i].length() < colunas) 
                    L[i] += " ";      
            // Substitui variáveis por valores
            L[i] = VariavelBase::substituirVariaveis(L[i]);
            // Primeiro: cortar se for maior que o LCD
            if (L[i].length() > colunas)
                L[i] = L[i].substring(0, colLCD);
            // Depois: sempre completar com espaços até atingir colLCD
            while (L[i].length() < colunas) 
                L[i] += " ";
            if (L[i] != linhasOld[i])
            {
                if (lcd)
                {
                    lcd->setCursor(0, i);
                    lcd->print(L[i]);
                }
                linhasOld[i] = L[i];
                flagMudanca = true;
            }
        }

        if (flagMudanca)
        {
            serial.println("+--------------------+");
            for (int i = 0; i < linLCD; i++)
                serial.println("|" + L[i] + "|");
            serial.print("  ----- ");
            serial.print(millis());
            serial.println(" -----");
        }
    }

};
