################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/Source/%.obj: ../FreeRTOS/Source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1250/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/Users/user/Desktop/RTOSProject/Source Code" --include_path="C:/Users/user/Desktop/RTOSProject/Source Code/MCAL/ADC" --include_path="C:/Users/user/Desktop/RTOSProject/Source Code/MCAL/GPIO" --include_path="C:/Users/user/Desktop/RTOSProject/Source Code/MCAL/GPTM" --include_path="C:/Users/user/Desktop/RTOSProject/Source Code/MCAL/UART" --include_path="C:/Users/user/Desktop/RTOSProject/Source Code/Common" --include_path="C:/Users/user/Desktop/RTOSProject/Source Code/HAL" --include_path="C:/Users/user/Desktop/RTOSProject/Source Code/MCAL" --include_path="C:/Users/user/Desktop/RTOSProject/Source Code/FreeRTOS/Source/include" --include_path="C:/Users/user/Desktop/RTOSProject/Source Code/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/ti/ccs1250/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs="ccs" --define=PART_TM4C123GH6PM -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="FreeRTOS/Source/$(basename $(<F)).d_raw" --obj_directory="FreeRTOS/Source" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


