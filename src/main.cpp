#include <Arduino.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include "Mensagem.h"
#include "ChavesSTM32.h"

// Configuração do hardware
HardwareSerial Serial2(PA3, PA2); // RX, TX
LiquidCrystal_I2C lcd(0x20, 20, 4); // Endereço, colunas, linhas
Mensagem m(Serial2, lcd, 115200, 20, 4);

ChaveSTM32 FCS(PB12, LOW);
ChaveSTM32 FCI(PB13, LOW);
ChaveSTM32 FCE(PB14, LOW);
ChaveSTM32 BtnA(PB5, LOW);
ChaveSTM32 BtnF(PB3, LOW);
ChaveSTM32 BtnP(PB4, LOW);



uint8_t scanI2C(void);

void setup()
{
    Wire.begin(); // Precisa ser iniciado aqui por causa do I2C (LCD)
    
    // Dar um tempo para o LCD inicializar
    delay(100);
    m.enviarMensagem("Sistema Iniciado", "End I2C = " + String(scanI2C(), HEX));
}

void loop()
{
    // Atualizar TODAS as chaves de uma vez
    ChaveSTM32::atualizarTodas();
    
    if (FCS.estaAtiva())
        m.enviarMensagem("FCS Ativado", String(FCS.tempoAtiva()) + " ms");
    if (FCI.estaAtiva())
        m.enviarMensagem("FCI Ativado", String(FCI.tempoAtiva()) + " ms");
    if (FCE.estaAtiva())
        m.enviarMensagem("FCE Ativado", String(FCE.tempoAtiva()) + " ms");
    if (BtnA.estaAtiva())
        m.enviarMensagem("BtnA Ativado", String(BtnA.tempoAtiva()) + " ms");
    if (BtnF.estaAtiva())
        m.enviarMensagem("BtnF Ativado", String(BtnF.tempoAtiva()) + " ms");
    if (BtnP.estaAtiva())
        m.enviarMensagem("BtnP Ativado", String(BtnP.tempoAtiva()) + " ms");

}

uint8_t scanI2C(void)
{
    Serial2.println("Escaneando I2C...");
    uint8_t foundAddress = 0;
    
    for (uint8_t address = 1; address < 127; address++) 
    {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) 
        {
            foundAddress = address;
            Serial2.println("LCD encontrado: 0x" + String(address, HEX));
            break; // Retorna o primeiro endereço encontrado
        }
    }
    
    if (foundAddress == 0) {
        Serial2.println("ERRO: LCD não encontrado!");
    }
    
    return foundAddress;
}