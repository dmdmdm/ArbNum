// Operations with scaled numbers
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#pragma once

#include "integer.h"

#undef USE_INTEGER_FOR_EXP

class ScaledDivide;

class Scaled {
	typedef enum { SPEC_NORMAL, SPEC_ERROR, SPEC_IGNORE } SpecialType;
	SpecialType	mSpecial = SPEC_NORMAL;
	Integer mUnscaled; // The unscaled value
#ifdef USE_INTEGER_FOR_EXP
	Integer mExp; // Exponent (Value = mUnscaled * 10 ^ -mExp)
#else
	int mExp; // Exponent (Value = mUnscaled * 10 ^ -mExp)
#endif

	void clear();
	void appendRight(const int digit, const int numberOfDigits);

public:
	void saveNumber(const char *);
	void setSign(const Scaled &);
	void mkNegative() { mUnscaled.mkNegative(); }

	// Constructors
	Scaled() {};
	Scaled(const Integer &unscaled, const Integer &exp);
	Scaled(const Integer &);
	Scaled(const char *a) { saveNumber(a); }
	Scaled(const double);
	Scaled(const int);
	Scaled(const long);
	Scaled(const long long);

	void mkError() { clear(); mSpecial = SPEC_ERROR; }
	bool isError() const { return mSpecial == SPEC_ERROR; }

	void mkIgnore() { clear(); mSpecial = SPEC_IGNORE; }
	bool isIgnore() const { return mSpecial == SPEC_IGNORE; }

	bool isNormal() const { return mSpecial == SPEC_NORMAL; }

	// Methods
	bool isZero() const;
	bool isCloseToZero() const;
	bool isNegative() const;
	bool isPositive() const; // Greater than zero
	bool isInteger() const;
	Integer getDenom() const;
	bool isOdd() const;
	bool isEven() const;

	void simplify();

	std::string toString() const;
	double toDouble() const; // Loses precision
	Integer toInteger() const; // Loses decimal part
	Unsigned toUnsigned() const; // Loses sign and decimal part 
	int toInt() const; // Loses precision and decimal part

	static void convertToSameExponent(Scaled &, Scaled &b);
	static Scaled abs(const Scaled &);
	static Scaled sign(const Scaled &);
	static int compare(const Scaled &, const Scaled &);
	static Scaled add(const Scaled &, const Scaled &);
	static Scaled subtract(const Scaled &, const Scaled &);
	static Scaled multiply(const Scaled &, const Scaled &);
	static Scaled halfInt(const Scaled &);
	static Scaled half(const Scaled &);
	static Scaled divide(const Scaled &, const Scaled &);
	static Scaled mod(const Scaled &, const Scaled &);
	static Scaled min(const Scaled &, const Scaled &);
	static Scaled max(const Scaled &, const Scaled &);
	static Scaled doNot(const Scaled &);
	static Scaled floor(const Scaled &);
	static Scaled pow(const Scaled &, const Scaled &);
	static Scaled gcd(const Scaled &, const Scaled &);
	static Scaled lcm(const Scaled &, const Scaled &);
	static Scaled factorial(const Scaled &);
	static Scaled sqrt(const Scaled &);
	static Scaled random(const Scaled &);
	static Scaled isPrime(const Scaled &);

	void set(const Scaled &);
	void add(const Scaled &);
	void subtract(const Scaled &);
	void multiply(const Scaled &);
	void divide(const Scaled &);
	void mod(const Scaled &);
	void pow(const Scaled &);

	Scaled operator+=(const Scaled &other) { add(other); return *this; }
	Scaled operator-=(const Scaled &other) { subtract(other); return *this; }
	Scaled operator*=(const Scaled &other) { multiply(other); return *this; }
	Scaled operator/=(const Scaled &other) { divide(other); return *this; }
	Scaled operator%=(const Scaled &other) { mod(other); return *this; }
	Scaled operator^=(const Scaled &other) { pow(other); return *this; }
	Scaled operator++(int);
	Scaled operator--(int);

	static bool testStoreStr(const char *, const char *);
	static bool testStoreDouble(const double);
	static bool testStoreLong(const long);
	static bool testStoreLongLong(const long long);
	static int compareDouble(const double, const double);
	static bool testCompare(const double, const double);
	static bool testAdd(const double, const double);
	static bool testSubtract(const double, const double);
	static bool testMultiply(const double, const double);
	static bool testDivide(const double, const double);
	static bool testMod(const double, const double);
	static bool testPow(const double, const double);
	static double gcdDouble(const double, const double);
	static bool testGcd(const double, const double);
	static double lcmDouble(const double, const double);
	static bool testLcm(const double, const double);
	static double sqrtDouble(const double);
	static bool testSqrt(const double);
	static double isPrimeDouble(const double);
	static bool testIsPrime(const double);
	static double factorialDouble(const double);
	static bool testFactorial(const double);
	static void makeTestValues(const long start, const long end, const long inc, std::vector<double> &);
	static bool testAll();
};

class ScaledDivide {
public:
	Scaled quotient, remainder;

	ScaledDivide() { quotient = 0; remainder = 0; }

	ScaledDivide(const Scaled &q, const Scaled &r) { quotient = q; remainder = r; }

	ScaledDivide(const IntegerDivide &idiv) {
		quotient = idiv.quotient;
		remainder = idiv.remainder;
	}

	std::string toString() const {
		return "{quotient=" + quotient.toString() + ", remainder=" + remainder.toString() + "}";
	}
};

inline bool operator<(const Scaled &a, const Scaled &b) { return Scaled::compare(a, b) < 0; }
inline bool operator>(const Scaled &a, const Scaled &b) { return Scaled::compare(a, b) > 0; }
inline bool operator==(const Scaled &a, const Scaled &b) { return Scaled::compare(a, b) == 0; }
inline bool operator<=(const Scaled &a, const Scaled &b) { return Scaled::compare(a, b) <= 0; }
inline bool operator>=(const Scaled &a, const Scaled &b) { return Scaled::compare(a, b) >= 0; }
inline Scaled operator+(const Scaled &a, const Scaled &b) { return Scaled::add(a, b); }
inline Scaled operator-(const Scaled &a, const Scaled &b) { return Scaled::subtract(a, b); }
inline Scaled operator*(const Scaled &a, const Scaled &b) { return Scaled::multiply(a, b); }
inline Scaled operator/(const Scaled &a, const Scaled &b) { return Scaled::divide(a, b); }
inline Scaled operator%(const Scaled &a, const Scaled &b) { return Scaled::mod(a, b); }
inline Scaled operator^(const Scaled &a, const Scaled &b) { return Scaled::pow(a, b); }
inline Scaled operator!(const Scaled &a) { return Scaled::doNot(a); }
