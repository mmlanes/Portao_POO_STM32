# Portao_POO_STM32

Este projeto implementa o controle de um portão eletrônico utilizando programação orientada a objetos (POO) em um microcontrolador STM32. Ele organiza a lógica do sistema em classes para facilitar a manutenção, reutilização e expansão do código.

## Estrutura do Projeto

### Classes Principais

1. **ChaveSTM32**
   - Representa uma chave ou sensor conectado a um pino do microcontrolador.
   - Gerencia o estado lógico do pino (ex.: HIGH ou LOW).
   - Exemplo de uso:
     ```cpp
     ChaveSTM32 fcS(PB12, LOW);
     ```
     O objeto `fcS` é inicializado para monitorar o pino `PB12` com estado inicial `LOW`.

2. **AcoesChaves**
   - Gerencia ações baseadas no estado de múltiplas chaves.
   - Recebe referências para objetos `ChaveSTM32` e executa ações específicas dependendo do estado das chaves.
   - Exemplo de uso:
     ```cpp
     AcoesChaves trocarModo(chave1, chave2, chave3);
     trocarModo.atuar();
     ```

### Funcionalidades Implementadas

- **Monitoramento de Chaves:**
  - O sistema monitora o estado de sensores ou chaves conectados ao microcontrolador.
  - A lógica para verificar se as chaves estão ativas é encapsulada na classe `ChaveSTM32`.

- **Ações Baseadas em Chaves:**
  - A classe `AcoesChaves` permite executar ações específicas quando determinadas condições são atendidas (ex.: todas as chaves estão ativas).

### Estrutura de Arquivos

- `ChavesSTM32.h` e `ChavesSTM32.cpp`: Implementação da classe `ChaveSTM32`.
- `AcoesChaves.h` e `AcoesChaves.cpp`: Implementação da classe `AcoesChaves`.
- `main.cpp`: Arquivo principal que inicializa os objetos e executa a lógica do sistema.

## Estado Atual

- A classe `ChaveSTM32` foi implementada e está funcional.
- A classe `AcoesChaves` foi criada para gerenciar múltiplas chaves e executar ações baseadas em seus estados.
- O código principal (`main.cpp`) inicializa objetos das classes e executa a lógica básica de controle.
- O sistema está em fase inicial de desenvolvimento, com foco no monitoramento de chaves e execução de ações simples.

## Próximos Passos

1. Implementar a lógica completa de controle do portão.
2. Adicionar testes para validar o comportamento das classes.
3. Documentar mais detalhadamente as funções e métodos.
4. Integrar com outros componentes do sistema, como motores e sensores adicionais.

## Como Compilar

1. Certifique-se de que você possui o ambiente de desenvolvimento STM32 configurado (ex.: STM32CubeIDE ou Makefile com `arm-none-eabi-gcc`).
2. Compile o projeto utilizando o ambiente configurado.
3. Faça o upload do firmware para o microcontrolador STM32.

## Contribuição

Contribuições são bem-vindas! Sinta-se à vontade para abrir issues ou enviar pull requests.

## Licença

Este projeto está licenciado sob a [MIT License](LICENSE).cd caminho/para/Portao_POO_STM32cd caminho/para/Portao_POO_STM32cd caminho/para/Portao_POO_STM32cd caminho/para/Portao_POO_STM32cd caminho/para/Portao_POO_STM32