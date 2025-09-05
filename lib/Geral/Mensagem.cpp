#include "Mensagem.h"

Mensagem::Mensagem(HardwareSerial &serialHardware, LiquidCrystal_I2C &displayLCD, uint32_t serialBaudRate, int colunas, int linhas)
    : serial(serialHardware), lcd(displayLCD), colLCD(colunas), linLCD(linhas)
{
    serial.begin(115200);
    serial.println("Serial iniciada..");

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("LCD Iniciado");
    for (int i = 0; i < 4; i++)
        linhasOld[i] = "";
}

void Mensagem::enviarMensagem(String L1, String L2, String L3, String L4)
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
        if (L[i] != linhasOld[i])
        {
            // Primeiro: cortar se for maior que o LCD
            if (L[i].length() > colunas)
                L[i] = L[i].substring(0, colLCD);
            // Depois: sempre completar com espaços até atingir colLCD
            while (L[i].length() < colunas) 
                L[i] += " ";
            lcd.setCursor(0, i);
            lcd.print(L[i]);
            linhasOld[i] = L[i];
            flagMudanca = true;
        }
    }

    if (flagMudanca)
    {
        serial.println("----- Mensagem -----");
        for (int i = 0; i < linLCD; i++)
        {
            serial.println("|" + L[i] + "|");
        }
        serial.println("----- " + String(millis()) + " -----");
    }
}
