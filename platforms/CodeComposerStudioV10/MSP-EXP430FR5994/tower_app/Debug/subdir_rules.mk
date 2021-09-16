################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/Applications/ti/ccs1031/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/bin/cl430" -vmspx --data_model=restricted -O4 --use_hw_mpy=F5 --include_path="/Applications/ti/ccs1031/ccs/ccs_base/msp430/include" --include_path="/Users/r/Projects/mulib-support/examples/CodeComposerStudioV10/MSP-EXP430FR5994/mu_platform" --include_path="/Users/r/Projects/mulib-support/examples/shared/tower_eg" --include_path="/Users/r/Projects/mulib-support/mulib" --include_path="/Users/r/Projects/mulib-support/examples/CodeComposerStudioV10/MSP-EXP430FR5994/tower_app" --include_path="/Applications/ti/ccs1031/ccs/tools/compiler/ti-cgt-msp430_20.2.5.LTS/include" --advice:power="all" --advice:hw_config="all" --define=__MSP430FR5994__ --define=_MPU_ENABLE -g --c11 --printf_support=nofloat --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


