# Required packages: readline-devel glibc-devel gcc gcc-c++ make

ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
	DETECTED_OS = Windows
else
	DETECTED_OS = $(shell uname)  # same as "uname -s"
endif

LDFLAGS = -lstdc++ -lm
ifne ($(DETECTED_OS),Windows) 
	LDFLAGS += -lreadline
endif

CC = gcc
LDFLAGS = -lstdc++ -lm -lreadline
CFLAGS = -g -I. -fno-rtti -fno-exceptions -Wall
CPPFLAGS = $(CFLAGS)
SOURCES = bc.cpp bc_tokenizer.cpp bc_tokenizer.h bc_calc.cpp bc_calc.h arbnum.cpp arbnum.h

all: bc

log_os:
	echo os=$(DETECTED_OS)

clean:
	rm -f *.o bc

format:
	clang-format -style '{BasedOnStyle: Google, DerivePointerBinding: false, Standard: Cpp11}' -i $(SOURCES)

install: all

bc: Makefile bc.o bc_tokenizer.o bc_calc.o arbnum.o
	$(CC) $(CFLAGS) -o $@ bc.o bc_tokenizer.o bc_calc.o arbnum.o $(LDFLAGS)
	
check:
	./bc -t
