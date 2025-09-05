#include <Arduino.h>
#include <HardwareSerial.h>
#include <LiquidCrystal_I2C.h>

#include "Mensagem.h"
#include "ChavesSTM32.h"
#include "EncoderSTM32.h"

// Configuração do hardware
HardwareSerial Serial2(PA3, PA2); // RX, TX

Mensagem m(Serial2, 115200, 20, 4);

ChaveSTM32 fcS(PB12, LOW);
ChaveSTM32 fcI(PB13, LOW);
ChaveSTM32 fcE(PB14, LOW);
ChaveSTM32 btnA(PB5, LOW);
ChaveSTM32 btnF(PB3, LOW);
ChaveSTM32 btnP(PB4, LOW);

EncoderSTM32 encAB(PA12, PA15, true, false, false);


//void Testar_Chaves(void);

void setup()
{
    delay(100);
    m.enviarMensagem("Sistema Iniciado");
    encAB.definirPosicaoMaximaAbs(2750);
}

void loop()
{
    // Atualizar TODAS as chaves de uma vez
    ChaveSTM32::atualizarTodas();
    m.enviarMensagem("Encoder", "Posicao = " + String(encAB.obterPosicao()), "PosN100aP100 = " + String(encAB.obterPosicao_N100aP100()));

    delay(1000); 

}

// void Testar_Chaves(void)
// {
//     if (fcS.estaAtiva())
//         m.enviarMensagem("FCS Ativado", String(fcS.tempoAtiva()) + " ms");
//     if (fcI.estaAtiva())
//         m.enviarMensagem("FCI Ativado", String(fcI.tempoAtiva()) + " ms");
//     if (fcE.estaAtiva())
//         m.enviarMensagem("FCE Ativado", String(fcE.tempoAtiva()) + " ms");
//     if (btnA.estaAtiva())
//         m.enviarMensagem("BtnA Ativado", String(btnA.tempoAtiva()) + " ms");
//     if (btnF.estaAtiva())
//         m.enviarMensagem("BtnF Ativado", String(btnF.tempoAtiva()) + " ms");
//     if (btnP.estaAtiva())
//         m.enviarMensagem("BtnP Ativado", String(btnP.tempoAtiva()) + " ms");
// }

