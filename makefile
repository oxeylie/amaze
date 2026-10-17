 #------------------------------------------------------------------------#
 # makefile
 # 
 # Copyright (C) 2026 @oxeylie
 # SPDX-License-Identifier: AGPL-3.0-or-later
 #------------------------------------------------------------------------#

CC        := $(if $(filter Darwin,$(shell uname -s)),clang,gcc)
ODIR      := ./build
OUTDIR    := ./out
BASEDIR   := ./src
TESTDIR   := $(BASEDIR)/tests

FLAGS     := -std=gnu23 -Wall -Wextra
CC_FLAGS  := $(FLAGS) -MMD -MP
LD_FLAGS  := $(FLAGS) 

# $(1) = name, $(2) = source, $(3) = build specific flags, $(4) = extra objs, $(5) = build target
define BUILD_TEMPLATE
$(1)_SRCDIR := $(2)
$(1)_INCDIR := $$($(1)_SRCDIR)/include
$(1)_CC_FLAGS := $(CC_FLAGS) -I$$($(1)_INCDIR) $(3)
$(1)_LD_FLAGS := $(LD_FLAGS)

$(1)_SRC    := $$(shell find $$($(1)_SRCDIR) -type f -name '*.c')
$(1)_OBJ    := $$(patsubst $$($(1)_SRCDIR)/%.c, $(ODIR)/$1/%.o, $$($(1)_SRC))

$(ODIR)/$1/%.o: $$($(1)_SRCDIR)/%.c
	@mkdir -p $$(dir $$@)
	$(CC) -c -o $$@ $$< $$($(1)_CC_FLAGS) 

ifneq ($(5),) # Only compile if build_target is set 
$(1)_TARGET := $(OUTDIR)/$(5)

$$($(1)_TARGET): $$($(1)_OBJ)  $(4)
	@mkdir -p $$(dir $$($(1)_TARGET))
	$(CC) -o $$@ $$^ $$($(1)_LD_FLAGS)

.PHONY: $(1)
$(1): $$($(1)_TARGET) 
endif

-include $$($(1)_OBJ:.o=.d)
endef

.PHONY: all check clean run-tests
all: server client
run-tests: common_test client_test server_test
	@echo "<--- Running Tests --->"
	@$(OUTDIR)/common-test
	@$(OUTDIR)/client-test
	@$(OUTDIR)/server-test

# client & server template
$(eval $(call BUILD_TEMPLATE,common,$(BASEDIR)/common,\
	,,)) # common
$(eval $(call BUILD_TEMPLATE,server,$(BASEDIR)/server,\
	-I$(common_INCDIR),$(common_OBJ),server)) # server
$(eval $(call BUILD_TEMPLATE,client,$(BASEDIR)/client,\
	-I$(common_INCDIR),$(common_OBJ),client)) # client

# tests
$(eval $(call BUILD_TEMPLATE,common_test,$(TESTDIR)/common,\
	-I$(common_INCDIR),$(common_OBJ),common-test)) # test
$(eval $(call BUILD_TEMPLATE,client_test,$(TESTDIR)/client,\
	-I$(common_INCDIR) -I$(client_INCDIR),$(common_OBJ) $(filter-out $(ODIR)/client/main.o,$(client_OBJ)),client-test)) # test
$(eval $(call BUILD_TEMPLATE,server_test,$(TESTDIR)/server,\
	-I$(common_INCDIR) -I$(server_INCDIR),$(common_OBJ) $(filter-out $(ODIR)/server/main.o,$(server_OBJ)),server-test)) # test

clean: 
	rm -rf $(ODIR) $(OUTDIR)

