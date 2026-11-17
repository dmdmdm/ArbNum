// An arbitrary precision library and interactive text program
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "util.h"
#include "integer.h"

int main() {
	const Integer a("90000000000");
	const Integer b("80000000000");
	const Integer c = a * b;
	printf("%s + %s = %s\n", a.toString().c_str(), b.toString().c_str(), c.toString().c_str());
}
