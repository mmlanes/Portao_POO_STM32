
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

