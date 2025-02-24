# lwip_rtos_mx65 (Tested for Nucleo-F767ZI)

This project is a firmware implementation for the STM32 microcontroller using the LWIP (Lightweight IP) stack and FreeRTOS. It is designed to provide network connectivity and real-time operating system capabilities.

## Features

- **LWIP Integration**: Provides a lightweight TCP/IP stack for network communication.
- **FreeRTOS**: Real-time operating system for task management.
- **STM32 Support**: Compatible with STM32 microcontrollers.
- **Modular Codebase**: Easy to extend and maintain.

## Requirements

- **STM32 Development Board** (Tested on Nucleo-F767ZI)
- **STM32CubeMX (Version 6.5)**: For generating initialization code.
- **Keil MDK or STM32CubeIDE**: For compiling and debugging the firmware.
- **LWIP**: Lightweight IP stack.
- **FreeRTOS**: Real-time operating system.

## Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/yourusername/lwip_rtos_mx65.git
    ```
2. Open the project in STM32CubeMX (Version 6.5) and generate the initialization code.
3. Open the project in your preferred IDE (Keil MDK or STM32CubeIDE).
4. Compile and flash the firmware to your STM32 development board.

## Usage

1. Connect your STM32 development board to the network.
2. Power on the board and ensure it is properly connected.
3. The firmware will initialize the network stack and start the FreeRTOS scheduler.
4. Use a network client to communicate with the STM32 board.

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact

For any questions or inquiries, please contact [your email].
