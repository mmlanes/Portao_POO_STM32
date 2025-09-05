#pragma once

#include <Arduino.h>

/// Interface para qualquer chave em microcontrolador
class IChave 
{
public:
    virtual bool estaAtiva() = 0;            // retorna true se a chave está ativa
    virtual unsigned long tempoAtiva() = 0;  // tempo acumulado em que ficou ativa
    virtual void atualizar() = 0;            // deve ser chamado periodicamente no loop
    virtual ~IChave() {}
};
