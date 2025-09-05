#pragma once

#include <Arduino.h>
#include "IChave.h"

/// Implementação para STM32 Bluepill usando Arduino.h
class ChaveSTM32 : public IChave 
{
private:
    uint8_t _pino;
    bool _ativoAlto;        // define se a chave é ativa em HIGH ou LOW
    bool _estado;           // estado lógico atual (true = ativa)
    bool _ultimoLeitura;    // última leitura válida
    unsigned long _inicioAtivo;   // quando começou a ficar ativa
    unsigned long _tempoAtivo;    // tempo acumulado em ms

    // Controle de debounce
    static const unsigned long DEBOUNCE_DELAY = 50; // 50 ms
    unsigned long _ultimoDebounce;

public:
    /// Construtor
    /// @param pino -> número do pino no STM32
    /// @param ativoAlto -> true se a chave é ativa em HIGH, false se ativa em LOW
    ChaveSTM32(uint8_t pino, bool ativoAlto = true) 
        : _pino(pino), _ativoAlto(ativoAlto), _estado(false),
          _ultimoLeitura(false), _inicioAtivo(0), _tempoAtivo(0),
          _ultimoDebounce(0)
    {
        pinMode(_pino, INPUT); // pode trocar para INPUT_PULLUP se necessário
    }

    /// Atualiza o estado da chave (chamar no loop)
    void atualizar() override 
    {
        bool leituraBruta = digitalRead(_pino);
        bool leituraNormalizada = _ativoAlto ? leituraBruta : !leituraBruta;

        if (leituraNormalizada != _ultimoLeitura) {
            _ultimoDebounce = millis(); // reinicia contador de debounce
        }

        if ((millis() - _ultimoDebounce) > DEBOUNCE_DELAY) 
        {
            if (leituraNormalizada != _estado) 
            {
                _estado = leituraNormalizada;
                if (_estado) // chave acabou de ser ativada       
                    _inicioAtivo = millis();
                else // chave acabou de ser desativada → acumula tempo  
                    _tempoAtivo += millis() - _inicioAtivo;
            }
        }
        _ultimoLeitura = leituraNormalizada;
    }

    /// Retorna se a chave está ativa
    bool estaAtiva() override 
    {
        return _estado;
    }

    /// Retorna o tempo acumulado em que a chave esteve ativa
    unsigned long tempoAtiva() override 
    {
        unsigned long total = _tempoAtivo;
        if (_estado) 
            total += millis() - _inicioAtivo;
        return total;
    }
};
