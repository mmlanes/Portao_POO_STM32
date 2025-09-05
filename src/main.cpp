#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "Mensagem.h"
#include "ChavesSTM32.h"

// Configuração do hardware
HardwareSerial Serial2(PA3, PA2); // RX, TX
LiquidCrystal_I2C lcd(0x20, 20, 4); // Endereço, colunas, linhas
ChaveSTM32 FCS(PB12, LOW);
ChaveSTM32 FCI(PB13, LOW);
ChaveSTM32 FCE(PB14, LOW);
ChaveSTM32 BtnA(PB5, LOW);
ChaveSTM32 BtnF(PB3, LOW);
ChaveSTM32 BtnP(PB4, LOW);
Mensagem m(Serial2, lcd, 115200, 20, 4);


uint8_t scanI2C(void);

void setup()
{
    m.enviarMensagem("Sistema Iniciado");
    
}

void loop()
{
    FCS.atualizar();
    if (FCS.estaAtiva())
    {
        m.enviarMensagem("FCS Ativado");
        m.enviarMensagem(String(FCS.tempoAtiva()) + " ms");
    }
}

uint8_t scanI2C(void)
{
    uint8_t foundAddress = 0;
    for (uint8_t address = 1; address < 127; address++) 
    {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) 
        {
            foundAddress = address;
            break; // Retorna o primeiro endereço encontrado
        }
    }
    return foundAddress;
}