// A simple arbitrary precision library and interactive text program
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "rational.h"

int example_use_of_ration_main() {
	Rational pi("pi");
	Rational pi2 = pi + 1;
	printf("pi=%s  pi+1=%s  pi+1=%.6f\n", pi.toString().c_str(), pi2.toString().c_str(), pi2.toDouble());

	const Rational a("2:4");	// We use colon for the fration slash
	const Rational b("2:3");

	const Rational sum = a + b;
	printf("%s + %s = %s\n", a.toString().c_str(), b.toString().c_str(), sum.toString().c_str());

	const Rational diff = b - a;
	printf("%s - %s = %s\n", b.toString().c_str(), a.toString().c_str(), diff.toString().c_str());

	const Rational prod = a * b;
	printf("%s * %s = %s\n", a.toString().c_str(), b.toString().c_str(), prod.toString().c_str());

	const Rational quotient = a / b;
	printf("%s / %s = %s\n", a.toString().c_str(), b.toString().c_str(), quotient.toString().c_str());

	const Rational mod_a(19,6);
	const Rational mod_n = 2;
	const Rational mod = mod_a % mod_n;
	printf("%s %% %s = %s\n", mod_a.toString().c_str(), mod_n.toString().c_str(), mod.toString().c_str());

	const Rational pow_n = 2;
	const Rational pow = a ^ pow_n;
	printf("%s ^ %s = %s\n", a.toString().c_str(), pow_n.toString().c_str(), pow.toString().c_str());

	const Rational not1 = ! a;
	printf("! %s = %s\n", a.toString().c_str(), not1.toString().c_str());

	printf("Tests...\n");
	Rational::testAll();
}
