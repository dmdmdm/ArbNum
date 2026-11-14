// A simple arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#pragma once

#include <string>
#include <vector>

class UnsignedDivide;

class Unsigned {
	// We use long rather than size_t in all function prototypes
	typedef std::vector<int> digits_t;
	digits_t mDigits;

	void zero();
	void pad(const long wantedLength);
	static void padShorterNumber(Unsigned &, Unsigned &);
	static void padShorterNumber(const Unsigned *&pA, const Unsigned *&pB, Unsigned &padded);

	static int charToInt(const char);
	static char intToChar(const int);

	void mkError();
	void prepend(const int digit) { mDigits.insert(mDigits.begin(), digit); }
	void append(const int digit) { mDigits.push_back(digit); }
	static Unsigned findDivisibleSizedChunk(const Unsigned &runningDividend, const Unsigned &dividend, const Unsigned &divisor, int &iDividendPos, bool &fits);
	static UnsignedDivide divideByOneWithRem(const Unsigned &);
	static UnsignedDivide divideByTwoWithRem(const Unsigned &);
	static UnsignedDivide divideWithRemSlow(const Unsigned &, const Unsigned &);
	static UnsignedDivide divideWithRemFast(const Unsigned &, const Unsigned &);

public:
	void trimLeading();
	long trimTrailing(const long);
	void clear();
	void append(const int digit, const long numberOfDigits);
	void shiftLeft(const long numberOfDigits) { append(0, numberOfDigits); }
	void shiftRight(const long numberOfDigits);
	long length() const { return (long)mDigits.size(); }
	void saveNumber(const char *);
	std::string toString() const;
	char firstDigitAsChar() const;
	int firstDigitAsInt() const;
	unsigned int toUnsignedInt() const; // Loses precision (will be always be positive)
	long toLong() const; // Loses precision
	long long toLongLong() const; // Loses precision
	void set(const Unsigned &);
	bool isZero() const;
	bool isSmall(const int) const;
	bool isOne() const { return isSmall(1); }
	bool isTwo() const { return isSmall(2); }
	bool isThree() const { return isSmall(3); }
	bool isFive() const { return isSmall(5); }
	bool isOdd() const;
	bool isEven() const;
	void multiplyByTen() { append(0); }

	static int compare(const Unsigned &, const Unsigned &);

	inline static void addMutable(Unsigned &, Unsigned &);
	static Unsigned add(const Unsigned &, const Unsigned &);

	inline static void subtractMutable(Unsigned &, Unsigned &);
	static Unsigned subtract(const Unsigned &, const Unsigned &);

	static Unsigned multiply(const Unsigned &, const Unsigned &);
	static UnsignedDivide divideWithRem(const Unsigned &, const Unsigned &);
	static Unsigned half(const Unsigned &);
	static Unsigned divide(const Unsigned &, const Unsigned &);
	static Unsigned mod(const Unsigned &, const Unsigned &);
	static Unsigned pow(const Unsigned &, const Unsigned &);
	static Unsigned gcd(const Unsigned &, const Unsigned &);
	static Unsigned lcm(const Unsigned &, const Unsigned &);
	static Unsigned factorial(const Unsigned &);
	static Unsigned sqrt(const Unsigned &);
	static Unsigned random(const Unsigned &);
	static Unsigned isPrime(const Unsigned &);

	void add(const Unsigned &);
	void subtract(const Unsigned &);
	void multiply(const Unsigned &);
	void divide(const Unsigned &);
	void mod(const Unsigned &);
	void pow(const Unsigned &);

	Unsigned() { }
	Unsigned(const Unsigned &);
	Unsigned(const char *);
	Unsigned(const unsigned int);
	Unsigned(const int);
	Unsigned(const long);
	Unsigned(const long long);
	Unsigned(const bool);

	int &operator[](const int i) { return mDigits[(size_t)i]; }
	Unsigned operator+=(const Unsigned &other) { add(other); return *this; }
	Unsigned operator-=(const Unsigned &other) { subtract(other); return *this; }
	Unsigned operator*=(const Unsigned &other) { multiply(other); return *this; }
	Unsigned operator/=(const Unsigned &other) { divide(other); return *this; }
	Unsigned operator%=(const Unsigned &other) { mod(other); return *this; }
	Unsigned operator^=(const Unsigned &other) { pow(other); return *this; }
	Unsigned operator++(int);
	Unsigned operator--(int);

	static bool testStoreStr(const char *, const unsigned int);
	static bool testStoreUnsigned(const unsigned int);
	static bool testStoreLong(const long);
	static bool testStoreLongLong(const long long);
	static int compareNativeUnsigned(const unsigned int, const unsigned int);
	static bool testCompare(const unsigned int, const unsigned int);
	static bool testAdd(const unsigned int, const unsigned int);
	static bool testSubtract(const unsigned int, const unsigned int);
	static bool testMultiply(const unsigned int, const unsigned int);
	static bool testDivide(const unsigned int, const unsigned int);
	static bool testMod(const unsigned int, const unsigned int);
	static bool testPow(const unsigned int, const unsigned int);
	static unsigned int gcdNativeUnsigned(const unsigned int, const unsigned int);
	static bool testGcd(const unsigned int, const unsigned int);
	static unsigned int lcmNativeUnsigned(const unsigned int, const unsigned int);
	static bool testLcm(const unsigned int, const unsigned int);
	static unsigned int sqrtNativeUnsigned(const unsigned int);
	static bool testSqrt(const unsigned int);
	static unsigned int isPrimeNativeUnsigned(const unsigned int);
	static bool testIsPrime(const unsigned int);
	static unsigned int factorialNativeUnsigned(const unsigned int);
	static bool testFactorial(const unsigned int);
	static void makeTestValues(const unsigned int start, const unsigned int end, const unsigned int inc, std::vector<unsigned int> &);
	static bool testAll();
};

class UnsignedDivide {
public:
	Unsigned quotient, remainder;

	UnsignedDivide() { quotient = 0; remainder = 0; }
	UnsignedDivide(const Unsigned &q, const Unsigned &r) { quotient = q; remainder = r; }

	void trimLeading() { quotient.trimLeading(); remainder.trimLeading(); }

	std::string toString() const {
		return "{quotient=" + quotient.toString() + ", remainder=" + remainder.toString() + "}";
	}
};

inline bool operator<(const Unsigned &a, const Unsigned &b) { return Unsigned::compare(a, b) < 0; }
inline bool operator>(const Unsigned &a, const Unsigned &b) { return Unsigned::compare(a, b) > 0; }
inline bool operator==(const Unsigned &a, const Unsigned &b) { return Unsigned::compare(a, b) == 0; }
inline bool operator<=(const Unsigned &a, const Unsigned &b) { return Unsigned::compare(a, b) <= 0; }
inline bool operator>=(const Unsigned &a, const Unsigned &b) { return Unsigned::compare(a, b) >= 0; }
inline Unsigned operator+(const Unsigned &a, const Unsigned &b) { return Unsigned::add(a, b); }
inline Unsigned operator-(const Unsigned &a, const Unsigned &b) { return Unsigned::subtract(a, b); }
inline Unsigned operator*(const Unsigned &a, const Unsigned &b) { return Unsigned::multiply(a, b); }
inline Unsigned operator/(const Unsigned &a, const Unsigned &b) { return Unsigned::divide(a, b); }
inline Unsigned operator%(const Unsigned &a, const Unsigned &b) { return Unsigned::mod(a, b); }
inline Unsigned operator^(const Unsigned &a, const Unsigned &b) { return Unsigned::pow(a, b); }
