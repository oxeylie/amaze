##
# Amaze MakeFile
##

CC        := $(if $(filter Darwin,$(shell uname -s)),clang,gcc)
FLAGS     := -std=gnu99 -Wall
ODIR      := ./build
OUTDIR    := ./out
BASEDIR   := ./src

common_SRCDIR := ./src/common
common_INCDIR := $(common_SRCDIR)/include

common_SRC := $(shell find $(common_SRCDIR) -type f -name '*.c')

# $(1) = name, $(2) = source, $(3) = flags 
define BUILD_TEMPLATE
$(1)_SRCDIR := $(2)
$(1)_INCDIR := $$($(1)_SRCDIR)/include
$(1)_FLAGS  := -I$$($(1)_INCDIR) -I$$(common_INCDIR) $(3)

$(1)_SRC    := $$(shell find $$($(1)_SRCDIR) -type f -name '*.c')
$(1)_OBJ    := $$(patsubst $$(BASEDIR)/%.c, $(ODIR)/$(1)/%.o, $$($(1)_SRC) $$(common_SRC))
$(1)_TARGET := $(OUTDIR)/$(1)

$$(ODIR)/$(1)/%.o: $$(BASEDIR)/%.c
	@mkdir -p $$(dir $$@)
	$(CC) -c -o $$@ $$< $$($(1)_FLAGS) $$(FLAGS)

$$($(1)_TARGET): $$($(1)_OBJ)
	@mkdir -p $$(dir $$($(1)_TARGET))
	$(CC) -o $$@ $$^ $$($(1)_FLAGS) $$(FLAGS)

.PHONY: $(1)
$(1): $$($(1)_TARGET) 
endef

.PHONY: all asan clean
all: server client

$(eval $(call BUILD_TEMPLATE,server,./src/server,))
$(eval $(call BUILD_TEMPLATE,client,./src/client,))

clean: 
	rm -rf $(ODIR) $(OUTDIR)
