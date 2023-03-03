# Required packages: readline-devel glibc-devel gcc gcc-c++ make

ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
	DETECTED_OS = Windows
else
	DETECTED_OS = $(shell uname)  # same as "uname -s"
endif

LDFLAGS = -lstdc++ -lm
ifeq ($(DETECTED_OS),Windows) 
else
	LDFLAGS += -lreadline
endif

CC = gcc
CFLAGS = -g -I. -fno-rtti -fno-exceptions -Wall
CPPFLAGS = $(CFLAGS)
SOURCES = bc.cpp bc_tokenizer.cpp bc_tokenizer.h bc_calc.cpp bc_calc.h arbnum.cpp arbnum.h

all: bc

clean:
	rm -f *.o bc

format:
	clang-format -style '{BasedOnStyle: Google, DerivePointerBinding: false, Standard: Cpp11}' -i $(SOURCES)

install: all

bc.exe: Makefile bc.o bc_tokenizer.o bc_calc.o arbnum.o
	$(CC) $(CFLAGS) -o $@ bc.o bc_tokenizer.o bc_calc.o arbnum.o $(LDFLAGS)
	
check_win: bc.exe
	@bc.exe -t
	
bc: Makefile bc.o bc_tokenizer.o bc_calc.o arbnum.o
	$(CC) $(CFLAGS) -o $@ bc.o bc_tokenizer.o bc_calc.o arbnum.o $(LDFLAGS)

check: bc
	./bc -t
