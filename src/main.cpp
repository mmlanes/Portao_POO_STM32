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
Variavel<uint16_t> freqPWM("freqPWM", 500, 100, 10e3, 10, true);
Variavel<uint16_t> dPWMMax("dPWMMax", 60, 10, 100, 1, true);
Variavel<float> acelPWM("acelPWM", 0.1f, 10.0f, 0.1, 0.1f, true);
Variavel<float> adjADC("adjADC", 1.7e-3f, 0.1e-3f, 10.0e-3f, 0.1e-3f, true);
Variavel<bool> salvarConfigFlash("salvarConfigFlash", false, false, true, true, false);

ModosOperacao modoNormal("Normal");
ModosOperacao modoEncAtivo("Encoder ativo");
ModosOperacao modoEncMaximo("Encoder maximo");
ModosOperacao modoEncReverso("Encoder reverso");
ModosOperacao modoFreqPWM("Frequencia PWM");
ModosOperacao modoDPWM("D PWM");
ModosOperacao modoAcelPWM("Aceleracao PWM");
ModosOperacao modoConstanteADC("Constante ADC");
ModosOperacao modoSalvarConfigFlash("Salvar config Flash");

AcoesChaves trocarModo(btnP, btnA, btnF, false, false, nullptr, [](uint8_t){ ModosOperacao::modoSeguinte(); }, 2000, true, 50000, 1, 60000, 1); 
AcoesChaves ativarEnc(btnA, btnP, btnF, false, false, &modoEncAtivo, [](uint8_t v){ encAtivo.incrementar(v); }, 500, false);
AcoesChaves desativarEnc(btnF, btnP, btnA, false, false, &modoEncAtivo, [](uint8_t v){ encAtivo.decrementar(v); }, 500, false);
AcoesChaves incEncMax(btnA, btnP, btnF, false, false, &modoEncMaximo, [](uint8_t v){ encMax.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves decEncMax(btnF, btnP, btnA, false, false, &modoEncMaximo, [](uint8_t v){ encMax.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves encoderReverso(btnA, btnP, btnF, false, false, &modoEncReverso, [](uint8_t v){ encRev.incrementar(v); }, 500, false);
AcoesChaves encoderDireto(btnF, btnP, btnA, false, false, &modoEncReverso, [](uint8_t v){ encRev.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves incFreqPWM(btnA, btnP, btnF, false, false, &modoFreqPWM, [](uint8_t v){ freqPWM.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves decFreqPWM(btnF, btnP, btnA, false, false, &modoFreqPWM, [](uint8_t v){ freqPWM.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves incDPWM(btnA, btnP, btnF, false, false, &modoDPWM, [](uint8_t v){ dPWMMax.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves decDPWM(btnF, btnP, btnA, false, false, &modoDPWM, [](uint8_t v){ dPWMMax.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves incAcelPWM(btnA, btnP, btnF, false, false, &modoAcelPWM, [](uint8_t v){ acelPWM.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves decAcelPWM(btnF, btnP, btnA, false, false, &modoAcelPWM, [](uint8_t v){ acelPWM.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves inAdjADC(btnA, btnP, btnF, false, false, &modoConstanteADC, [](uint8_t v){ adjADC.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves decAdjADC(btnF, btnP, btnA, false, false, &modoConstanteADC, [](uint8_t v){ adjADC.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves acSalvarConfigFlash(btnF, btnP, btnA, true, true, &modoSalvarConfigFlash, [](uint8_t v){ salvarConfigFlash.incrementar(v); }, 5000, false);


//void Testar_Chaves(void);

void setup()
{
    ModosOperacao::modoSeguinte(); // Inicia no primeiro modo
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
    m.enviarMensagem(ModosOperacao::modoAtual()->obterNome(), 
                     "encAtivo = " + String(encAtivo.obterValor()), 
                     "freqPWM = $freqPWM$", 
                     "adjADC = " + String(adjADC.obterValor(),5));

    AcoesChaves::atuarTodas(ModosOperacao::modoAtual());
    //Serial2.println(VariavelBase::todasPersistentesParaString());

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

