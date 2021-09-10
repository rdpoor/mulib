################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS :=  \
../Config/ \
../doxygen/ \
../doxygen/generator/ \
../examples/ \
../examples/include/ \
../examples/src/ \
../include/ \
../modules/ \
../modules/clk/ \
../modules/clk/doc/ \
../modules/pmic/ \
../modules/pmic/doc/ \
../modules/rtc/ \
../modules/rtc/doc/ \
../modules/sleep/ \
../modules/sleep/doc/ \
../modules/usart/ \
../modules/usart/doc/ \
../mu_platform \
../platform_test_eg \
../src/ \
../utils/ \
../utils/assembler/


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../../../../../../sources/platform_test_eg/src/platform_test.c \
../../../mu_platform/src/mu_button_io.c \
../../../mu_platform/src/mu_kbd_io.c \
../../../mu_platform/src/mu_led_io.c \
../../../mu_platform/src/mu_platform.c \
../../../mu_platform/src/mu_rtc.c \
../../../mu_platform/src/mu_time.c \
../atmel_start.c \
../driver_isr.c \
../examples/src/usart_basic_example.c \
../main.c \
../src/clk.c \
../src/driver_init.c \
../src/osc.c \
../src/pmic.c \
../src/rtc.c \
../src/sleep.c \
../src/usart_basic.c


PREPROCESSING_SRCS +=  \
../src/protected_io.S


ASM_SRCS += 


OBJS +=  \
platform_test_eg/platform_test.o \
mu_platform/mu_button_io.o \
mu_platform/mu_kbd_io.o \
mu_platform/mu_led_io.o \
mu_platform/mu_platform.o \
mu_platform/mu_rtc.o \
mu_platform/mu_time.o \
atmel_start.o \
driver_isr.o \
examples/src/usart_basic_example.o \
main.o \
src/clk.o \
src/driver_init.o \
src/osc.o \
src/pmic.o \
src/protected_io.o \
src/rtc.o \
src/sleep.o \
src/usart_basic.o

OBJS_AS_ARGS +=  \
platform_test_eg/platform_test.o \
mu_platform/mu_button_io.o \
mu_platform/mu_kbd_io.o \
mu_platform/mu_led_io.o \
mu_platform/mu_platform.o \
mu_platform/mu_rtc.o \
mu_platform/mu_time.o \
atmel_start.o \
driver_isr.o \
examples/src/usart_basic_example.o \
main.o \
src/clk.o \
src/driver_init.o \
src/osc.o \
src/pmic.o \
src/protected_io.o \
src/rtc.o \
src/sleep.o \
src/usart_basic.o

C_DEPS +=  \
platform_test_eg/platform_test.d \
mu_platform/mu_button_io.d \
mu_platform/mu_kbd_io.d \
mu_platform/mu_led_io.d \
mu_platform/mu_platform.d \
mu_platform/mu_rtc.d \
mu_platform/mu_time.d \
atmel_start.d \
driver_isr.d \
examples/src/usart_basic_example.d \
main.d \
src/clk.d \
src/driver_init.d \
src/osc.d \
src/pmic.d \
src/protected_io.d \
src/rtc.d \
src/sleep.d \
src/usart_basic.d

C_DEPS_AS_ARGS +=  \
platform_test_eg/platform_test.d \
mu_platform/mu_button_io.d \
mu_platform/mu_kbd_io.d \
mu_platform/mu_led_io.d \
mu_platform/mu_platform.d \
mu_platform/mu_rtc.d \
mu_platform/mu_time.d \
atmel_start.d \
driver_isr.d \
examples/src/usart_basic_example.d \
main.d \
src/clk.d \
src/driver_init.d \
src/osc.d \
src/pmic.d \
src/protected_io.d \
src/rtc.d \
src/sleep.d \
src/usart_basic.d

OUTPUT_FILE_PATH +=platform_test_app.elf

OUTPUT_FILE_PATH_AS_ARGS +=platform_test_app.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler
platform_test_eg/platform_test.o: ../../../../../../sources/platform_test_eg/src/platform_test.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

mu_platform/mu_button_io.o: ../../../mu_platform/src/mu_button_io.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

mu_platform/mu_kbd_io.o: ../../../mu_platform/src/mu_kbd_io.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

mu_platform/mu_led_io.o: ../../../mu_platform/src/mu_led_io.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

mu_platform/mu_platform.o: ../../../mu_platform/src/mu_platform.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

mu_platform/mu_rtc.o: ../../../mu_platform/src/mu_rtc.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

mu_platform/mu_time.o: ../../../mu_platform/src/mu_time.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

./atmel_start.o: .././atmel_start.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

./driver_isr.o: .././driver_isr.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

examples/src/usart_basic_example.o: ../examples/src/usart_basic_example.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

./main.o: .././main.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

src/clk.o: ../src/clk.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

src/driver_init.o: ../src/driver_init.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

src/osc.o: ../src/osc.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

src/pmic.o: ../src/pmic.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

src/rtc.o: ../src/rtc.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

src/sleep.o: ../src/sleep.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	

src/usart_basic.o: ../src/usart_basic.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -I"../examples/include" -I"../include" -I"../utils" -I"../utils/assembler" -I".." -I"../Config" -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include" -I"../../../../../../sources/platform_test_eg/incl" -I"../../../mu_platform/incl"  -Og -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	




# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler
src/protected_io.o: ../src/protected_io.S
	@echo Building file: $<
	@echo Invoking: AVR/GNU Assembler : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -Wa,-gdwarf2 -x assembler-with-cpp -c -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u" -I "../Config" -I "../examples/include" -I "../include" -I "../utils" -I "../utils/assembler" -I ".." -I "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\include"  -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -Wa,-g   -o "$@" "$<" 
	@echo Finished building: $<
	





ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : 5.4.0
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="platform_test_app.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mmcu=atxmega128a1u -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\Atmel\XMEGAA_DFP\1.1.68\gcc\dev\atxmega128a1u"  
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "platform_test_app.elf" "platform_test_app.hex"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "platform_test_app.elf" "platform_test_app.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "platform_test_app.elf" > "platform_test_app.lss"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "platform_test_app.elf" "platform_test_app.srec"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size.exe" "platform_test_app.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "platform_test_app.elf" "platform_test_app.a" "platform_test_app.hex" "platform_test_app.lss" "platform_test_app.eep" "platform_test_app.map" "platform_test_app.srec" "platform_test_app.usersignatures"
	