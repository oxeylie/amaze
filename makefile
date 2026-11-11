 #------------------------------------------------------------------------#
 # makefile
 # 
 # Copyright (C) 2026 @oxeylie
 # SPDX-License-Identifier: AGPL-3.0-or-later
 #------------------------------------------------------------------------#

MAJOR_V     := 0
MINOR_V     := 1
APPENDIX    := dev
GPU_SUPPORT := true
DL_SUPPORT  := true
DEBUG       :=
HOST        := $(if $(shell echo | $(CC) -dM -E - | grep -w __APPLE__),darwin,linux)

CC  :=$(if $(and $(CC),$(filter $(origin CC),environment)),$(CC),gcc)
SC  :=glslang
MCC :=spirv-cross

ODIR    :=./build
OUTDIR  :=./out
BASEDIR :=./src
TESTDIR :=./tests
SHD_EXT :=$(if $(findstring darwin, $(HOST)),metal,spv)

EXTRA_FLAGS :=
FLAGS       :=-std=gnu23 -Wall -Wextra -Wpedantic -Wshadow $(EXTRA_FLAGS)
CC_FLAGS    :=$(FLAGS) -MMD -MP -DSHADER_EXT=$(SHD_EXT) \
							$(if $(GPU_SUPPORT),-DGPU_SUPPORT,) $(if $(DL_SUPPORT),-DDL_SUPPORT,) $(if $(DEBUG),-DDEBUG,) \
							-DMAJOR_V=\"$(MAJOR_V)\" -DMINOR_V=\"$(MINOR_V)\" $(if $(APPENDIX),-DAPPENDIX=\"$(APPENDIX)\",-UAPPENDIX)
LD_FLAGS    :=-lncurses $(if $DL_SUPPORT,-ldl,) $(FLAGS)
SC_FLAGS    :=
MCC_FLAGS   :=


# $(1) = name, $(2) = source, $(3) = build specific flags, $(4) = build specific include dirs, $(5) = extra objs, $(6) = extra shader objs, $(7) = build target
define BUILD_TEMPLATE
$(1)_SRCDIR  :=$(2)
$(1)_INCDIRS :=$$($(1)_SRCDIR)/include $(4)

$(1)_SRC    :=$$(shell find $$($(1)_SRCDIR) -type f -name '*.c')
$(1)_OBJSTR :=$(ODIR)/$1/%.o
$(1)_OBJ    :=$$(patsubst $$($(1)_SRCDIR)/%.c, $$($(1)_OBJSTR), $$($(1)_SRC))
$(1)_GLS    :=$$(shell find $$($(1)_SRCDIR) -type f -name '*.comp')
$(1)_SPVSTR :=$(ODIR)/$1/%.spv
$(1)_SPV    :=$$(patsubst $$($(1)_SRCDIR)/%.comp, $$($(1)_SPVSTR), $$($(1)_GLS))
$(1)_MSLSTR :=$(ODIR)/$1/%.metal
$(1)_MSL    :=$$(patsubst $$($(1)_SPVSTR), $$($(1)_MSLSTR), $$($(1)_SPV))
$(1)_SHD    :=$(if $(findstring darwin, $(HOST)),$$($(1)_MSL),$$($(1)_SPV))

$(1)_EMBDIRS :=$$(dir $$($(1)_SHD) $(6))

$(1)_CC_FLAGS  :=$(CC_FLAGS) $$(addprefix -I,$$(sort $$($(1)_INCDIRS))) $$(addprefix --embed-dir=, $$(sort $$($(1)_EMBDIRS)))$(3)
$(1)_LD_FLAGS  :=$(LD_FLAGS) $(3)
$(1)_SC_FLAGS  :=$(SC_FLAGS) $$(addprefix -I,$$(sort $$($(1)_INCDIRS)))
$(1)_MCC_FLAGS :=$(MCC_FLAGS)

$$($(1)_OBJSTR): $$($(1)_SRCDIR)/%.c $(if $(GPU_SUPPORT),$$($(1)_SHD) $(6),)
	@mkdir -p $$(dir $$@)
	$(CC) -o $$@ -c $$< $$($(1)_CC_FLAGS)

$$($(1)_SPVSTR): $$($(1)_SRCDIR)/%.comp
	@mkdir -p $$(dir $$@)
	$(SC) -o $$@ -V $$< $$($(1)_SC_FLAGS) 1>/dev/null

# Doesn't compile .metallib because it needs XCode metal toolchain
$$($(1)_MSLSTR): $$($(1)_SPVSTR)
	@mkdir -p $$(dir $$@)
	$(MCC) $$< --msl --output $$@ $$($(1)_MCC_FLAGS)

.PHONY: $(1)_obj_target $(1)_shader_target
$(1)_obj_target: $$($(1)_OBJ)
$(1)_shader_target: $$($(1)_SHD)

ifneq ($(7),) # Only make an end target if build_target is set
$(1)_TARGET := $(OUTDIR)/$(7)

$$($(1)_TARGET): $$($(1)_OBJ) $(5)
	@mkdir -p $$(dir $$($(1)_TARGET))
	$(CC) -o $$@ $$^ $$($(1)_LD_FLAGS)

.PHONY: $(1)
$(1): $$($(1)_TARGET) 
endif

-include $$($(1)_OBJ:.o=.d)
endef

.PHONY: all check clean tests
all: client server
tests: common_test client_test server_test
clean:
	rm -rf $(ODIR) $(OUTDIR)

# client & server template
$(eval $(call BUILD_TEMPLATE,common,$(BASEDIR)/common,,\
	,,,)) # common
$(eval $(call BUILD_TEMPLATE,client,$(BASEDIR)/client,,$(common_INCDIRS),\
	$(common_OBJ),$(common_SHD),amaze)) # client
$(eval $(call BUILD_TEMPLATE,server,$(BASEDIR)/server,,$(common_INCDIRS),\
	$(common_OBJ),$(common_SHD),amaze-server)) # server

# tests
$(eval $(call BUILD_TEMPLATE,common_test,$(TESTDIR)/common,,$(common_INCDIRS),\
	$(common_OBJ),$(common_SHD),common-test)) # test
$(eval $(call BUILD_TEMPLATE,client_test,$(TESTDIR)/client,,$(common_INCDIRS) $(client_INCDIRS),\
	$(common_OBJ) $(filter-out $(ODIR)/client/main.o,$(client_OBJ)),$(common_SHD) $(client_SHD),client-test)) # test
$(eval $(call BUILD_TEMPLATE,server_test,$(TESTDIR)/server,,$(common_INCDIRS) $(server_INCDIRS),\
	$(common_OBJ) $(filter-out $(ODIR)/server/main.o,$(server_OBJ)),$(common_SHD) $(server_SHD),server-test)) # test
