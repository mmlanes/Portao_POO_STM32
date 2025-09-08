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
#include "AcoesChaves.h"
#include "MensagemLCD.h"
#include "ConfigFlash.h"
#include "Motor.h"
#include "Portao.h"


// Arquivos para deixar o conteúdo de main.cpp menor
// (Não são bibliotecas, são apenas arquivos .h com código específico do projeto)
#include "MetodosMain.h"
#include "ObjetosMain.h"


void setup()
{
    ConfigFlash cfg;
    cfg.obterStringCompleta();
    Serial2.println("CFG: " + cfg.obterStringCompleta() + "|");
    CarregarVariaveisFlash();
    delay(200);
    ModosOperacao::modoSeguinte();
}

void loop()
{
    dPWM.definirValor(pwm.obterDpwmAtual());                    // Atualiza Variavel<uint8_t> dPWM
    ChaveSTM32::atualizarTodas();                               // Atualiza todas as chaves
    motor.monitorar();                                          // Atualiza o estado do motor
    pwm.atualizaRampa();                                        // Atualiza a rampa de PWM
    AcoesChaves::atuarTodas(ModosOperacao::modoAtual());        // Atualiza as ações das chaves
    iMedio.definirValor(adc.obterGrandezaMediaPeriodica(500));  // Atualiza a média do ADC
    for (auto* msg : MensagemLCD::todas())                      // Envia mensagens para o LCD
        m.enviarMensagem(msg);
    CarregarSalvarVariaveisFlash();                             // Carrega ou salva variáveis na Flash
}


