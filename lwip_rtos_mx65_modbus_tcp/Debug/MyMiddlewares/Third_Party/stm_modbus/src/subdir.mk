################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus.c \
../MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus_pdu.c \
../MyMiddlewares/Third_Party/stm_modbus/src/tcp_modserver.c 

OBJS += \
./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus.o \
./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus_pdu.o \
./MyMiddlewares/Third_Party/stm_modbus/src/tcp_modserver.o 

C_DEPS += \
./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus.d \
./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus_pdu.d \
./MyMiddlewares/Third_Party/stm_modbus/src/tcp_modserver.d 


# Each subdirectory must supply rules for building sources it contributes
MyMiddlewares/Third_Party/stm_modbus/src/%.o MyMiddlewares/Third_Party/stm_modbus/src/%.su MyMiddlewares/Third_Party/stm_modbus/src/%.cyclo: ../MyMiddlewares/Third_Party/stm_modbus/src/%.c MyMiddlewares/Third_Party/stm_modbus/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F767xx -c -I../MyMiddlewares/Third_Party/stm_modbus/inc -I../Core/Inc -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-MyMiddlewares-2f-Third_Party-2f-stm_modbus-2f-src

clean-MyMiddlewares-2f-Third_Party-2f-stm_modbus-2f-src:
	-$(RM) ./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus.cyclo ./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus.d ./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus.o ./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus.su ./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus_pdu.cyclo ./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus_pdu.d ./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus_pdu.o ./MyMiddlewares/Third_Party/stm_modbus/src/stm_modbus_pdu.su ./MyMiddlewares/Third_Party/stm_modbus/src/tcp_modserver.cyclo ./MyMiddlewares/Third_Party/stm_modbus/src/tcp_modserver.d ./MyMiddlewares/Third_Party/stm_modbus/src/tcp_modserver.o ./MyMiddlewares/Third_Party/stm_modbus/src/tcp_modserver.su

.PHONY: clean-MyMiddlewares-2f-Third_Party-2f-stm_modbus-2f-src

