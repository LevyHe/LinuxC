################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Mylog/mylog.c \
../Mylog/mylog_base.c 

OBJS += \
./Mylog/mylog.o \
./Mylog/mylog_base.o 

C_DEPS += \
./Mylog/mylog.d \
./Mylog/mylog_base.d 


# Each subdirectory must supply rules for building sources it contributes
Mylog/%.o: ../Mylog/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/levy/Project/CIP/EntherNet_IP/EntherNet" -I"/home/levy/Project/CIP/EntherNet_IP/Mylog" -I"/home/levy/Project/CIP/EntherNet_IP/Apps" -I"/home/levy/Project/CIP/EntherNet_IP" -I"/home/levy/Project/CIP/EntherNet_IP/Mysql" -O0 -g3 -Wall -c -fmessage-length=0 -std=c99  -D_GNU_SOURCE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


