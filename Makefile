# Required packages: readline-devel glibc-devel gcc gcc-c++ make

ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
	DETECTED_OS = Windows
else
	DETECTED_OS = $(shell uname)
endif

LDFLAGS = -lstdc++ -lm
ifeq ($(DETECTED_OS),Windows) 
else
	LDFLAGS += -lreadline
endif

CC = gcc
CFLAGS = -g -I. -fno-rtti -fno-exceptions -Wall
CPPFLAGS = $(CFLAGS)
HEADERS = bc_calc.h bc_tokenizer.h integer.h scaled.h unsigned.h util.h
SOURCES = bc_calc.cpp bc.cpp bc_tokenizer.cpp example_use_of_integer.cpp example_use_of_scaled.cpp integer.cpp scaled.cpp unsigned.cpp util.cpp

all: bc example_use_of_integer example_use_of_scaled

clean:
	rm -f *.o bc

# format:
#	clang-format -style '{BasedOnStyle: Google, DerivePointerBinding: false, Standard: Cpp11}' -i $(SOURCES)

install: all

bc.exe: Makefile bc.o bc_tokenizer.o bc_calc.o util.o unsigned.o integer.o scaled.o
	$(CC) $(CFLAGS) -o $@ bc.o bc_tokenizer.o bc_calc.o util.o unsigned.o integer.o scaled.o (LDFLAGS)

check_win: bc.exe
	echo Tests are disabled in Windows

bc: Makefile $(HEADERS) bc.o bc_tokenizer.o bc_calc.o util.o unsigned.o integer.o scaled.o
	$(CC) $(CFLAGS) -o $@ bc.o bc_tokenizer.o bc_calc.o util.o unsigned.o integer.o scaled.o $(LDFLAGS)

example_use_of_integer: Makefile $(HEADERS) util.o unsigned.o integer.o example_use_of_integer.o
	$(CC) $(CFLAGS) -o $@ util.o unsigned.o integer.o example_use_of_integer.o $(LDFLAGS)

example_use_of_scaled: Makefile $(HEADERS) util.o unsigned.o integer.o scaled.o example_use_of_scaled.o
	$(CC) $(CFLAGS) -o $@ util.o unsigned.o integer.o scaled.o example_use_of_scaled.o $(LDFLAGS)

check: bc
	# Use POSIX [ rather than [[
	@if [ "$(DETECTED_OS)" = "Windows" ]; then \
		.\\bc -t; \
	else \
		./bc -t; \
	fi	

check_to_file: bc
	./bc -t > tests.txt 2>&1
