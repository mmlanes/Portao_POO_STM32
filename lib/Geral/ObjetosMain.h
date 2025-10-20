# pragma once

// Configuração do hardware
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
#include "ChavesCombinadas.h"
#include "AcoesChavesCombinadas.h"
#include "MensagemLCD.h"
#include "ConfigFlash.h"
#include "Motor.h"
#include "Portao.h"
#include "ControladorPortao.h"
#include "Protecao.h"


void ResetManual();  // Declaração adiantada


HardwareSerial Serial2(PA3, PA2); // RX, TX

Mensagem m(Serial2, 115200, 20, 4); 

ChaveSTM32 fcS(PB12, LOW);
ChaveSTM32 fcI(PB13, LOW);
ChaveSTM32 fcE(PB14, LOW);
ChaveSTM32 btnA(PB5, LOW);
ChaveSTM32 btnF(PA12, LOW); // Era PB3 mas troquei com o B (PA12) do encoder
ChaveSTM32 btnP(PB4, LOW);

// Variáveis para encoder
Variavel<int32_t> encPos("E.p", 0, -100e3, 100e3, 1, false);
Variavel<uint8_t> encPosPartida0a100("E.p.St", 10, 0, 100, 1, true);
Variavel<uint8_t> encPosParada0a100("E.p.Pr", 90, 0, 100, 1, true);
Variavel<bool> encAtivo("E.a", true, true);
Variavel<uint32_t> encMax("E.m", 2750, 100, 10e3, 1, true);
Variavel<bool> encRev("E.r", false, true);
// Variáveis para motor e PWM
Variavel<uint16_t> freqPWM("F", 500, 100, 10e3, 10, true);
Variavel<uint8_t> dPWM("D", 0, 0, 100, 0, false); // passo=0, serve só para monitorar
Variavel<uint8_t> dPWMMax("D.m", 60, 10, 100, 1, true);
Variavel<uint8_t> dPWMPartida("D.St", 15, 0, 100, 1, true);
Variavel<uint8_t> dPWMParada("D.Pr", 10, 0, 100, 1, true);
Variavel<float> acelPWM("A", 1.0f, 0.1f, 100.0, 0.1f, true);
Variavel<float> adjADC("A.adc", 1.7e-3f, 0.1e-3f, 10.0e-3f, 0.1e-3f, true);
Variavel<float> iMedio("iM", 0.0f, 0.0f, 20.0f, 0.0f, false);
Variavel<float> iPico("iP", 0.0f, 0.0f, 20.0f, 0.0f, false);
// Variáveis para portão
Variavel<String> posicaoPortao("p.P", "", false);
Variavel<String> operacaoPortao("o.P", "", false);
Variavel<float> rampaPWMPosicao("r.P", 2.0f, 0.1f, 20.0f, 0.1f, true);
// Variáveis para salvar e carregar configuração
Variavel<bool> salvarConfigFlash("s.C.F", false, false);
Variavel<bool> carregarConfigFlash("c.D.F", false, false);
// Variáveis para proteção
Variavel<bool> protecaoEncoderParadoAtuada("p.E.Pr.a", false, false);
Variavel<bool> protecaoSobrecorrenteAtuada("p.SC.A", false, false);
Variavel<bool> protecaoEncoderParado("u.P.E.Pr", true, true);
Variavel<bool> protecaoSobrecorrente("u.P.SC", true, true);
Variavel<float> iProt("iProt", 5.0f, 1.0f, 20.0f, 0.1f, true);

EncoderSTM32 encAB(encPos, encMax, encRev, PA15, PB3, true, false, false);

ModosOperacao modoNormal("N:");
ModosOperacao modoMonitorGeral("Monitor geral");
ModosOperacao modoProtEncParadoAtuado("Prot. EncPar Atuada");
ModosOperacao modoProtSobrecorrenteAtuado("Prot. Imax");
ModosOperacao modoUsarProtEncParado("Usar Prot. EncPar. ?");
ModosOperacao modoUsarProtSobrecorrente("Usar Prot. Imax ?");
ModosOperacao modoEncAtivo("Encoder ativo");
ModosOperacao modoEncMaximo("Encoder maximo");
ModosOperacao modoEncReverso("Encoder reverso");

ModosOperacao modoPosPartida("Posicao partida");
ModosOperacao modoPosParada("Posicao parada");
ModosOperacao modoDPWMPartida("DPWM partida");
ModosOperacao modoDPWMParada("DPWM parada");

ModosOperacao modoFreqPWM("Frequencia PWM");
ModosOperacao modoDPWM("D PWM");
ModosOperacao modoAcelPWM("Aceleracao PWM");
ModosOperacao modoConstanteADC("Constante ADC");
ModosOperacao modoCorrenteProtecao("Corrente protecao");
ModosOperacao modoRampaPWMPosicao("Rampa PWM/Posicao");
ModosOperacao modoSalvarConfigFlash("Salvar config Flash");
ModosOperacao modoCarregarConfigFlash("Carrega config Flash");

// Constantes para reduzir uso de flash  
String trocarTela = "Tela P+A >> e P+F <<";
String incDecBool = "Valor: BtA=1 e BtF=0";
String incDecNum = "Valor: BtA=+ e BtF=-";
MensagemLCD mNormal(&modoNormal, "$modo$ PP=$p.P$ OP=$o.P$ EA=$E.a$", "D=$D$ AB=$E.p$ PEI=$p.E.Pr.a$$p.SC.A$", "Rst:A+F 5s Im=$iM$A", trocarTela);
//MensagemLCD mNormal(&modoNormal, "$modo$", "D=$D$ AB=$E.p$ Im=$iM$A", "PP.OP.RA.PEI=$p.P$.$o.P$.$E.a$.$protEncParado$$protSobrecorrente$", trocarTela);
MensagemLCD mProtEncParado(&modoProtEncParadoAtuado, "$modo$", "ativada = $p.E.Pr.a$", "(atua se Enc Ativo)", trocarTela);
MensagemLCD mProtSobrecorrente(&modoProtSobrecorrenteAtuado, "$modo$", "ativada = $p.SC.A$", " ", trocarTela);
MensagemLCD mUsarProtEncParado(&modoUsarProtEncParado, "$modo$", "usar = $u.P.E.Pr$", " ", trocarTela);
MensagemLCD mUsarProtSobrecorrente(&modoUsarProtSobrecorrente, "$modo$", "usar = $u.P.SC$", " ", trocarTela);
MensagemLCD mMonitorGeral(&modoMonitorGeral, "$modo$", "Pos=$E.p$/$E.m$ Rev=$E.r$", " ", trocarTela);
MensagemLCD mAtivarEnc(&modoEncAtivo, "$modo$", "$E.a$", incDecBool, trocarTela);
MensagemLCD mEncMaximo(&modoEncMaximo, "$modo$", "$E.m$", incDecNum, trocarTela);
MensagemLCD mEncReverso(&modoEncReverso, "$modo$", "$E.r$", incDecBool, trocarTela);
MensagemLCD mPosPartida(&modoPosPartida, "$modo$", "$E.p.St$ %", incDecNum, trocarTela);
MensagemLCD mPosParada(&modoPosParada, "$modo$", "$E.p.Pr$ %", incDecNum, trocarTela);
MensagemLCD mDPWMPartida(&modoDPWMPartida, "$modo$", "$D.St$ %", incDecNum, trocarTela);
MensagemLCD mDPWMParada(&modoDPWMParada, "$modo$", "$D.Pr$ %", incDecNum, trocarTela);
MensagemLCD mFreqPWM(&modoFreqPWM, "$modo$", "$F$ Hz", incDecNum, trocarTela);
MensagemLCD mDPWM(&modoDPWM, "$modo$", "$D.m$ %", incDecNum, trocarTela);
MensagemLCD mAcelPWM(&modoAcelPWM, "$modo$", "$A$ dPwm/s", incDecNum, trocarTela);
MensagemLCD mConstADC(&modoConstanteADC, "$modo$", "$A.adc$", incDecNum, trocarTela);
MensagemLCD mCorrenteProtecao(&modoCorrenteProtecao, "$modo$", "($iM$)|Ip=$iProt$ A", incDecNum, trocarTela);
MensagemLCD mRampaPWMPosicao(&modoRampaPWMPosicao, "$modo$", "$r.P$ %PWM/%PosEnc", incDecNum, trocarTela);
MensagemLCD mSalvarFlash(&modoSalvarConfigFlash, "$modo$", "$s.C.F$", "Bts(A+F) 5s salvar", trocarTela);
MensagemLCD mCarregarFlash(&modoCarregarConfigFlash, "$modo$", "$c.D.F$", "Bts(A+F) 5s carreg", trocarTela);

HardwareTimer timer2(TIM2);  // criado fora da classe
FastADC_PA0_STM32_S* adc = nullptr;

auto& pwm = PWM_PB1_STM32_S::getInstance(freqPWM, dPWM, dPWMMax, acelPWM); // 5000 Hz e função de leitura do ADC

Motor motor(PB11, PB10, pwm); // pino KD, KE, PWM
Portao portao(fcS, fcI, encAB, motor, encAtivo, encPosPartida0a100, encPosParada0a100, dPWMPartida, dPWMParada, rampaPWMPosicao);
Protecao protecao(portao, iProt, 
                  protecaoEncoderParadoAtuada, protecaoSobrecorrenteAtuada,
                  protecaoEncoderParado, protecaoSobrecorrente);
ControladorPortao controladorPortao(portao, protecao);

// Para em qualquer modo com btnP
ChavesCombinadas aPf_N({&btnA, &btnP, &btnF}, {false, true, false}); // pararPortao
AcoesChavesCombinadas pararPortaoN(aPf_N, nullptr, [](uint8_t v){ controladorPortao.parar(); }, 500, false);
// Reset manual
ChavesCombinadas ApF_N({&btnA, &btnP, &btnF}, {true, false, true}); // reset
AcoesChavesCombinadas resetar(ApF_N, &modoNormal, [](uint8_t v){ ResetManual(); }, 5000, false);
// Trocar modo
ChavesCombinadas APF_N({&btnA, &btnP, &btnF}, {true, true, true}); // Vai para modo normal
AcoesChavesCombinadas irModoNormal(APF_N, nullptr, [](uint8_t){ ModosOperacao::definirModoAtualPorPosicao(0); }, 2000, true, 50000, 1, 60000, 1); 
ChavesCombinadas APf_N({&btnA, &btnP, &btnF}, {true, true, false}); // Avançar modo
AcoesChavesCombinadas avancarModo(APf_N, nullptr, [](uint8_t){ ModosOperacao::modoSeguinte(); }, 1000, true, 50000, 1, 60000, 1); 
ChavesCombinadas aPF_N({&btnA, &btnP, &btnF}, {false, true, true}); // Voltar modo
AcoesChavesCombinadas voltarModo(aPF_N, nullptr, [](uint8_t){ ModosOperacao::modoAnterior(); }, 1000, true, 50000, 1, 60000, 1); 
// Modo normal
ChavesCombinadas Apf_MN({&btnA, &btnP, &btnF}, {true, false, false}); // abrirPortao
AcoesChavesCombinadas abrirPortao(Apf_MN, &modoNormal, [](uint8_t v){ controladorPortao.abrir(); }, 1000, false);
ChavesCombinadas apF_MN({&btnA, &btnP, &btnF}, {false, false, true}); // fecharPortao
AcoesChavesCombinadas fecharPortao(apF_MN, &modoNormal, [](uint8_t v){ controladorPortao.fechar(); }, 1000, false);
ChavesCombinadas aPf_MN({&btnA, &btnP, &btnF}, {false, true, false}); // pararPortao
AcoesChavesCombinadas pararPortao(aPf_MN, &modoNormal, [](uint8_t v){ controladorPortao.parar(); }, 50, false);
// Ativar/desativar FLAG ATUADO protecao por encoder parado
ChavesCombinadas Apf_MPEP({&btnA, &btnP, &btnF}, {true, false, false}); // ativarProtEnc
AcoesChavesCombinadas ativarFlagProtEncP(Apf_MPEP, &modoProtEncParadoAtuado, [](uint8_t v){ protecaoEncoderParadoAtuada.incrementar(v); }, 500, false);
ChavesCombinadas apF_MPEP({&btnA, &btnP, &btnF}, {false, false, true}); // desativarProtEnc
AcoesChavesCombinadas desativarFlagProtEncP(apF_MPEP, &modoProtEncParadoAtuado, [](uint8_t v){ protecaoEncoderParadoAtuada.decrementar(v); }, 500, false);
// Ativar/desativar FLAG ATUADO protecao por sobrecorrente
ChavesCombinadas Apf_MPS({&btnA, &btnP, &btnF}, {true, false, false}); // ativarProtSobrecorrente
AcoesChavesCombinadas ativarFlagProtSobrecorrente(Apf_MPS, &modoProtSobrecorrenteAtuado, [](uint8_t v){ protecaoSobrecorrenteAtuada.incrementar(v); }, 500, false);
ChavesCombinadas apF_MPS({&btnA, &btnP, &btnF}, {false, false, true}); // desativarProtSobrecorrente
AcoesChavesCombinadas desativarFlagProtSobrecorrente(apF_MPS, &modoProtSobrecorrenteAtuado, [](uint8_t v){ protecaoSobrecorrenteAtuada.decrementar(v); }, 500, false);
// Ativar/desativar PROTECAO por encoder parado
ChavesCombinadas Apf_MUPEP({&btnA, &btnP, &btnF}, {true, false, false}); // ativarProtEnc
AcoesChavesCombinadas ativarProtEncP(Apf_MUPEP, &modoUsarProtEncParado, [](uint8_t v){ protecaoEncoderParado.incrementar(v); }, 500, false);
ChavesCombinadas apF_MUPEP({&btnA, &btnP, &btnF}, {false, false, true}); // desativarProtEnc
AcoesChavesCombinadas desativarProtEncP(apF_MUPEP, &modoUsarProtEncParado, [](uint8_t v){ protecaoEncoderParado.decrementar(v); }, 500, false);
// Ativar/desativar PROTECAO por sobrecorrente
ChavesCombinadas Apf_MUPS({&btnA, &btnP, &btnF}, {true, false, false}); // ativarProtSobrecorrente
AcoesChavesCombinadas ativarProtSobrecorrente(Apf_MUPS, &modoUsarProtSobrecorrente, [](uint8_t v){ protecaoSobrecorrente.incrementar(v); }, 500, false);
ChavesCombinadas apF_MUPS({&btnA, &btnP, &btnF}, {false, false, true}); // desativarProtSobrecorrente
AcoesChavesCombinadas desativarProtSobrecorrente(apF_MUPS, &modoUsarProtSobrecorrente, [](uint8_t v){ protecaoSobrecorrente.decrementar(v); }, 500, false);
// Modo normal: acelerado
ChavesCombinadas Apf_MN_Ac({&btnA, &btnP, &btnF}, {true, false, false}); // acelerarAbrirPortao
AcoesChavesCombinadas acelerarAbrirPortao(Apf_MN_Ac, &modoNormal, [](uint8_t v){ controladorPortao.abrirFecharAceleradoSemEncoder(); }, 2000, false);
ChavesCombinadas apF_MN_Ac({&btnA, &btnP, &btnF}, {false, false, true}); // acelerarFecharPortao
AcoesChavesCombinadas acelerarFecharPortao(apF_MN_Ac, &modoNormal, [](uint8_t v){ controladorPortao.abrirFecharAceleradoSemEncoder(); }, 2000, false);
// Modo normal: desacelerado
ChavesCombinadas ApF_MN_Des({&btnA, &btnP, &btnF}, {true, false, true}); // desacelerarPortao
AcoesChavesCombinadas desacelerarPortao(ApF_MN_Des, &modoNormal, [](uint8_t v){ controladorPortao.abrirFecharDesaceleradoSemEncoder(); }, 2000, false);
ChavesCombinadas apf_MN({&btnA, &btnP, &btnF}, {false, false, false}); // velocidadeNormalPortao
AcoesChavesCombinadas velocidadeNormalPortao(apf_MN, &modoNormal, [](uint8_t v){ controladorPortao.velocidadeNormalSemEncoder(); }, 500, false);
// Modo EncoderAtivo
ChavesCombinadas Apf_MEA({&btnA, &btnP, &btnF}, {true, false, false}); // ativarEnc
AcoesChavesCombinadas ativarEnc(Apf_MEA, &modoEncAtivo, [](uint8_t v){ encAtivo.incrementar(v); }, 500, false);
ChavesCombinadas apF_MEA({&btnA, &btnP, &btnF}, {false, false, true}); // desativarEnc
AcoesChavesCombinadas desativarEnc(apF_MEA, &modoEncAtivo, [](uint8_t v){ encAtivo.decrementar(v); }, 500, false);
// Modo EncoderMaximo
ChavesCombinadas Apf_MEM({&btnA, &btnP, &btnF}, {true, false, false}); // incEncMax
AcoesChavesCombinadas incEncMax(Apf_MEM, &modoEncMaximo, [](uint8_t v){ encMax.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
ChavesCombinadas apF_MEM({&btnA, &btnP, &btnF}, {false, false, true}); // decEncMax
AcoesChavesCombinadas decEncMax(apF_MEM, &modoEncMaximo, [](uint8_t v){ encMax.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
// Modo EncoderReverso
ChavesCombinadas Apf_MER({&btnA, &btnP, &btnF}, {true, false, false}); // encoderReverso
AcoesChavesCombinadas encoderReverso(Apf_MER, &modoEncReverso, [](uint8_t v){ encRev.incrementar(v); }, 500, false);
ChavesCombinadas apF_MER({&btnA, &btnP, &btnF}, {false, false, true}); // encoderDireto
AcoesChavesCombinadas encoderDireto(apF_MER, &modoEncReverso, [](uint8_t v){ encRev.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
// Modo Posicao Partida
ChavesCombinadas Apf_MPPt({&btnA, &btnP, &btnF}, {true, false, false}); // incPosPartida
AcoesChavesCombinadas incPosPartida(Apf_MPPt, &modoPosPartida, [](uint8_t v){ encPosPartida0a100.incrementar(v); }, 500, true, 5000, 1, 100, 1);
ChavesCombinadas apF_MPPt({&btnA, &btnP, &btnF}, {false, false, true}); // decPosPartida
AcoesChavesCombinadas decPosPartida(apF_MPPt, &modoPosPartida, [](uint8_t v){ encPosPartida0a100.decrementar(v); }, 500, true, 5000, 1, 100, 1);
// Modo Posicao Parada
ChavesCombinadas Apf_MPPd({&btnA, &btnP, &btnF}, {true, false, false}); // incPosParada
AcoesChavesCombinadas incPosParada(Apf_MPPd, &modoPosParada, [](uint8_t v){ encPosParada0a100.incrementar(v); }, 500, true, 5000, 1, 100, 1);
ChavesCombinadas apF_MPPd({&btnA, &btnP, &btnF}, {false, false, true}); // decPosParada
AcoesChavesCombinadas decPosParada(apF_MPPd, &modoPosParada, [](uint8_t v){ encPosParada0a100.decrementar(v); }, 500, true, 5000, 1, 100, 1);
// Modo DPWM Partida
ChavesCombinadas Apf_MDPt({&btnA, &btnP, &btnF}, {true, false, false}); // incDPWMPartida
AcoesChavesCombinadas incDPWMPartida(Apf_MDPt, &modoDPWMPartida, [](uint8_t v){ dPWMPartida.incrementar(v); }, 500, true, 5000, 1, 100, 1);
ChavesCombinadas apF_MDPt({&btnA, &btnP, &btnF}, {false, false, true}); // decDPWMPartida
AcoesChavesCombinadas decDPWMPartida(apF_MDPt, &modoDPWMPartida, [](uint8_t v){ dPWMPartida.decrementar(v); }, 500, true, 5000, 1, 100, 1);
// Modo DPWM Parada
ChavesCombinadas Apf_MDPd({&btnA, &btnP, &btnF}, {true, false, false}); // incDPWMParada
AcoesChavesCombinadas incDPWMParada(Apf_MDPd, &modoDPWMParada, [](uint8_t v){ dPWMParada.incrementar(v); }, 500, true, 5000, 1, 100, 1);
ChavesCombinadas apF_MDPd({&btnA, &btnP, &btnF}, {false, false, true}); // decDPWMParada
AcoesChavesCombinadas decDPWMParada(apF_MDPd, &modoDPWMParada, [](uint8_t v){ dPWMParada.decrementar(v); }, 500, true, 5000, 1, 100, 1);
// Modo Ajuste frequência PWM
ChavesCombinadas Apf_MF({&btnA, &btnP, &btnF}, {true, false, false}); // incFreqPWM
AcoesChavesCombinadas incFreqPWM(Apf_MF, &modoFreqPWM, [](uint8_t v){ freqPWM.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
ChavesCombinadas apF_MF({&btnA, &btnP, &btnF}, {false, false, true}); // decFreqPWM
AcoesChavesCombinadas decFreqPWM(apF_MF, &modoFreqPWM, [](uint8_t v){ freqPWM.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
// Modo DPWM
ChavesCombinadas Apf_MD({&btnA, &btnP, &btnF}, {true, false, false}); // incDPWM
AcoesChavesCombinadas incDPWM(Apf_MD, &modoDPWM, [](uint8_t v){ dPWMMax.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
ChavesCombinadas apF_MD({&btnA, &btnP, &btnF}, {false, false, true}); // decDPWM
AcoesChavesCombinadas decDPWM(apF_MD, &modoDPWM, [](uint8_t v){ dPWMMax.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
// Modo AcelPWM
ChavesCombinadas Apf_MA({&btnA, &btnP, &btnF}, {true, false, false}); // incAcelPWM
AcoesChavesCombinadas incAcelPWM(Apf_MA, &modoAcelPWM, [](uint8_t v){ acelPWM.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
ChavesCombinadas apF_MA({&btnA, &btnP, &btnF}, {false, false, true}); // decAcelPWM
AcoesChavesCombinadas decAcelPWM(apF_MA, &modoAcelPWM, [](uint8_t v){ acelPWM.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
// Modo Ajuste ADC
ChavesCombinadas Apf_MK({&btnA, &btnP, &btnF}, {true, false, false}); // incAdjADC
AcoesChavesCombinadas incAdjADC(Apf_MK, &modoConstanteADC, [](uint8_t v){ adjADC.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
ChavesCombinadas apF_MK({&btnA, &btnP, &btnF}, {false, false, true}); // decAdjADC
AcoesChavesCombinadas decAdjADC(apF_MK, &modoConstanteADC, [](uint8_t v){ adjADC.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
// Modo Ajuste corrente de proteção
ChavesCombinadas Apf_MP({&btnA, &btnP, &btnF}, {true, false, false}); // incCorrenteProtecao
AcoesChavesCombinadas incCorrenteProtecao(Apf_MP, &modoCorrenteProtecao, [](uint8_t v){ iProt.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
ChavesCombinadas apF_MP({&btnA, &btnP, &btnF}, {false, false, true}); // decCorrenteProtecao
AcoesChavesCombinadas decCorrenteProtecao(apF_MP, &modoCorrenteProtecao, [](uint8_t v){ iProt.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
// Modo Ajuste rampa PWM/Posição
ChavesCombinadas Apf_MRP({&btnA, &btnP, &btnF}, {true, false, false}); // incCorrenteProtecao
AcoesChavesCombinadas incRampaPWMPosicao(Apf_MRP, &modoRampaPWMPosicao, [](uint8_t v){ rampaPWMPosicao.incrementar(v); }, 500, true, 5000, 10, 10000, 100);
ChavesCombinadas apF_MRP({&btnA, &btnP, &btnF}, {false, false, true}); // decCorrenteProtecao
AcoesChavesCombinadas decRampaPWMPosicao(apF_MRP, &modoRampaPWMPosicao, [](uint8_t v){ rampaPWMPosicao.decrementar(v); }, 500, true, 5000, 10, 10000, 100);
// Modo salvar e carregar configuração na flash
ChavesCombinadas ApF_MSF({&btnA, &btnP, &btnF}, {true, false, true}); // acSalvarConfigFlash
AcoesChavesCombinadas acSalvarConfigFlash(ApF_MSF, &modoSalvarConfigFlash, [](uint8_t v){ salvarConfigFlash.incrementar(v); }, 5000, false);
ChavesCombinadas ApF_MCF({&btnA, &btnP, &btnF}, {true, false, true}); // acCarregarConfigFlash
AcoesChavesCombinadas acCarregarConfigFlash(ApF_MCF, &modoCarregarConfigFlash, [](uint8_t v){ carregarConfigFlash.incrementar(v); }, 5000, false);



