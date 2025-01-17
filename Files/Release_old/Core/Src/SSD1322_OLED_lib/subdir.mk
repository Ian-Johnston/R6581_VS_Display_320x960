################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (11.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/SSD1322_OLED_lib/SSD1322_API.c \
../Core/Src/SSD1322_OLED_lib/SSD1322_GFX.c \
../Core/Src/SSD1322_OLED_lib/SSD1322_HW_Driver.c 

OBJS += \
./Core/Src/SSD1322_OLED_lib/SSD1322_API.o \
./Core/Src/SSD1322_OLED_lib/SSD1322_GFX.o \
./Core/Src/SSD1322_OLED_lib/SSD1322_HW_Driver.o 

C_DEPS += \
./Core/Src/SSD1322_OLED_lib/SSD1322_API.d \
./Core/Src/SSD1322_OLED_lib/SSD1322_GFX.d \
./Core/Src/SSD1322_OLED_lib/SSD1322_HW_Driver.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/SSD1322_OLED_lib/%.o Core/Src/SSD1322_OLED_lib/%.su Core/Src/SSD1322_OLED_lib/%.cyclo: ../Core/Src/SSD1322_OLED_lib/%.c Core/Src/SSD1322_OLED_lib/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -Ofast -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-SSD1322_OLED_lib

clean-Core-2f-Src-2f-SSD1322_OLED_lib:
	-$(RM) ./Core/Src/SSD1322_OLED_lib/SSD1322_API.cyclo ./Core/Src/SSD1322_OLED_lib/SSD1322_API.d ./Core/Src/SSD1322_OLED_lib/SSD1322_API.o ./Core/Src/SSD1322_OLED_lib/SSD1322_API.su ./Core/Src/SSD1322_OLED_lib/SSD1322_GFX.cyclo ./Core/Src/SSD1322_OLED_lib/SSD1322_GFX.d ./Core/Src/SSD1322_OLED_lib/SSD1322_GFX.o ./Core/Src/SSD1322_OLED_lib/SSD1322_GFX.su ./Core/Src/SSD1322_OLED_lib/SSD1322_HW_Driver.cyclo ./Core/Src/SSD1322_OLED_lib/SSD1322_HW_Driver.d ./Core/Src/SSD1322_OLED_lib/SSD1322_HW_Driver.o ./Core/Src/SSD1322_OLED_lib/SSD1322_HW_Driver.su

.PHONY: clean-Core-2f-Src-2f-SSD1322_OLED_lib

