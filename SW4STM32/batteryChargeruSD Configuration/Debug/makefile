################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Middlewares/FatFs/src/drivers/subdir.mk
-include Middlewares/FatFs/src/subdir.mk
-include Middlewares/FatFs/subdir.mk
-include Drivers/STM32F3xx_HAL_Driver/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include Drivers/BSP/STM32303E_EVAL/subdir.mk
-include Application/User/subdir.mk
-include Application/SW4STM32/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: batteryChargeruSD\ Configuration.elf

# Tool invocations
batteryChargeruSD\ Configuration.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU GCC Linker'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -specs=nosys.specs -specs=nano.specs -T"../STM32F303VETx_FLASH.ld" -Wl,-Map=output.map -Wl,--gc-sections -lm -o "batteryChargeruSD Configuration.elf" @"objects.list" $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

post-build:
	-@echo 'Generating binary and Printing size information:'
	arm-none-eabi-objcopy -O binary "batteryChargeruSD Configuration.elf" "batteryChargeruSD Configuration.bin"
	arm-none-eabi-size "batteryChargeruSD Configuration.elf"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
