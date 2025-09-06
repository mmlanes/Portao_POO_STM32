#include <Arduino.h>
#include <HardwareSerial.h>
#include <LiquidCrystal_I2C.h>

#include "Mensagem.h"
#include "ChaveSTM32.h"
#include "EncoderSTM32.h"
#include "PWM_PB1_STM32.h"
#include "FastADC_PA0_STM32.h"
#include "Variavel.h"
#include "ModosOperacao.h"
#include "AcoesChaves.h"

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

Variavel<bool> encAtivo("encAtivo", true, false, true, true, true);
Variavel<uint32_t> encMax("encMax", 2750, 100, 10e3, 1, true);
Variavel<bool> encRev("encRev", false, false, true, true, true);
Variavel<uint16_t> freqPWM("freqPWM", 500, 100, 10e3, 100, true);
Variavel<uint16_t> dPWMMax("dPWMMax", 60, 10, 100, 1, true);
Variavel<float> acelPWM("acelPWM", 0.1f, 10.0f, 0.1, 0.1f, true);
Variavel<float> adjADC("adjADC", 1.7e-3f, 0.1e-3f, 10.0e-3f, 0.1e-3f, true);

ModosOperacao modoNormal("Normal");
ModosOperacao modoConfig("Config");
ModosOperacao modoTeste("Teste");


AcoesChaves trocarModo(btnP, btnA, btnF, false, false, "", [](uint8_t){ ModosOperacao::modoSeguinte(); }, 3000, false); 
AcoesChaves incEncMax(btnA, btnP, btnF, false, false, "Config", [](uint8_t v){ encMax.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves decEncMax(btnF, btnP, btnA, false, false, "Config", [](uint8_t v){ encMax.decrementar(v); }, 500, true, 5000, 10, 10000, 100);


//void Testar_Chaves(void);

void setup()
{
    //ModosOperacao::modoSeguinte(); // Inicia no primeiro modo
    encMax.incrementar();
    Serial2.println("..");
    delay(100);
    m.enviarMensagem("Sistema Iniciado");
    encAB.definirPosicaoMaximaAbs(2750);
    Serial2.println("...");
    //PWM_PB1_STM32::setupPwmUpDown(500, FastADC_PA0_STM32::leituraSincronizadaPWM);
    Serial2.println("....");
}

void loop()
{
    //Serial2.print(".");
    // Atualizar TODAS as chaves de uma vez
    ChaveSTM32::atualizarTodas();
    m.enviarMensagem("Modo = " + ModosOperacao::modoAtual().obterNome(), 
                     "Posicao = " + String(encAB.obterPosicao()), 
                     "PosN100aP100 = " + String(encAB.obterPosicao_N100aP100()), 
                     "PosMax = " + String(encMax.obterValor()));

    AcoesChaves::atuarTodas();

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

