#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <LiquidCrystal_I2C.h>

class Mensagem 
{
public:
    Mensagem(HardwareSerial& serialHardware, LiquidCrystal_I2C& displayLCD, uint32_t serialBaudRate = 115200, int colunas = 20, int linhas = 4);
    void enviarMensagem(String L1, String L2 = "", String L3 = "", String L4 = "");
private:
    HardwareSerial& serial;
    LiquidCrystal_I2C& lcd;
    String linhasOld[4];
    int colLCD;
    int linLCD;
};
