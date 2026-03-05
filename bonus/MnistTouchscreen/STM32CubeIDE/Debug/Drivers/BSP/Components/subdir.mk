################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/Components/cs42l51/cs42l51.c \
C:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/Components/ft3x67/ft3x67.c \
C:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/Components/mfxstm32l152/mfxstm32l152.c \
C:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/Components/ov9655/ov9655.c 

OBJS += \
./Drivers/BSP/Components/cs42l51.o \
./Drivers/BSP/Components/ft3x67.o \
./Drivers/BSP/Components/mfxstm32l152.o \
./Drivers/BSP/Components/ov9655.o 

C_DEPS += \
./Drivers/BSP/Components/cs42l51.d \
./Drivers/BSP/Components/ft3x67.d \
./Drivers/BSP/Components/mfxstm32l152.d \
./Drivers/BSP/Components/ov9655.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/Components/cs42l51.o: C:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/Components/cs42l51/cs42l51.c Drivers/BSP/Components/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32L4R9xx -DUSE_HAL_DRIVER -DLCD_DIMMING -DLCD_OFF_AFTER_DIMMING -c -I../../Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Device/ST/STM32L4xx/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/STM32L4xx_HAL_Driver/Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/STM32L4R9I-Discovery -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Utilities/Log -I../../X-CUBE-AI/App -I../../Middlewares/ST/AI/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/Components/ft3x67.o: C:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/Components/ft3x67/ft3x67.c Drivers/BSP/Components/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32L4R9xx -DUSE_HAL_DRIVER -DLCD_DIMMING -DLCD_OFF_AFTER_DIMMING -c -I../../Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Device/ST/STM32L4xx/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/STM32L4xx_HAL_Driver/Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/STM32L4R9I-Discovery -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Utilities/Log -I../../X-CUBE-AI/App -I../../Middlewares/ST/AI/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/Components/mfxstm32l152.o: C:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/Components/mfxstm32l152/mfxstm32l152.c Drivers/BSP/Components/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32L4R9xx -DUSE_HAL_DRIVER -DLCD_DIMMING -DLCD_OFF_AFTER_DIMMING -c -I../../Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Device/ST/STM32L4xx/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/STM32L4xx_HAL_Driver/Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/STM32L4R9I-Discovery -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Utilities/Log -I../../X-CUBE-AI/App -I../../Middlewares/ST/AI/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/Components/ov9655.o: C:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/Components/ov9655/ov9655.c Drivers/BSP/Components/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32L4R9xx -DUSE_HAL_DRIVER -DLCD_DIMMING -DLCD_OFF_AFTER_DIMMING -c -I../../Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Device/ST/STM32L4xx/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/STM32L4xx_HAL_Driver/Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/STM32L4R9I-Discovery -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Utilities/Log -I../../X-CUBE-AI/App -I../../Middlewares/ST/AI/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-Components

clean-Drivers-2f-BSP-2f-Components:
	-$(RM) ./Drivers/BSP/Components/cs42l51.cyclo ./Drivers/BSP/Components/cs42l51.d ./Drivers/BSP/Components/cs42l51.o ./Drivers/BSP/Components/cs42l51.su ./Drivers/BSP/Components/ft3x67.cyclo ./Drivers/BSP/Components/ft3x67.d ./Drivers/BSP/Components/ft3x67.o ./Drivers/BSP/Components/ft3x67.su ./Drivers/BSP/Components/mfxstm32l152.cyclo ./Drivers/BSP/Components/mfxstm32l152.d ./Drivers/BSP/Components/mfxstm32l152.o ./Drivers/BSP/Components/mfxstm32l152.su ./Drivers/BSP/Components/ov9655.cyclo ./Drivers/BSP/Components/ov9655.d ./Drivers/BSP/Components/ov9655.o ./Drivers/BSP/Components/ov9655.su

.PHONY: clean-Drivers-2f-BSP-2f-Components

