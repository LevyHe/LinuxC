################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cjson/cJSON.c 

OBJS += \
./cjson/cJSON.o 

C_DEPS += \
./cjson/cJSON.d 


# Each subdirectory must supply rules for building sources it contributes
cjson/%.o: ../cjson/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/levy/Project/bms_ccc/bms_ccc" -I"/home/levy/Project/bms_ccc/bms_ccc/app" -I"/home/levy/Project/bms_ccc/bms_ccc/hmi" -I"/home/levy/Project/bms_ccc/bms_ccc/cjson" -I"/home/levy/Project/bms_ccc/bms_ccc/mylib" -I"/home/levy/Project/bms_ccc/bms_ccc/mylog" -I"/home/levy/Project/bms_ccc/bms_ccc/sqlite" -O0 -g3 -Wall -c -fmessage-length=0 -std=c99  -D_GNU_SOURCE -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


