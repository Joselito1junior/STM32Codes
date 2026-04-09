# lwip_rtos_mx65_modbus_tcp

Firmware para **STM32F767ZI (Nucleo-F767ZI)** que implementa um **servidor Modbus TCP** sobre LwIP + FreeRTOS.

---

## Visão geral

O projeto expõe registradores Holding Modbus via TCP na porta 502, permitindo que qualquer cliente Modbus (mestre) leia e escreva registradores remotamente pela rede Ethernet.

### Arquitetura em camadas

```
freertos.c                   → ponto de entrada RTOS; inicializa LwIP e sobe a stack
    └── APP_Network          → traduz configuração de alto nível para o Modbus
            └── LI_modbus    → glue layer: registradores, contexto slave, cria tasks
                    └── tcp_modserver     → callbacks LwIP para Modbus TCP
                    └── stm_modbus        → biblioteca Modbus (TCP e RTU)
```

### Fluxo de execução

```
StartDefaultTask
    ├── MX_LWIP_Init()               ← inicializa o stack LwIP + Ethernet
    ├── APP_Network_Init(&net_cfg)
    │       └── LI_Modbus_Init(TCP, SERVER)
    │               └── osThreadNew(li_modbus_tcp_server_task)
    │                       ├── tcp_modserver_init()    ← registra callbacks LwIP na porta 502
    │                       └── for(;;)
    │                             ├── ethernetif_input()    ← recebe pacotes Ethernet
    │                             └── sys_check_timeouts()  ← mantém timers LwIP
    └── osThreadSuspend()            ← defaultTask não tem mais nada a fazer
```

---

## Configuração

Toda a configuração editável está centralizada em **um único arquivo**:

```
Core/Inc/LI_modbus_config.h
```

| Define | Padrão | Descrição |
|--------|--------|-----------|
| `LI_MODBUS_TRANSPORT` | `0` | `0` = TCP, `1` = RTU (futuro) |
| `LI_MODBUS_ROLE` | `0` | `0` = Server, `1` = Client (futuro) |
| `LI_MODBUS_TCP_PORT` | `502` | Porta TCP do servidor Modbus |
| `LI_MODBUS_UNIT_ID` | `1` | Endereço/Unit ID do slave (1–247) |
| `LI_MODBUS_NUM_REGS` | `10` | Número de holding registers expostos |

> Não edite `stm_modbus_config.h` para parâmetros de aplicação — esse arquivo é parte da biblioteca e contém apenas limites de protocolo.

---

## Estrutura de arquivos relevantes

```
Core/
  Inc/
    LI_modbus_config.h      ← ÚNICO arquivo de configuração da aplicação
    LI_modbus.h             ← API da camada Modbus (enums Transport/Role, funções)
    APP_Network.h           ← API de inicialização de rede
  Src/
    LI_modbus.c             ← glue layer: tabela de registradores, slave context, tasks
    APP_Network.c           ← inicialização de alto nível
    freertos.c              ← tasks RTOS e ponto de entrada

MyMiddlewares/Third_Party/stm_modbus/
  inc/
    stm_modbus.h            ← tipos, constantes de protocolo, API da biblioteca
    stm_modbus_config.h     ← limites de protocolo (PDU size, FC03 max regs)
  src/
    stm_modbus.c            ← implementação Modbus TCP/RTU
    tcp_modserver.c         ← servidor TCP LwIP com processamento Modbus
```

---

## Dependência entre headers

```
LI_modbus_config.h          ← editar aqui
    ▲ include
stm_modbus_config.h         ← limites de protocolo + reexporta aliases MODBUS_APP_*
    ▲ include
stm_modbus.h                ← biblioteca Modbus
    ▲ include
LI_modbus.h                 ← enums Transport/Role + API pública
    ▲ include
APP_Network.h               ← APP_Network_Config_t + APP_Network_Init()
```

---

## Requisitos

| Componente | Versão |
|------------|--------|
| Hardware | Nucleo-F767ZI (STM32F767ZI) |
| IDE | STM32CubeIDE 1.19.0 |
| STM32CubeMX | 6.5+ |
| RTOS | FreeRTOS (via CMSIS-OS v2) |
| Stack TCP/IP | LwIP 2.x |
| Biblioteca Modbus | stm_modbus (MyMiddlewares) |

---

## Como usar

1. Abra o projeto no **STM32CubeIDE**.
2. Ajuste as configurações em `Core/Inc/LI_modbus_config.h` conforme necessário.
3. Compile e grave o firmware na placa.
4. Conecte a placa à rede via cabo Ethernet.
5. Use qualquer cliente Modbus TCP (ex: [Modbus Poll](https://www.modbustools.com/modbus_poll.html), `pymodbus`, `mbpoll`) apontando para o IP da placa na porta `502`.

### Exemplo com `mbpoll` (Linux)

```bash
# Leitura dos registradores 0 a 9 (10 registradores)
mbpoll -a 1 -r 1 -c 10 <IP_DA_PLACA>
```

### Exemplo com Python (`pymodbus`)

```python
from pymodbus.client import ModbusTcpClient

client = ModbusTcpClient("<IP_DA_PLACA>", port=502)
client.connect()
result = client.read_holding_registers(address=0, count=10, slave=1)
print(result.registers)
client.close()
```

---

## Modos futuros (não implementados)

- `LI_MODBUS_TRANSPORT = 1` → Modbus RTU via UART
- `LI_MODBUS_ROLE = 1` → Modbus Client/Master (TCP e RTU)


## Configuração para usar a USART2 STM32 como server
1. Mode: Asynchronous
2. DMA Settings:
    USART2RX: Priority: Very High
3. DMA Request Settings
    Mode: Circular
4. NVIC Settings
    USART2 global interrupt: Enable