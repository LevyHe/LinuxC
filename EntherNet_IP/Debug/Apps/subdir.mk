################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Apps/.main2.c \
../Apps/.main3.c \
../Apps/.test.c \
../Apps/main.c \
../Apps/pbbms.c 

OBJS += \
./Apps/.main2.o \
./Apps/.main3.o \
./Apps/.test.o \
./Apps/main.o \
./Apps/pbbms.o 

C_DEPS += \
./Apps/.main2.d \
./Apps/.main3.d \
./Apps/.test.d \
./Apps/main.d \
./Apps/pbbms.d 


# Each subdirectory must supply rules for building sources it contributes
Apps/%.o: ../Apps/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/levy/Project/CIP/EntherNet_IP/EntherNet" -I"/home/levy/Project/CIP/EntherNet_IP/Mylog" -I"/home/levy/Project/CIP/EntherNet_IP/Apps" -I"/home/levy/Project/CIP/EntherNet_IP" -I"/home/levy/Project/CIP/EntherNet_IP/Mysql" -O0 -g3 -Wall -c -fmessage-length=0 -std=c99  -D_GNU_SOURCE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


