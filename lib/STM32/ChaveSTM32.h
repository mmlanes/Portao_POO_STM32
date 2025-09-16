#pragma once

#include <Arduino.h>
#include "IChave.h"
#include <vector>

/// Implementação para STM32 Bluepill usando Arduino.h
class ChaveSTM32 : public IChave 
{
private:
    uint8_t _pino;
    bool _estadoAtivo;        // define se a chave é ativa em HIGH ou LOW
    bool _estadoLogado;       // estado lógico atual 
    bool _ultimoLeitura;      // última leitura válida
    unsigned long _inicioAtivo; // quando começou a ficar ativa

    // Controle de debounce
    static const unsigned long DEBOUNCE_DELAY = 50; // 50 ms
    unsigned long _ultimoDebounce;

    // Função estática para acessar a lista de chaves (padrão Singleton)
    static std::vector<ChaveSTM32*>& obterListaChaves() 
    {
        static std::vector<ChaveSTM32*> lista;
        return lista;
    }

public:
    /// Construtor
    ChaveSTM32(uint8_t pino, bool estadoAtivo = LOW) 
        : _pino(pino), _estadoAtivo(estadoAtivo), _estadoLogado(false),
          _ultimoLeitura(estadoAtivo), _inicioAtivo(0), _ultimoDebounce(0)
    {
        pinMode(_pino, INPUT_PULLUP);
        
        // Adicionar esta instância à lista global
        obterListaChaves().push_back(this);
    }

    /// Destructor - remove da lista
    ~ChaveSTM32() 
    {
        auto& lista = obterListaChaves();
        auto it = std::find(lista.begin(), lista.end(), this);
        if (it != lista.end()) {
            lista.erase(it);
        }
    }

    /// Método estático para atualizar TODAS as chaves de uma vez
    static void atualizarTodas() 
    {
        for (ChaveSTM32* chave : obterListaChaves()) {
            if (chave != nullptr) {
                chave->atualizar();
            }
        }
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

        // Atualiza estado lógico
        if (_estadoLogado != leitura) 
        {
            _estadoLogado = leitura;

            if (_estadoLogado == _estadoAtivo) 
            {
                _inicioAtivo = millis(); // começou a ficar ativo
            }
            else
            {
                _inicioAtivo = 0; // zerar tempo quando inativa
            }
        }
    }

    /// Retorna se a chave está ativa
    bool estaAtiva() override 
    {
        return _estadoLogado == _estadoAtivo;
    }

    /// Retorna o tempo contínuo em que a chave está ativa
    unsigned long tempoAtiva() override 
    {
        if (_estadoLogado == _estadoAtivo && _inicioAtivo != 0)
            return millis() - _inicioAtivo;
        else
            return 0;
    }
};
