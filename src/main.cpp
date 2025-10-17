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
#include "Protecao.h"
#include "ControladorPortao.h"


// Arquivos para deixar o conteúdo de main.cpp menor
// (Não são bibliotecas, são apenas arquivos .h com código específico do projeto)
#include "MetodosMain.h"
#include "ObjetosMain.h"


void setup()
{
    #ifndef DISABLE_DEBUG
    Serial2.println("Iniciando sistema...");
    #endif
    ConfigFlash cfg;
    cfg.obterStringCompleta();
    #ifndef DISABLE_DEBUG
    Serial2.println("CFG: " + cfg.obterStringCompleta() + "|");
    #endif
    CarregarVariaveisFlash();
    delay(200);
    pwm.defineFrequencia(freqPWM.obterValor());
    uint32_t periodoPWMUs = 1000000 / freqPWM.obterValor();
    adc = FastADC_PA0_STM32_S::PeriodoTotal(iMedio, iPico, 20, periodoPWMUs, 100, &timer2);
    pinMode(PC13, OUTPUT);
    digitalWrite(PC13, LOW); // LED apagado,
}

void loop()
{
    // Atualização de variáveis para visualização e controle
    dPWM.definirValor(pwm.obterDpwmAtual());                    // Atualiza Variavel<uint8_t> dPWM
    posicaoPortao.definirValor(portao.obterPosicaoAtualString()); // Atualiza Variavel<uint8_t> posPortao
    operacaoPortao.definirValor(controladorPortao.obterOperacaoAtualString());// Atualiza Variavel<uint8_t> operPortao
    
    // Ações essenciais do sistema
    ChaveSTM32::atualizarTodas();                               // Atualiza todas as chaves
    motor.monitorar();                                          // Atualiza o estado do motor
    portao.monitorar();                                         // Atualiza o portão
    //protecao.monitorar();                                       // Monitora as proteções
    controladorPortao.monitorar();                              // Monitora o controlador do portão

    //AcoesChaves::atuarTodas(ModosOperacao::modoAtual());        // Atualiza as ações das chaves
    AcoesChavesCombinadas::atuarTodas(ModosOperacao::obterModoAtual());

    AtualizaImedioIpico();                                 // Atualiza Imedio e Ipico periodicamente
    monitorarProtecao();                                   // Monitora as proteções

    for (auto* msg : MensagemLCD::todas())                      // Envia mensagens para o LCD
        m.enviarMensagem(msg);
    CarregarSalvarVariaveisFlash();                             // Carrega ou salva variáveis na Flash

    //imprimirBufferImedioMediaMovelPeriodico();

    //delay(500);
}


