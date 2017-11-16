################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../apps/main.c \
../apps/main2.c 

OBJS += \
./apps/main.o \
./apps/main2.o 

C_DEPS += \
./apps/main.d \
./apps/main2.d 


# Each subdirectory must supply rules for building sources it contributes
apps/%.o: ../apps/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/levy/Project/CIP/EntherNet_IP/EntherNet" -I"/home/levy/Project/CIP/EntherNet_IP" -I"/home/levy/Project/CIP/EntherNet_IP/Mysql" -O0 -g3 -Wall -c -fmessage-length=0 -std=c99  -D_GNU_SOURCE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


