#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>
#include <LiquidCrystal_I2C.h>

#include "Mensagem.h"
#include "ChaveSTM32.h"
#include "EncoderSTM32.h"
#include "PWM_PB1_STM32_S.h"
#include "FastADC_PA0_STM32_S.h"
#include "Variavel.h"
#include "ModosOperacao.h"
#include "MensagemLCD.h"
#include "ConfigFlash.h"
#include "Motor.h"
#include "Portao.h"
#include "Protecao.h"
#include "GerenciadorAcoes.h" // Nova classe

// -------------------------- HARDWARE --------------------------
HardwareSerial Serial2(PA3, PA2); // RX, TX
Mensagem m(Serial2, 115200, 20, 4); 

// -------------------------- VARIÁVEIS --------------------------
Variavel<int32_t> encPos("encPos", 0, -100e3, 100e3, 1, false);
Variavel<uint8_t> encPosPartida0a100("encPosPartida0a100", 10, 0, 100, 1, true);
Variavel<uint8_t> encPosParada0a100("encPosParada0a100", 90, 0, 100, 1, true);
Variavel<bool> encAtivo("encAtivo", true, true);
Variavel<uint32_t> encMax("encMax", 2750, 100, 10e3, 1, true);
Variavel<bool> encRev("encRev", false, true);

Variavel<uint16_t> freqPWM("freqPWM", 500, 100, 10e3, 10, true);
Variavel<uint8_t> dPWM("dPWM", 0, 0, 100, 0, false);
Variavel<uint8_t> dPWMMax("dPWMMax", 60, 10, 100, 1, true);
Variavel<uint8_t> dPWMPartida("dPWMPartida", 15, 0, 100, 1, true);
Variavel<uint8_t> dPWMParada("dPWMParada", 10, 0, 100, 1, true);
Variavel<float> acelPWM("acelPWM", 1.0f, 0.1f, 100.0f, 0.1f, true);
Variavel<float> adjADC("adjADC", 1.7e-3f, 0.1e-3f, 10.0e-3f, 0.1e-3f, true);
Variavel<float> iMedio("iMedio", 0.0f, 0.0f, 20.0f, 0.0f, false);

Variavel<String> posicaoPortao("posPortao", "", false);
Variavel<String> operacaoPortao("operPortao", "", false);
Variavel<float> rampaPWMPosicao("rampaPWMPos", 2.0f, 0.1f, 20.0f, 0.1f, true);

Variavel<bool> salvarConfigFlash("salvarConfigFlash", false, false);
Variavel<bool> carregarConfigFlash("carregarConfigFlash", false, false);

Variavel<bool> protecaoEncoderParadoAtuada("protEncParado", false, false);
Variavel<bool> protecaoSobrecorrenteAtuada("protSobrecorrente", false, false);
Variavel<float> iProt("iProt", 5.0f, 1.0f, 20.0f, 0.1f, true);

// -------------------------- MODOS --------------------------
ModosOperacao modoNormal("Normal");
ModosOperacao modoMonitorGeral("Monitor geral");
ModosOperacao modoProtEncParado("Prot. Encoder Parado");
ModosOperacao modoProtSobrecorrente("Prot. Sobrecorrente");
ModosOperacao modoEncAtivo("Encoder ativo");
ModosOperacao modoEncMaximo("Encoder maximo");
ModosOperacao modoEncReverso("Encoder reverso");
ModosOperacao modoFreqPWM("Frequencia PWM");
ModosOperacao modoDPWM("D PWM");
ModosOperacao modoAcelPWM("Aceleracao PWM");
ModosOperacao modoConstanteADC("Constante ADC");
ModosOperacao modoCorrenteProtecao("Corrente protecao");
ModosOperacao modoRampaPWMPosicao("Rampa PWM/Posicao");
ModosOperacao modoSalvarConfigFlash("Salvar config Flash");
ModosOperacao modoCarregarConfigFlash("Carrega config Flash");

// -------------------------- MENSAGENS --------------------------
String trocarTela = "Tela P+A >> e P+F <<";
String incDecBool = "Valor: BtA=1 e BtF=0";
String incDecNum = "Valor: BtA=+ e BtF=-";

MensagemLCD mNormal(&modoNormal, "$modo$", "D=$dPWM$ Pos=$encPos$", "PP=$posPortao$ OP=$operPortao$ EA=$encAtivo$", trocarTela);
MensagemLCD mProtEncParado(&modoProtEncParado, "$modo$", "ativada = $protEncParado$", "(atua se Enc Ativo)", trocarTela);
MensagemLCD mProtSobrecorrente(&modoProtSobrecorrente, "$modo$", "ativada = $protSobrecorrente$", " ", trocarTela);
MensagemLCD mMonitorGeral(&modoMonitorGeral, "$modo$", "Pos=$encPos$/$encMax$ Rev=$encRev$", " ", trocarTela);
MensagemLCD mAtivarEnc(&modoEncAtivo, "$modo$", "$encAtivo$", incDecBool, trocarTela);
MensagemLCD mEncMaximo(&modoEncMaximo, "$modo$", "$encMax$", incDecNum, trocarTela);
MensagemLCD mEncReverso(&modoEncReverso, "$modo$", "$encRev$", incDecBool, trocarTela);
MensagemLCD mFreqPWM(&modoFreqPWM, "$modo$", "$freqPWM$ Hz", incDecNum, trocarTela);
MensagemLCD mDPWM(&modoDPWM, "$modo$", "$dPWMMax$ %", incDecNum, trocarTela);
MensagemLCD mAcelPWM(&modoAcelPWM, "$modo$", "$acelPWM$ dPwm/s", incDecNum, trocarTela);
MensagemLCD mConstADC(&modoConstanteADC, "$modo$", "$adjADC$", incDecNum, trocarTela);
MensagemLCD mCorrenteProtecao(&modoCorrenteProtecao, "$modo$", "($iMedio$)|Ip=$iProt$ A", incDecNum, trocarTela);
MensagemLCD mRampaPWMPosicao(&modoRampaPWMPosicao, "$modo$", "$rampaPWMPos$ %PWM/%PosEnc", incDecNum, trocarTela);
MensagemLCD mSalvarFlash(&modoSalvarConfigFlash, "$modo$", "$salvarConfigFlash$", "Bts(A+F) 5s salvar", trocarTela);
MensagemLCD mCarregarFlash(&modoCarregarConfigFlash, "$modo$", "$carregarConfigFlash$", "Bts(A+F) 5s carreg", trocarTela);

// -------------------------- CHAVES --------------------------
ChaveSTM32 fcS(PB12, LOW);
ChaveSTM32 fcI(PB13, LOW);
ChaveSTM32 fcE(PB14, LOW);
ChaveSTM32 btnA(PB5, LOW);
ChaveSTM32 btnF(PB3, LOW);
ChaveSTM32 btnP(PB4, LOW);

EncoderSTM32 encAB(encPos, encMax, encRev, PA12, PA15, true, false, false);

auto& adc = FastADC_PA0_STM32_S::getInstance();
auto& pwm = PWM_PB1_STM32_S::getInstance(freqPWM, dPWM, dPWMMax, acelPWM, FastADC_PA0_STM32_S::leituraSincronizadaPWM);

Motor motor(PB11, PB10, pwm, adc, iMedio, adjADC);
Portao portao(fcS, fcI, encAB, motor, encAtivo, encPosPartida0a100, encPosParada0a100, dPWMPartida, dPWMParada, rampaPWMPosicao);
Protecao protecao(portao, iProt, protecaoEncoderParadoAtuada, protecaoSobrecorrenteAtuada);

// -------------------------- FUNÇÕES --------------------------
static void acaoIrModoNormal(uint8_t) { ModosOperacao::definirModoAtualPorPosicao(0); }
static void acaoAvancarModo(uint8_t) { ModosOperacao::modoSeguinte(); }
static void acaoVoltarModo(uint8_t) { ModosOperacao::modoAnterior(); }
static void acaoAbrirPortao(uint8_t) { portao.abrir(); }
static void acaoFecharPortao(uint8_t) { portao.fechar(); }
static void acaoPararPortao(uint8_t) { portao.parar(); }
static void acaoAbrirAcelerado(uint8_t) { portao.abrirFecharAceleradoSemEncoder(); }
static void acaoFecharAcelerado(uint8_t) { portao.abrirFecharAceleradoSemEncoder(); }
static void acaoDesacelerarPortao(uint8_t) { portao.abrirFecharDesaceleradoSemEncoder(); }
static void acaoVelocidadeNormal(uint8_t) { portao.velocidadeNormalSemEncoder(); }

static void acaoAtivarEnc(uint8_t v) { encAtivo.incrementar(v); }
static void acaoDesativarEnc(uint8_t v) { encAtivo.decrementar(v); }
static void acaoIncEncMax(uint8_t v) { encMax.incrementar(v); }
static void acaoDecEncMax(uint8_t v) { encMax.decrementar(v); }
static void acaoEncoderReverso(uint8_t v) { encRev.incrementar(v); }
static void acaoEncoderDireto(uint8_t v) { encRev.decrementar(v); }
static void acaoIncFreqPWM(uint8_t v) { freqPWM.incrementar(v); }
static void acaoDecFreqPWM(uint8_t v) { freqPWM.decrementar(v); }
static void acaoIncDPWM(uint8_t v) { dPWMMax.incrementar(v); }
static void acaoDecDPWM(uint8_t v) { dPWMMax.decrementar(v); }
static void acaoIncAcelPWM(uint8_t v) { acelPWM.incrementar(v); }
static void acaoDecAcelPWM(uint8_t v) { acelPWM.decrementar(v); }
static void acaoIncAdjADC(uint8_t v) { adjADC.incrementar(v); }
static void acaoDecAdjADC(uint8_t v) { adjADC.decrementar(v); }
static void acaoIncCorrenteProtecao(uint8_t v) { iProt.incrementar(v); }
static void acaoDecCorrenteProtecao(uint8_t v) { iProt.decrementar(v); }
static void acaoIncRampaPWMPosicao(uint8_t v) { rampaPWMPosicao.incrementar(v); }
static void acaoDecRampaPWMPosicao(uint8_t v) { rampaPWMPosicao.decrementar(v); }
static void acaoSalvarConfigFlash(uint8_t v) { salvarConfigFlash.incrementar(v); }
static void acaoCarregarConfigFlash(uint8_t v) { carregarConfigFlash.incrementar(v); }

// -------------------------- GERENCIADOR DE AÇÕES --------------------------
GerenciadorAcoes gerenciador;

// -------------------------- ARRAYS DE CHAVES --------------------------
IChave* chaves3[3] = { &btnA, &btnP, &btnF };

// -------------------------- CONFIGURAÇÃO DAS AÇÕES --------------------------
void configurarAcoes() {
    // Modos gerais
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b111, nullptr, acaoIrModoNormal, 2000, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b110, nullptr, acaoAvancarModo, 2000, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b011, nullptr, acaoVoltarModo, 2000, true });

    // Portão
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b100, &modoNormal, acaoAbrirPortao, 1000, false });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b001, &modoNormal, acaoFecharPortao, 1000, false });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b010, &modoNormal, acaoPararPortao, 50, false });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b100, &modoNormal, acaoAbrirAcelerado, 2000, false });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b001, &modoNormal, acaoFecharAcelerado, 2000, false });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b101, &modoNormal, acaoDesacelerarPortao, 2000, false });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b000, &modoNormal, acaoVelocidadeNormal, 500, false });

    // Encoder
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b100, &modoEncAtivo, acaoAtivarEnc, 500, false });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b001, &modoEncAtivo, acaoDesativarEnc, 500, false });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b100, &modoEncMaximo, acaoIncEncMax, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b001, &modoEncMaximo, acaoDecEncMax, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b100, &modoEncReverso, acaoEncoderReverso, 500, false });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b001, &modoEncReverso, acaoEncoderDireto, 500, true });

    // PWM
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b100, &modoFreqPWM, acaoIncFreqPWM, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b001, &modoFreqPWM, acaoDecFreqPWM, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b100, &modoDPWM, acaoIncDPWM, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b001, &modoDPWM, acaoDecDPWM, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b100, &modoAcelPWM, acaoIncAcelPWM, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b001, &modoAcelPWM, acaoDecAcelPWM, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b100, &modoConstanteADC, acaoIncAdjADC, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b001, &modoConstanteADC, acaoDecAdjADC, 500, true });

    // Proteção e Rampa
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b100, &modoCorrenteProtecao, acaoIncCorrenteProtecao, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b001, &modoCorrenteProtecao, acaoDecCorrenteProtecao, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b100, &modoRampaPWMPosicao, acaoIncRampaPWMPosicao, 500, true });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b001, &modoRampaPWMPosicao, acaoDecRampaPWMPosicao, 500, true });

    // Flash
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b101, &modoSalvarConfigFlash, acaoSalvarConfigFlash, 5000, false });
    gerenciador.adicionarAcao({ {chaves3[0], chaves3[1], chaves3[2]}, 0b101, &modoCarregarConfigFlash, acaoCarregarConfigFlash, 5000, false });
}
