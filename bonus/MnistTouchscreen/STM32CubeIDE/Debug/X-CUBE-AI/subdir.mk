################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/matte/Documents/embedded_AI/Projet_Maintenance_Predictive/bonus/MnistTouchscreen/X-CUBE-AI/App/app_x-cube-ai.c \
C:/Users/matte/Documents/embedded_AI/Projet_Maintenance_Predictive/bonus/MnistTouchscreen/X-CUBE-AI/App/mnist.c \
C:/Users/matte/Documents/embedded_AI/Projet_Maintenance_Predictive/bonus/MnistTouchscreen/X-CUBE-AI/App/mnist_data.c \
C:/Users/matte/Documents/embedded_AI/Projet_Maintenance_Predictive/bonus/MnistTouchscreen/X-CUBE-AI/App/mnist_data_params.c 

OBJS += \
./X-CUBE-AI/app_x-cube-ai.o \
./X-CUBE-AI/mnist.o \
./X-CUBE-AI/mnist_data.o \
./X-CUBE-AI/mnist_data_params.o 

C_DEPS += \
./X-CUBE-AI/app_x-cube-ai.d \
./X-CUBE-AI/mnist.d \
./X-CUBE-AI/mnist_data.d \
./X-CUBE-AI/mnist_data_params.d 


# Each subdirectory must supply rules for building sources it contributes
X-CUBE-AI/app_x-cube-ai.o: C:/Users/matte/Documents/embedded_AI/Projet_Maintenance_Predictive/bonus/MnistTouchscreen/X-CUBE-AI/App/app_x-cube-ai.c X-CUBE-AI/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32L4R9xx -DUSE_HAL_DRIVER -DLCD_DIMMING -DLCD_OFF_AFTER_DIMMING -c -I../../Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Device/ST/STM32L4xx/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/STM32L4xx_HAL_Driver/Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/STM32L4R9I-Discovery -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Utilities/Log -I../../X-CUBE-AI/App -I../../Middlewares/ST/AI/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
X-CUBE-AI/mnist.o: C:/Users/matte/Documents/embedded_AI/Projet_Maintenance_Predictive/bonus/MnistTouchscreen/X-CUBE-AI/App/mnist.c X-CUBE-AI/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32L4R9xx -DUSE_HAL_DRIVER -DLCD_DIMMING -DLCD_OFF_AFTER_DIMMING -c -I../../Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Device/ST/STM32L4xx/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/STM32L4xx_HAL_Driver/Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/STM32L4R9I-Discovery -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Utilities/Log -I../../X-CUBE-AI/App -I../../Middlewares/ST/AI/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
X-CUBE-AI/mnist_data.o: C:/Users/matte/Documents/embedded_AI/Projet_Maintenance_Predictive/bonus/MnistTouchscreen/X-CUBE-AI/App/mnist_data.c X-CUBE-AI/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32L4R9xx -DUSE_HAL_DRIVER -DLCD_DIMMING -DLCD_OFF_AFTER_DIMMING -c -I../../Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Device/ST/STM32L4xx/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/STM32L4xx_HAL_Driver/Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/STM32L4R9I-Discovery -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Utilities/Log -I../../X-CUBE-AI/App -I../../Middlewares/ST/AI/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
X-CUBE-AI/mnist_data_params.o: C:/Users/matte/Documents/embedded_AI/Projet_Maintenance_Predictive/bonus/MnistTouchscreen/X-CUBE-AI/App/mnist_data_params.c X-CUBE-AI/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32L4R9xx -DUSE_HAL_DRIVER -DLCD_DIMMING -DLCD_OFF_AFTER_DIMMING -c -I../../Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/CMSIS/Device/ST/STM32L4xx/Include -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/STM32L4xx_HAL_Driver/Inc -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Drivers/BSP/STM32L4R9I-Discovery -IC:/Users/matte/STM32Cube/Repository/STM32Cube_FW_L4_V1.18.2/Utilities/Log -I../../X-CUBE-AI/App -I../../Middlewares/ST/AI/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-X-2d-CUBE-2d-AI

clean-X-2d-CUBE-2d-AI:
	-$(RM) ./X-CUBE-AI/app_x-cube-ai.cyclo ./X-CUBE-AI/app_x-cube-ai.d ./X-CUBE-AI/app_x-cube-ai.o ./X-CUBE-AI/app_x-cube-ai.su ./X-CUBE-AI/mnist.cyclo ./X-CUBE-AI/mnist.d ./X-CUBE-AI/mnist.o ./X-CUBE-AI/mnist.su ./X-CUBE-AI/mnist_data.cyclo ./X-CUBE-AI/mnist_data.d ./X-CUBE-AI/mnist_data.o ./X-CUBE-AI/mnist_data.su ./X-CUBE-AI/mnist_data_params.cyclo ./X-CUBE-AI/mnist_data_params.d ./X-CUBE-AI/mnist_data_params.o ./X-CUBE-AI/mnist_data_params.su

.PHONY: clean-X-2d-CUBE-2d-AI

