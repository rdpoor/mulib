################################################################################
# Automatically-generated file. Do not edit!
################################################################################

CG_TOOL_ROOT := /Applications/ti/ccs1031/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./main.obj" \
"./mu_platform/mu_platform.obj" \
"./mu_platform/mu_stddemo.obj" \
"./mu_platform/mu_time.obj" \
"./mulib/core/mu_bvec.obj" \
"./mulib/core/mu_cirq.obj" \
"./mulib/core/mu_dlist.obj" \
"./mulib/core/mu_fsm.obj" \
"./mulib/core/mu_list.obj" \
"./mulib/core/mu_log.obj" \
"./mulib/core/mu_pstore.obj" \
"./mulib/core/mu_queue.obj" \
"./mulib/core/mu_sched.obj" \
"./mulib/core/mu_spsc.obj" \
"./mulib/core/mu_str.obj" \
"./mulib/core/mu_strbuf.obj" \
"./mulib/core/mu_task.obj" \
"./mulib/core/mu_thunk.obj" \
"./mulib/core/mu_timer.obj" \
"./mulib/core/mu_vect.obj" \
"./mulib/core/mu_version.obj" \
"./mulib/extras/mu_random.obj" \
"./mulib/extras/mu_rfc_1123.obj" \
"./mulib/mulib.obj" \
"./tower_eg/animator.obj" \
"./tower_eg/ansi_term.obj" \
"./tower_eg/disk.obj" \
"./tower_eg/fb.obj" \
"./tower_eg/pole.obj" \
"./tower_eg/tower.obj" \
"../lnk_msp430fr5994.cmd" \
$(GEN_CMDS__FLAG) \
-llibmpu_init.a \
-llibmath.a \
-llibc.a \

-include ../makefile.init

RM := rm -rf
RMDIR := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include mu_platform/subdir_vars.mk
-include mulib/core/subdir_vars.mk
-include mulib/extras/subdir_vars.mk
-include mulib/subdir_vars.mk
-include tower_eg/subdir_vars.mk
-include subdir_rules.mk
-include mu_platform/subdir_rules.mk
-include mulib/core/subdir_rules.mk
-include mulib/extras/subdir_rules.mk
-include mulib/subdir_rules.mk
-include tower_eg/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
tower_app.out \

EXE_OUTPUTS__QUOTED += \
"tower_app.out" \

BIN_OUTPUTS += \
tower_app.hex \

BIN_OUTPUTS__QUOTED += \
"tower_app.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "tower_app.out"

# Tool invocations
tower_app.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: MSP430 Linker'
	"/Applications/ti/ccs1031/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/bin/cl430" -vmspx --data_model=restricted -O4 --use_hw_mpy=F5 --advice:power="all" --advice:hw_config="all" --define=__MSP430FR5994__ --define=_MPU_ENABLE -g --c11 --printf_support=nofloat --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 -z -m"tower_app.map" --heap_size=300 --stack_size=160 --cinit_hold_wdt=on -i"/Applications/ti/ccs1031/ccs/ccs_base/msp430/include" -i"/Applications/ti/ccs1031/ccs/ccs_base/msp430/lib/5xx_6xx_FRxx" -i"/Applications/ti/ccs1031/ccs/ccs_base/msp430/lib/FR59xx" -i"/Applications/ti/ccs1031/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/lib" -i"/Applications/ti/ccs1031/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/include" --priority --reread_libs --define=_MPU_ENABLE --diag_wrap=off --display_error_number --warn_sections --xml_link_info="tower_app_linkInfo.xml" --use_hw_mpy=F5 --rom_model --unused_section_elimination=on -o "tower_app.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

tower_app.hex: $(EXE_OUTPUTS)
	@echo 'Building secondary target: "$@"'
	@echo 'Invoking: MSP430 Hex Utility'
	"/Applications/ti/ccs1031/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/bin/hex430" --memwidth=8 --romwidth=8 --diag_wrap=off -o "tower_app.hex" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building secondary target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "main.obj" "mu_platform/mu_platform.obj" "mu_platform/mu_stddemo.obj" "mu_platform/mu_time.obj" "mulib/core/mu_bvec.obj" "mulib/core/mu_cirq.obj" "mulib/core/mu_dlist.obj" "mulib/core/mu_fsm.obj" "mulib/core/mu_list.obj" "mulib/core/mu_log.obj" "mulib/core/mu_pstore.obj" "mulib/core/mu_queue.obj" "mulib/core/mu_sched.obj" "mulib/core/mu_spsc.obj" "mulib/core/mu_str.obj" "mulib/core/mu_strbuf.obj" "mulib/core/mu_task.obj" "mulib/core/mu_thunk.obj" "mulib/core/mu_timer.obj" "mulib/core/mu_vect.obj" "mulib/core/mu_version.obj" "mulib/extras/mu_random.obj" "mulib/extras/mu_rfc_1123.obj" "mulib/mulib.obj" "tower_eg/animator.obj" "tower_eg/ansi_term.obj" "tower_eg/disk.obj" "tower_eg/fb.obj" "tower_eg/pole.obj" "tower_eg/tower.obj" 
	-$(RM) "main.d" "mu_platform/mu_platform.d" "mu_platform/mu_stddemo.d" "mu_platform/mu_time.d" "mulib/core/mu_bvec.d" "mulib/core/mu_cirq.d" "mulib/core/mu_dlist.d" "mulib/core/mu_fsm.d" "mulib/core/mu_list.d" "mulib/core/mu_log.d" "mulib/core/mu_pstore.d" "mulib/core/mu_queue.d" "mulib/core/mu_sched.d" "mulib/core/mu_spsc.d" "mulib/core/mu_str.d" "mulib/core/mu_strbuf.d" "mulib/core/mu_task.d" "mulib/core/mu_thunk.d" "mulib/core/mu_timer.d" "mulib/core/mu_vect.d" "mulib/core/mu_version.d" "mulib/extras/mu_random.d" "mulib/extras/mu_rfc_1123.d" "mulib/mulib.d" "tower_eg/animator.d" "tower_eg/ansi_term.d" "tower_eg/disk.d" "tower_eg/fb.d" "tower_eg/pole.d" "tower_eg/tower.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

