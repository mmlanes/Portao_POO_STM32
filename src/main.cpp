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
#include "MensagemLCD.h"
#include "ConfigFlash.h"

// Configuração do hardware
HardwareSerial Serial2(PA3, PA2); // RX, TX

Mensagem m(Serial2, 115200, 20, 4); 

ChaveSTM32 fcS(PB12, LOW);
ChaveSTM32 fcI(PB13, LOW);
ChaveSTM32 fcE(PB14, LOW);
ChaveSTM32 btnA(PB5, LOW);
ChaveSTM32 btnF(PB3, LOW);
ChaveSTM32 btnP(PB4, LOW);

Variavel<int32_t> encPos("encPos", 0, -100e3, 100e3, 1, false);
Variavel<bool> encAtivo("encAtivo", true, false, true, true, true);
Variavel<uint32_t> encMax("encMax", 2750, 100, 10e3, 1, true);
Variavel<bool> encRev("encRev", false, false, true, true, true);
Variavel<uint16_t> freqPWM("freqPWM", 500, 100, 10e3, 10, true);
Variavel<uint16_t> dPWMMax("dPWMMax", 60, 10, 100, 1, true);
Variavel<float> acelPWM("acelPWM", 0.1f, 10.0f, 0.1, 0.1f, true);
Variavel<float> adjADC("adjADC", 1.7e-3f, 0.1e-3f, 10.0e-3f, 0.1e-3f, true);
Variavel<float> iMedio("iMedio", 0.0f, 0.0f, 20.0f, 0.0f, false);
Variavel<float> iProt("iProt", 5.0f, 1.0f, 20.0f, 0.1f, true);
Variavel<bool> salvarConfigFlash("salvarConfigFlash", false, false, true, true, false);
Variavel<bool> carregarConfigFlash("carregarConfigFlash", false, false, true, true, false);

EncoderSTM32 encAB(encPos, encMax, encRev, PA12, PA15, true, false, false);

ModosOperacao modoNormal("Normal");
ModosOperacao modoMonitorGeral("Monitor geral");
ModosOperacao modoEncAtivo("Encoder ativo");
ModosOperacao modoEncMaximo("Encoder maximo");
ModosOperacao modoEncReverso("Encoder reverso");
ModosOperacao modoFreqPWM("Frequencia PWM");
ModosOperacao modoDPWM("D PWM");
ModosOperacao modoAcelPWM("Aceleracao PWM");
ModosOperacao modoConstanteADC("Constante ADC");
ModosOperacao modoCorrenteProtecao("Corrente protecao");
ModosOperacao modoSalvarConfigFlash("Salvar config Flash");
ModosOperacao modoCarregarConfigFlash("Carrega config Flash");

AcoesChaves avancarModo(btnP, btnA, btnF, true, false, nullptr, [](uint8_t){ ModosOperacao::modoSeguinte(); }, 2000, true, 50000, 1, 60000, 1); 
AcoesChaves voltarModo(btnP, btnA, btnF, false, true, nullptr, [](uint8_t){ ModosOperacao::modoAnterior(); }, 2000, true, 50000, 1, 60000, 1); 
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
AcoesChaves incAdjADC(btnA, btnP, btnF, false, false, &modoConstanteADC, [](uint8_t v){ adjADC.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves decAdjADC(btnF, btnP, btnA, false, false, &modoConstanteADC, [](uint8_t v){ adjADC.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves incCorrenteProtecao(btnA, btnP, btnF, false, false, &modoCorrenteProtecao, [](uint8_t v){ iProt.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves decCorrenteProtecao(btnF, btnP, btnA, false, false, &modoCorrenteProtecao, [](uint8_t v){ iProt.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
AcoesChaves acSalvarConfigFlash(btnF, btnP, btnA, true, true, &modoSalvarConfigFlash, [](uint8_t v){ salvarConfigFlash.incrementar(v); }, 5000, false);
AcoesChaves acCarregarConfigFlash(btnF, btnP, btnA, true, true, &modoCarregarConfigFlash, [](uint8_t v){ carregarConfigFlash.incrementar(v); }, 5000, false);

String trocarTela = "Tela P+A >> e P+F <<";
String incDecBool = "Valor: BtA=1 e BtF=0";
String incDecNum = "Valor: BtA=+ e BtF=-";
MensagemLCD mMonitorGeral(&modoMonitorGeral, "$modo$", "Pos=$encPos$/$encMax$ Rev=$encRev$", " ", trocarTela);
MensagemLCD mAtivarEnc(&modoEncAtivo, "$modo$", "$encAtivo$", incDecBool, trocarTela);
MensagemLCD mEncMaximo(&modoEncMaximo, "$modo$", "$encMax$", incDecNum, trocarTela);
MensagemLCD mEncReverso(&modoEncReverso, "$modo$", "$encRev$", incDecBool, trocarTela);
MensagemLCD mFreqPWM(&modoFreqPWM, "$modo$", "$freqPWM$ Hz", incDecNum, trocarTela);
MensagemLCD mDPWM(&modoDPWM, "$modo$", "$dPWMMax$ %", incDecNum, trocarTela);
MensagemLCD mAcelPWM(&modoAcelPWM, "$modo$", "$acelPWM$ dPwm/s", incDecNum, trocarTela);
MensagemLCD mConstADC(&modoConstanteADC, "$modo$", "$adjADC$", incDecNum, trocarTela);
MensagemLCD mCorrenteProtecao(&modoCorrenteProtecao, "$modo$", "($iMedio$)|Ip=$iProt$ A", incDecNum, trocarTela);
MensagemLCD mSalvarFlash(&modoSalvarConfigFlash, "$modo$", "$salvarConfigFlash$", "Bts(A+F+P) 5s salvar", trocarTela);
MensagemLCD mCarregarFlash(&modoCarregarConfigFlash, "$modo$", "$carregarConfigFlash$", "Bts(A+F+P) 5s carreg", trocarTela);


//void Testar_Chaves(void);
void CarregarVariaveisFlash(void);

void setup()
{
    ConfigFlash cfg;
    cfg.obterStringCompleta();
    Serial2.println("CFG: " + cfg.obterStringCompleta() + "|");
    CarregarVariaveisFlash();
    FastADC_PA0_STM32::setupAdcPa0Fast();
    PWM_PB1_STM32::setupPwmUpDown(freqPWM.obterValor(), FastADC_PA0_STM32::leituraSincronizadaPWM);
    delay(100);
    encPos.definirValor(10);
}

void loop()
{
    ChaveSTM32::atualizarTodas();

    for (auto* msg : MensagemLCD::todas()) 
        m.enviarMensagem(msg); 

    AcoesChaves::atuarTodas(ModosOperacao::modoAtual());

    if (carregarConfigFlash.obterValor())
    {
        carregarConfigFlash.definirValor(false);
        CarregarVariaveisFlash();
        m.enviarMensagem("Config Flash", "Carregada", " ", "aguarde 3s");
        delay(3000);
    }

    if (salvarConfigFlash.obterValor())
    {
        salvarConfigFlash.definirValor(false);
        ConfigFlash cfg;
        String C = VariavelBase::todasPersistentesParaString();
        cfg.SalvarStringConfig(C);
        m.enviarMensagem("Config Flash", "Salva", " ", "aguarde 3s");
        delay(3000);
    }

    iMedio.definirValor(FastADC_PA0_STM32::obterGrandezaMediaPeriodica(500));

}

void CarregarVariaveisFlash(void)
{
    ConfigFlash cfg;

    for (auto v : VariavelBase::_todas) 
    {
        if (!v->ehPersistente()) 
            continue; // só persistentes

        String nome = v->obterNome();
        String valorStr = cfg.obterValor(nome);
        if (valorStr.length() == 0) 
            continue; // valor não encontrado
        switch (v->tipo())
        {
            case TipoVariavel::INT32:
                static_cast<Variavel<int32_t>*>(v)->definirValor((int32_t)valorStr.toInt());
                break;
            case TipoVariavel::UINT32:
                static_cast<Variavel<uint32_t>*>(v)->definirValor((uint32_t)valorStr.toInt());
                break;
            case TipoVariavel::UINT16:
                static_cast<Variavel<uint16_t>*>(v)->definirValor((uint16_t)valorStr.toInt());
                break;
            case TipoVariavel::FLOAT:
                static_cast<Variavel<float>*>(v)->definirValor(valorStr.toFloat());
                break;
            case TipoVariavel::BOOL:
                static_cast<Variavel<bool>*>(v)->definirValor(valorStr == "1" || valorStr.equalsIgnoreCase("true"));
                break;
            default:
                // tipo desconhecido, não faz nada
                break;
        }
    }
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

