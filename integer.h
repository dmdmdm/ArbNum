// A simple arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#pragma once

#include <string>
#include <vector>
#include "unsigned.h"

class IntegerDivide;

class Integer {
	typedef enum { SPEC_NORMAL, SPEC_ERROR, SPEC_IGNORE } SpecialType;
	SpecialType	mSpecial = SPEC_NORMAL;
	int 		mSign;	// -1 or 1
	Unsigned	mUnsigned;

public:
	int trimTrailing(const int);
	void clear();

	void mkNegative() { mSign = -1; }
	void mkPositive() { mSign = 1; }
	void flipSign() { mSign *= -1; }
	void setSign(const Integer &other) { mSign = other.mSign; }
	void appendRight(const int digit, const int numberOfDigits);

	bool isZero() const { return mUnsigned.isZero(); }
	bool isOne() const { return mUnsigned.isOne() && mSign > 0; }
	bool isPositiveOrZero() const { return mSign >= 0; }
	bool isNegativeOrZero() const { return mSign <= 0; }
	bool isPositive() const { return mSign > 0 && !isZero(); }
	bool isNegative() const { return mSign < 0 && !isZero(); }
	bool isOdd() const { return mUnsigned.isOdd(); }
	bool isEven() const { return mUnsigned.isEven(); }

	void saveNumber(const char *);

	void mkError() { clear(); mSpecial = SPEC_ERROR; }
	bool isError() const { return mSpecial == SPEC_ERROR; }

	void mkIgnore() { clear(); mSpecial = SPEC_IGNORE; }
	bool isIgnore() const { return mSpecial == SPEC_IGNORE; }

	bool isNormal() const { return mSpecial == SPEC_NORMAL; }

	// Constructors
	Integer() { clear(); }
	Integer(const Unsigned &);
	Integer(const Integer &);
	Integer(const char *);
	Integer(const int);
	Integer(const long);
	Integer(const long long);
	Integer(const bool);

	static Integer abs(const Integer &);
	static Integer sign(const Integer &);
	static int compare(const Integer &, const Integer &);
	static Integer add(const Integer &, const Integer &);
	static Integer subtract(const Integer &, const Integer &);
	static Integer multiply(const Integer &, const Integer &);
	static IntegerDivide divideWithRem(const Integer &dividend, const Integer &divisor);
	static Integer half(const Integer &);
	static Integer divide(const Integer &, const Integer &);
	static Integer mod(const Integer &, const Integer &);
	static Integer min(const Integer &, const Integer &);
	static Integer max(const Integer &, const Integer &);
	static Integer doNot(const Integer &);
	static Integer pow(const Integer &, const Integer &);
	static Integer gcd(const Integer &, const Integer &);
	static Integer lcm(const Integer &, const Integer &);
	static Integer factorial(const Integer &);
	static Integer sqrt(const Integer &);
	static Integer random(const Integer &);
	static Integer isPrime(const Integer &);

	void set(const Integer &);
	void add(const Integer &);
	void subtract(const Integer &);
	void multiply(const Integer &);
	void divide(const Integer &);
	void mod(const Integer &);
	void pow(const Integer &);

	std::string toString() const;
	char firstDigitAsChar() const;
	int firstDigitAsInt() const;
	int toInt() const; // Loses precision
	long toLong() const; // Loses precision
	long long toLongLong() const; // Loses precision
	double toDouble() const; // Loses precision
	Unsigned toUnsigned() const;

	Integer operator+=(const Integer &other) { add(other); return *this; }
	Integer operator-=(const Integer &other) { subtract(other); return *this; }
	Integer operator*=(const Integer &other) { multiply(other); return *this; }
	Integer operator/=(const Integer &other) { divide(other); return *this; }
	Integer operator%=(const Integer &other) { mod(other); return *this; }
	Integer operator^=(const Integer &other) { pow(other); return *this; }
	Integer operator++(int);
	Integer operator--(int);

	static bool testStoreStr(const char *, const int);
	static bool testStoreInt(const int);
	static bool testStoreLong(const long);
	static bool testStoreLongLong(const long long);
	static int compareLong(const long, const long);
	static bool testCompare(const long, const long);
	static bool testAdd(const long, const long);
	static bool testSubtract(const long, const long);
	static bool testMultiply(const long, const long);
	static bool testDivide(const long, const long);
	static bool testMod(const long, const long);
	static bool testPow(const long, const long);
	static long gcdLong(const long, const long);
	static bool testGcd(const long, const long);
	static long lcmLong(const long, const long);
	static bool testLcm(const long, const long);
	static long sqrtLong(const long);
	static bool testSqrt(const long);
	static long isPrimeLong(const long);
	static bool testIsPrime(const long);
	static long factorialLong(const long);
	static bool testFactorial(const long);
	static void makeTestValues(const long start, const long end, const long inc, std::vector<long> &);
	static bool testAll();
};

class IntegerDivide {
public:
	Integer quotient, remainder;

	IntegerDivide() { quotient = 0; remainder = 0; }
	IntegerDivide(const Integer &q, const Integer &r) { quotient = q; remainder = r; }

	std::string toString() const {
		return "{quotient=" + quotient.toString() + ", remainder=" + remainder.toString() + "}";
	}
};

inline bool operator<(const Integer &a, const Integer &b) { return Integer::compare(a, b) < 0; }
inline bool operator>(const Integer &a, const Integer &b) { return Integer::compare(a, b) > 0; }
inline bool operator==(const Integer &a, const Integer &b) { return Integer::compare(a, b) == 0; }
inline bool operator<=(const Integer &a, const Integer &b) { return Integer::compare(a, b) <= 0; }
inline bool operator>=(const Integer &a, const Integer &b) { return Integer::compare(a, b) >= 0; }
inline Integer operator+(const Integer &a, const Integer &b) { return Integer::add(a, b); }
inline Integer operator-(const Integer &a, const Integer &b) { return Integer::subtract(a, b); }
inline Integer operator*(const Integer &a, const Integer &b) { return Integer::multiply(a, b); }
inline Integer operator/(const Integer &a, const Integer &b) { return Integer::divide(a, b); }
inline Integer operator%(const Integer &a, const Integer &b) { return Integer::mod(a, b); }
inline Integer operator^(const Integer &a, const Integer &b) { return Integer::pow(a, b); }
inline Integer operator!(const Integer &a) { return Integer::doNot(a); }
