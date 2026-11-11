// A simple arbitrary precision library and interactive text program
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "util.h"
#include "integer.h"

int main() {
	Integer a("90000000000");
	Integer b("80000000000");
	Integer c = a * b;
	printf("result=%s\n", c.toString().c_str());
}
