################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Mysql/mysql_assist.c \
../Mysql/mysql_pool.c 

OBJS += \
./Mysql/mysql_assist.o \
./Mysql/mysql_pool.o 

C_DEPS += \
./Mysql/mysql_assist.d \
./Mysql/mysql_pool.d 


# Each subdirectory must supply rules for building sources it contributes
Mysql/%.o: ../Mysql/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/levy/Project/CIP/EntherNet_IP/EntherNet" -I"/home/levy/Project/CIP/EntherNet_IP/Mylog" -I"/home/levy/Project/CIP/EntherNet_IP/Apps" -I"/home/levy/Project/CIP/EntherNet_IP" -I"/home/levy/Project/CIP/EntherNet_IP/Mysql" -O0 -g3 -Wall -c -fmessage-length=0 -std=c99  -D_GNU_SOURCE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


