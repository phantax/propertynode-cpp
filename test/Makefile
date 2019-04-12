.PHONY: all info clean

# Use gcc as default compiler and linker
# May be changed by passing arguments to make
ifeq ($(origin CC), default)
    CC=g++
endif
ifeq ($(origin LD), default)
    LD=g++
endif

ifeq ($(origin CC), undefined)
    CFLAGS  = -O0 -g
endif

SOURCES = $(shell ls *.cpp)

all: test

test: $(SOURCES) ../build/libpropertynode-cpp.a
	@echo "\033[01;32m=> Compiling '$<' ...\033[00;00m"
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) $(SOURCES) -lpropertynode-cpp $(LDFLAGS) -L../build -I../ -o $@
	@echo ""

info:
	@echo "Compiler is \"$(CC)\" defined by $(origin CC)"
	@echo "Linker is \"$(LD)\" defined by $(origin LD)"

clean:
	rm -f test




