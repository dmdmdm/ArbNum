// A simple arbitrary precision library and interactive text program
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include <stdio.h>
#include "arbnum.h"

void example_use_of_arbnum_main() {
	ArbNum a("90000000000");
	ArbNum b("80000000000");
	ArbNum c = a * b;
	printf("result=%s\n", c.toString().c_str());
}