#pragma once
#include <Arduino.h>
#include <type_traits>
#include <math.h>
#include <vector>

/// Interface base para qualquer variável
class IVariavel 
{
public:
    virtual String obterNome() const = 0;
    virtual String paraString() const = 0;
    virtual bool ehPersistente() const = 0;
    virtual ~IVariavel() {}
};
