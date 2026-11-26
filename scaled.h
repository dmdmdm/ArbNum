// Operations with scaled numbers
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#pragma once

#include "integer.h"

class ScaledDivide;

class Scaled {
	typedef enum { SPEC_NORMAL, SPEC_ERROR, SPEC_IGNORE } SpecialType;
	SpecialType	mSpecial = SPEC_NORMAL;
	Integer mUnscaled; // The unscaled value
	long mExp = 0; // Exponent (Value = mUnscaled * 10 ^ -mExp)
	// Using a long for the exponent means there is a limit on the size of our numbers.
	// The POSIX bc does the same thing.

	void clear();
	void shiftLeft(const long numberOfDigits) { mUnscaled.shiftLeft(numberOfDigits); }

public:
	void saveNumber(const char *);
	void copySign(const Scaled &);
	void mkNegative() { mUnscaled.mkNegative(); }
	int signInt() const { return mUnscaled.signInt(); }
	void flipSign() { mUnscaled.flipSign(); }
	void trimToPrecision();

	// Constructors
	Scaled() {};
	Scaled(const long &unscaled, const long &exp);
	Scaled(const Integer &);
	Scaled(const char *a) { saveNumber(a); }
	Scaled(const double);
	Scaled(const long double);
	Scaled(const int);
	Scaled(const long);
	Scaled(const long long);

	void mkError() { clear(); mSpecial = SPEC_ERROR; }
	bool isError() const { return mSpecial == SPEC_ERROR; }

	void mkIgnore() { clear(); mSpecial = SPEC_IGNORE; }
	bool isIgnore() const { return mSpecial == SPEC_IGNORE; }

	bool isNormal() const { return mSpecial == SPEC_NORMAL; }

	// Methods
	bool isZero() const; // Maybe you want isCloseToZero() ?
	bool isOne() const;
	bool isCloseToZero() const;
	bool isNegative() const;
	bool isNegativeOrZero() const;
	bool isPositive() const; // Greater than zero
	bool isInteger() const;
	bool isOdd() const;
	bool isEven() const;
	void multiplyByTen();
	void multiplyByTen(const long exp);

	void simplify();

	std::string toString() const;
	double toDouble() const; // Loses precision
	long double toLongDouble() const; // Loses precision
	Integer toInteger() const; // Loses decimal part
	inline Unsigned toUnsigned() const { return toInteger().toUnsigned(); } // Loses sign and decimal part 
	inline int toInt() const { return toInteger().toInt(); }  // Loses precision and decimal part
	inline long toLong() const { return toInteger().toLong(); }  // Loses precision and decimal part
	inline long long toLongLong() const { return toInteger().toLongLong(); } // Loses precision and decimal part
	std::string toDump() const;

	static Scaled getPi();
	static Scaled getE();
	static void setPrecisionGlobal(const long); // I prefer to call it this even though bc calls it 'scale'
	static long getPrecisionGlobal();
	static void convertToSameExponent(Scaled &, Scaled &);
	static void convertToZeroExponent(Scaled &, Scaled &); // Whole numbers
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
	static Scaled trunc(const Scaled &a) { return a.toInteger(); }
	static Scaled floor(const Scaled &);
	static Scaled ceil(const Scaled &);
	static Scaled round(const Scaled &);
	static Scaled gcd(const Scaled &, const Scaled &);
	static Scaled lcm(const Scaled &, const Scaled &);
	static Scaled factorial(const Scaled &);
	static Scaled sqrt(const Scaled &);
	static Scaled random(const Scaled &);
	static Scaled isPrime(const Scaled &);
	static Scaled sin(const Scaled &);
	static Scaled cos(const Scaled &);
	static Scaled tan(const Scaled &);
	inline static Scaled expTaylor(const Scaled &); // private
	static Scaled exp(const Scaled &); // e ^ x
	static Scaled powInt(const Scaled &, const Scaled &);
	static Scaled pow(const Scaled &, const Scaled &);
	static Scaled ln(const Scaled &); // Natural logarithm (base-e)
	static Scaled log(const Scaled &); // Base-10 logartihm

	void set(const Scaled &);
	void set(const long unscaled, const long exp);
	void add(const Scaled &);
	void subtract(const Scaled &);
	void multiply(const Scaled &);
	void divide(const Scaled &);
	void mod(const Scaled &);
	void pow(const Scaled &);

	// These don't return Scaled& for speed and that's rarely needed
	void operator+=(const Scaled &other) { add(other); }
	void operator-=(const Scaled &other) { subtract(other); }
	void operator*=(const Scaled &other) { multiply(other); }
	void operator/=(const Scaled &other) { divide(other);  }
	void operator%=(const Scaled &other) { mod(other); }
	void operator^=(const Scaled &other) { pow(other); }
	void operator++(int);
	void operator--(int);

	static void testConstants(int &nSuccess, int &nFail);
	static bool testStoreStr(const char *, const char *);
	static bool testStoreDouble(const double);
	static bool testStoreLongDouble(const long double);
	static bool testStoreLong(const long);
	static bool testStoreLongLong(const long long);
	static bool testFloor(const double);
	static bool testCeil(const double);
	static bool testRound(const double);
	static int compareDouble(const double, const double);
	static bool testCompare(const double, const double);
	static bool testAdd(const double, const double);
	static bool testSubtract(const double, const double);
	static bool testMultiply(const double, const double);
	static bool testDivide(const double, const double);
	static bool testMod(const double, const double);
	static bool testPiMath(const double mult, const double div);
	static double gcdDouble(const double, const double);
	static long double gcdLongDouble(const long double, const long double);
	static bool testGcd(const double, const double);
	static double lcmDouble(const double, const double);
	static long double lcmLongDouble(const long double, const long double);
	static bool testLcm(const long double, const long double);
	static bool testSqrt(const double);
	static double isPrimeDouble(const double);
	static bool testIsPrime(const double);
	static double factorialDouble(const double);
	static bool testFactorial(const double);
	static bool testSin(const double);
	static bool testCos(const double);
	static bool testTan(const double);
	static bool testExp(const double);
	static bool testPow(const long double, const long double);
	static bool testLn(const double);
	static bool testLog(const double);
	static void makeTestValues(const long start, const long end, const long inc, std::vector<double> &);
	inline static bool testAllInner(const std::string &testName, const long precision, int &nSuccess, int &nFail, const long startMs); // private
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
