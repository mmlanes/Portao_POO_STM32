
# pragma once

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

#include "ObjetosMain.h"

void CarregarVariaveisFlash(void);
void CarregarSalvarVariaveisFlash(void);
void ResetManual();

void CarregarSalvarVariaveisFlash(void)
{
    if (carregarConfigFlash.obterValor())
    {
        carregarConfigFlash.definirValor(false);
        CarregarVariaveisFlash();
        m.enviarMensagem("Config Flash", "Carregada", " ", "aguarde 3s");
        delay(1000);
    }

    if (salvarConfigFlash.obterValor())
    {
        salvarConfigFlash.definirValor(false);
        ConfigFlash cfg;
        String C = VariavelBase::todasPersistentesParaString();
        Serial2.println("Salvando: " + C);
        cfg.SalvarStringConfig(C);
        m.enviarMensagem("Config Flash", "Salva", " ", "aguarde 3s");
        delay(1000);
    }
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
                valorStr.replace("m", "e-3"); // permite usar 'm' como sufixo de mili
                valorStr.replace("u", "e-6"); // permite usar 'u' como sufixo de micro
                valorStr.replace("k", "e+3"); // permite usar 'k' como sufixo de kilo
                valorStr.replace("M", "e+6"); // permite usar 'M' como sufixo de mega
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

void imprimirBufferImedioMediaMovelPeriodico(uint32_t intervaloMs = 5000) 
{
    static unsigned long ultimoTempo = 0;
    unsigned long agora = millis();

    if (agora - ultimoTempo >= intervaloMs) 
    {
        ultimoTempo = agora;

        adc->desanexarInterrupcao();   // isr não será chamada
        adc->pausarTimer();            // se houver função para pausar o timer
        adc->imprimirMediaMovel();
        adc->anexarInterrupcao();      // isr pode rodar novamente
        adc->resumirTimer();           // se houver função para retomar timer
    }
}

void AtualizaImedioIpico(void)
{
    static unsigned long ultimoTempo = 0;
    unsigned long agora = millis();
    if (agora - ultimoTempo < 1000) return; // Atualiza no máximo a cada 1000ms
    ultimoTempo = agora;

    // ADC Nulo
    float ADC0 = adc->obterValorNuloAdc();
    // Constante de ajuste do ADC
    float AdjADC = adjADC.obterValor();
    // Cálculo de Imedio 
    float mediaADC = adc->obterMediaMovel();
    float Im = (mediaADC - ADC0)  * AdjADC;
    if (Im < 0.1) Im = 0;
    iMedio.definirValor(Im);  // Atualiza a média do ADC
    // Cálculo de Ipico 
    float maiorADC = adc->obterMaiorMediaSimples();
    //float maiorADC = adc->obterMaiorMediaSimples();
    //float maiorADC = adc->obterMaiorAmostra();
    float Ip = (maiorADC - ADC0)  * AdjADC;
    if (Ip < 0.1) Ip = 0;
    iPico.definirValor(Ip);  // Atualiza o pico do ADC

    //Serial2.println("Im=" + String(Im, 3) + " Ip=" + String(Ip, 3));
}

void monitorarProtecao()
{
    // Proteção por sobrecorrente
    if (protecaoSobrecorrente.obterValor())
    {
        if (iMedio.obterValor() >= iProt.obterValor())
            protecaoSobrecorrenteAtuada.definirValor(true);
    }

    // Proteção por encoder parado
    if (protecaoEncoderParado.obterValor())
    {
        if (encAB.estaParado())
            protecaoEncoderParadoAtuada.definirValor(true);
        else
            protecaoEncoderParadoAtuada.definirValor(false);
    }
}

void ResetManual()
{
    NVIC_SystemReset();
}