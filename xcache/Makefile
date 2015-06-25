CC=g++
XCACHEDIR=$(PWD)
CLICKDIR=$(PWD)/../click-2.0.1/
CFLAGS=-g -I$(CLICKDIR)/include/click -I$(XCACHEDIR)/include -I$(XCACHEDIR)/bin -I$(XCACHEDIR) -I$(XCACHEDIR)/proto -Wall -Werror -fdiagnostics-color
LDFLAGS=
export ## Make variables available to subsequent makefiles

TARGETS=proto src

.PHONY: $(TARGETS) bin

all: bin
$(TARGETS):
	make -C $@
	mv $@/*.o bin/

bin: $(TARGETS)
	make -C $@

clean:
	make -C src clean
	make -C bin clean
	make -C proto clean
