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

CC := $(if $(and $(CC),$(filter $(origin CC),environment)),$(CC),gcc)
SC := slangc

ODIR    := ./build
OUTDIR  := ./out
BASEDIR := ./src
TESTDIR := ./tests
SHD_EXT := $(if $(findstring darwin, $(HOST)),metal,spv)
SHD_TYP := $(if $(findstring darwin, $(HOST)),metal,spirv)

EXTRA_FLAGS :=
FLAGS       := -std=gnu23 -Wall -Wextra -Wpedantic -Wshadow $(EXTRA_FLAGS)
CC_DEFINES  := $(if $(GPU_SUPPORT),GPU_SUPPORT,) $(if $(DL_SUPPORT),DL_SUPPORT,) $(if $(DEBUG),DEBUG,) SHADER_EXT=$(SHD_EXT)\
							 MAJOR_V=\"$(MAJOR_V)\" MINOR_V=\"$(MINOR_V)\" $(if $(APPENDIX),APPENDIX=\"$(APPENDIX)\",)
CC_FLAGS    := -MMD -MP $(addprefix -D, $(CC_DEFINES)) $(FLAGS)
LD_FLAGS    := -lncurses $(if $(DL_SUPPORT),-ldl,) \
							 $(if $(and $(GPU_SUPPORT),$(findstring darwin, $(HOST))),-framework Metal -framework Foundation -lobjc,) $(FLAGS)
SC_DEFINES  :=
SC_FLAGS    :=

# $(1) = name, $(2) = source, $(3) = build specific flags, $(4) = build specific include dirs, $(5) = extra objs, $(6) = extra shader objs, $(7) = build target
define BUILD_TEMPLATE
$(1)_SRCDIR  := $(2)
$(1)_INCDIRS := $$($(1)_SRCDIR)/include $(4)

$(1)_SRC    := $$(shell find $$($(1)_SRCDIR) -type f -name '*.c')
$(1)_OBJSTR := $(ODIR)/$1/%.o
$(1)_OBJ    := $$(patsubst $$($(1)_SRCDIR)/%.c, $$($(1)_OBJSTR), $$($(1)_SRC))
$(1)_SLANG  := $$(shell find $$($(1)_SRCDIR) -type f -name '*.slang')
$(1)_SHDSTR := $(ODIR)/$1/%.$(SHD_EXT)
$(1)_SHD    := $$(patsubst $$($(1)_SRCDIR)/%.slang, $$($(1)_SHDSTR), $$($(1)_SLANG))

$(1)_EMBDIRS := $$(dir $$($(1)_SHD) $(6))

$(1)_CC_FLAGS := $$(addprefix -I,$$(sort $$($(1)_INCDIRS))) $$(addprefix --embed-dir=, $$(sort $$($(1)_EMBDIRS))) $(CC_FLAGS) $(3)
$(1)_LD_FLAGS := $(LD_FLAGS) $(3)
$(1)_SC_FLAGS := $$(addprefix -I,$$(sort $$($(1)_INCDIRS))) $(SC_FLAGS) 

$$($(1)_OBJSTR): $$($(1)_SRCDIR)/%.c $(if $(GPU_SUPPORT),$$($(1)_SHD) $(6),)
	@mkdir -p $$(dir $$@)
	$(CC) -o $$@ -c $$< $$($(1)_CC_FLAGS)

$$($(1)_SHDSTR): $$($(1)_SRCDIR)/%.slang
	@mkdir -p $$(dir $$@)
	@rm -f $$@
	$(SC) -o $$@ -target $(SHD_TYP) $$< $$($(1)_SC_FLAGS)

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

COMMA       := ,
SPACE       := $(empty) $(empty)
JSON_PATHS  := $(subst $(SPACE),$(COMMA),$(patsubst %,"%",$(client_INCDIRS) $(common_INCDIRS) $(server_INCDIRS)))
JSON_MACROS := $(subst $(SPACE),$(COMMA),$(patsubst %,"%",$(SC_DEFINES)))

slangdconfig.json: Makefile
	@printf '{  "slang.additionalSearchPaths": [%s], "slang.predefinedMacros": [%s]  }' \
		'$(JSON_PATHS)' '$(JSON_MACROS)' > $@
