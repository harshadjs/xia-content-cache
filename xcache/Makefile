CC=gcc
XCACHEDIR=$(PWD)
CLICKDIR=/home/harshad/CMU/Fall-1/Independent_study/xia/xia-core/click-2.0.1
CFLAGS=-g -I$(CLICKDIR)/include/click -I$(XCACHEDIR)/include -Wall -Werror
export ## Make variables available to subsequent makefiles

TARGETS=core plugins

.PHONY: $(TARGETS) bin

all: bin
$(TARGETS):
	make -C $@
	mv $@/*.o bin/

bin: $(TARGETS)
	make -C $@

clean:
	make -C core clean
	make -C plugins clean
	make -C bin clean
