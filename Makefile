# Required packages: readline-devel glibc-devel gcc gcc-c++ make

CC = gcc
LDFLAGS = -lstdc++ -lm -lreadline
CFLAGS = -g -I. -fno-rtti -fno-exceptions -Wall
CPPFLAGS = $(CFLAGS)
SOURCES = bc.cpp bc_tokenizer.cpp bc_tokenizer.h bc_calc.cpp bc_calc.h arbnum.cpp arbnum.h

all: bc

clean:
	rm -f *.o bc

format:
	clang-format -style '{BasedOnStyle: Google, DerivePointerBinding: false, Standard: Cpp11}' -i $(SOURCES)

install: all

bc: Makefile bc.o bc_tokenizer.o bc_calc.o arbnum.o
	$(CC) $(CFLAGS) -o $@ bc.o bc_tokenizer.o bc_calc.o arbnum.o $(LDFLAGS)
