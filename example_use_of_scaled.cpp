// An arbitrary precision library and interactive text program
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include <cmath>
#include <format>
#include "scaled.h"
#include "util.h"

int main() {
	const Scaled a = "60000000000000000000";
	const Scaled b = "70000000000000000000";
	const Scaled sum = a + b;
	printf("%s + %s = %s\n", a.toString().c_str(), b.toString().c_str(), sum.toString().c_str());

	const Scaled diff = b - a;
	printf("%s - %s = %s\n", b.toString().c_str(), a.toString().c_str(), diff.toString().c_str());

	const Scaled prod = a * b;
	printf("%s * %s = %s\n", a.toString().c_str(), b.toString().c_str(), prod.toString().c_str());

	const Scaled quotient = a / b;
	printf("%s / %s = %s\n", a.toString().c_str(), b.toString().c_str(), quotient.toString().c_str());

	const Scaled mod_a(19, 6);
	const Scaled mod_n = 2;
	const Scaled mod = mod_a % mod_n;
	printf("%s %% %s = %s\n", mod_a.toString().c_str(), mod_n.toString().c_str(), mod.toString().c_str());

	const Scaled pow_n = 2;
	const Scaled pow = a ^ pow_n;
	printf("%s ^ %s = %s\n", a.toString().c_str(), pow_n.toString().c_str(), pow.toString().c_str());

	const Scaled not1 = ! a;
	printf("! %s = %s\n", a.toString().c_str(), not1.toString().c_str());
}
