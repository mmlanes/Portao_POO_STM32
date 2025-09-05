#pragma once

#include <Arduino.h>
#include "IChave.h"

/// Implementação para STM32 Bluepill usando Arduino.h
class ChaveSTM32 : public IChave 
{
private:
    uint8_t _pino;
    bool _estadoAtivo;        // define se a chave é ativa em HIGH ou LOW
    bool _estadoLogado;       // estado lógico atual 
    bool _ultimoLeitura;      // última leitura válida
    unsigned long _inicioAtivo; // quando começou a ficar ativa
    unsigned long _tempoAtivo;  // tempo acumulado em ms

    // Controle de debounce
    static const unsigned long DEBOUNCE_DELAY = 50; // 50 ms
    unsigned long _ultimoDebounce;

public:
    /// Construtor
    ChaveSTM32(uint8_t pino, bool estadoAtivo = LOW) 
        : _pino(pino), _estadoAtivo(estadoAtivo), _estadoLogado(false),
          _ultimoLeitura(false), _inicioAtivo(0), _tempoAtivo(0),
          _ultimoDebounce(0)
    {
        pinMode(_pino, INPUT_PULLUP);
    }

    /// Atualiza o estado da chave (chamar no loop)
    void atualizar() override 
    {
        bool leitura = digitalRead(_pino);

        // Debounce: só atualiza se estiver estável por DEBOUNCE_DELAY
        if (leitura != _ultimoLeitura) 
        {
            _ultimoDebounce = millis();
            _ultimoLeitura = leitura;
            return;
        }

        if ((millis() - _ultimoDebounce) < DEBOUNCE_DELAY)
            return;

        // Atualiza estado lógico e tempo ativo
        if (_estadoLogado != leitura) 
        {
            _estadoLogado = leitura;

            if (_estadoLogado == _estadoAtivo) 
            {
                _inicioAtivo = millis(); // começou a ficar ativo
            }
            else if (_inicioAtivo != 0) 
            {
                _tempoAtivo += millis() - _inicioAtivo; // acumula tempo
                _inicioAtivo = 0;
            }
        }
    }

    /// Retorna se a chave está ativa
    bool estaAtiva() override 
    {
        return _estadoLogado == _estadoAtivo;
    }

    /// Retorna o tempo acumulado em que a chave esteve ativa
    unsigned long tempoAtiva() override 
    {
        unsigned long total = _tempoAtivo;
        if (_estadoLogado == _estadoAtivo && _inicioAtivo != 0) 
        {
            total += millis() - _inicioAtivo;
        }
        return total;
    }
};
