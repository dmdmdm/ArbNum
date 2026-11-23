// An arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include <limits.h>
#include <cmath>
#include <numeric>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> // For random seed
#include <algorithm>
#include "util.h"
#include "unsigned.h"

//------------------------------------------------------------------------------
// Unsigned

INLINE const Unsigned s_UnsignedZero(0);
INLINE const Unsigned s_UnsignedOne(1);
INLINE const Unsigned s_UnsignedTwo(2);
INLINE const Unsigned s_UnsignedThree(3);
INLINE const Unsigned s_UnsignedFive(5);
INLINE const Unsigned s_UnsignedSix(6);
INLINE const int UNASSIGNED = -1;

void Unsigned::pad(const long wantedLength) {
	mDigits.insert(mDigits.begin(), (size_t)wantedLength - mDigits.size(), 0);
}

void Unsigned::append(const int digit, const long numberOfDigits) {
	mDigits.insert(mDigits.end(), (size_t) numberOfDigits, digit);
}

void Unsigned::shiftRight(const long numberOfDigits) {
	if (numberOfDigits > (long)mDigits.size()) {
		ArbNum_log("shiftRight: asked to delete more digits than are present\n");
	}
	const long n = std::min(numberOfDigits, (long)mDigits.size());
	mDigits.erase(mDigits.end() - n, mDigits.end());
}

void Unsigned::padShorterNumber(Unsigned &a, Unsigned &b) {
	if (a.length() < b.length()) {
		a.pad(b.length());
	}
	else {
		b.pad(a.length());
	}
}

void Unsigned::padShorterNumber(const Unsigned *&pA, const Unsigned *&pB, Unsigned &padded) {
	if (pA->length() < pB->length()) {
		padded = *pA;
		padded.pad(pB->length());
		pA = &padded;
	}
	else {
		padded = *pB;
		padded.pad(pA->length());
		pB = &padded;
	}
}

void Unsigned::zero() {
#if __cplusplus >= 201103L
	for (auto &digit : mDigits) {
		digit = 0;
	}
#else
	for (digits_t::iterator it = mDigits.begin(); it != mDigits.end(); it++) {
		*it = 0;
	}
#endif
}

bool Unsigned::isZero() const {
	if (isEmpty()) return true;

	for (digits_t::const_iterator it = mDigits.begin(); it != mDigits.end(); it++) {
		if (*it != 0) return false;
	}

	return true;
}

bool Unsigned::isSmallNumber(const int n) const {
	if (length() != 1) return false;
	return mDigits[0] == n;
}

inline bool isOdd(const int n) { return n % 2 != 0; }
inline bool isEven(const int n) { return ! isOdd(n); }

bool Unsigned::isOdd() const {
	if (isEmpty()) return false;
	return ::isOdd(lastDigitAsInt());
}

bool Unsigned::isEven() const {
	if (isEmpty()) return true;
	return ::isEven(lastDigitAsInt());
}

void Unsigned::trimLeading() {
	digits_t::iterator lastZero;
	bool hits = false;

	for (digits_t::iterator it = mDigits.begin(); it != mDigits.end(); it++) {
		if (*it != 0) break;
		lastZero = it;
		hits = true;
	}

	if (!hits) return;

	if (lastZero != mDigits.end()) {
		lastZero++;	// Because erase() doesn't include the last index
	}

	if (lastZero == mDigits.end()) {
		lastZero--;	// Its all zeros, keep one
	}

	mDigits.erase(mDigits.begin(), lastZero);
}

long Unsigned::trimTrailing(const long atMost) {
	long nTrailing = 0;

	// Count trailing zeros (up to atMost)
	for (digits_t::reverse_iterator it = mDigits.rbegin(); it != mDigits.rend(); it++) {
		if (nTrailing >= atMost) break;
		if (*it != 0) break;
		nTrailing++;
	}

	if (nTrailing == 0) return 0;

	mDigits.erase(mDigits.end() - nTrailing, mDigits.end());
	return nTrailing;
}

long Unsigned::removeAtEnd(const long atMost) {
	const long nRemoved = std::min(atMost, (long)mDigits.size());
	if (nRemoved <= 0) return 0;
	mDigits.erase(mDigits.end() - nRemoved, mDigits.end());
	return nRemoved;
}

int Unsigned::charToInt(const char c) {
	return c - '0';
}

char Unsigned::intToChar(const int n) {
	if (n < 0) return '^';
	return (char)(n + '0');
}

void Unsigned::clear() {
	mDigits.clear();
}

void Unsigned::mkError() {
	clear();	// Doesn't really set an error.  But will case unit tests to fail which should raise a flag.
}

void Unsigned::saveNumber(const char *s) {
	for (const char *p = s; *p; p++) {
		if (*p == '.') {
			ArbNum_log("Decimals are not supported -- only integers: %s\n", s);
			mkError();
			return;
		}

		if (!isdigit(*p)) {
			ArbNum_log("Invalid number: %s\n", s);
			mkError();
			return;
		}

		append(charToInt(*p));
	}
	trimLeading();
}

// No validity checking
inline char intToCharFast(const int n) {
	return (char)(n + '0');
}

std::string Unsigned::toString() const {
	if (mDigits.empty()) {
		return "0";
	}

	std::string out;
	for (digits_t::const_iterator it = mDigits.begin(); it != mDigits.end(); it++) {
		out += intToCharFast(*it);
	}

	return out;
}

char Unsigned::firstDigitAsChar() const {
	if (isEmpty()) return 0;
	return intToChar(mDigits[0]);
}

int Unsigned::firstDigitAsInt() const {
	if (isEmpty()) return 0;
	return mDigits[0];
}

int Unsigned::lastDigitAsInt() const {
	if (isEmpty()) return 0;
	return mDigits[size() - 1];
}

unsigned int Unsigned::toUnsignedInt() const {
	return (unsigned int)atoi(toString().c_str());

	/*
	// Oddly, this is slower
	unsigned int out = 0;
	for (digits_t::const_iterator it = mDigits.begin(); it != mDigits.end(); it++) {
		out = out * 10 + *it;
	}
	return out;
	*/
}

int Unsigned::toInt() const {
	return atoi(toString().c_str());
}

long Unsigned::toLong() const {
	return atol(toString().c_str());
}

long long Unsigned::toLongLong() const {
	return atoll(toString().c_str());
}

void Unsigned::set(const Unsigned &in) {
	mDigits = in.mDigits;
}

Unsigned::Unsigned(const Unsigned &in) {
	set(in);
}

Unsigned::Unsigned(const char *in) {
	saveNumber(in);
}

Unsigned::Unsigned(const unsigned int n) {
	char	buf[100];

#ifdef __BORLANDC__
	sprintf(buf, "%u", n);
#else
	snprintf(buf, sizeof(buf), "%u", n);
#endif

	saveNumber(buf);
}

Unsigned::Unsigned(const int n) {
	char	buf[100];

#ifdef __BORLANDC__
	sprintf(buf, "%d", n);
#else
	snprintf(buf, sizeof(buf), "%d", n);
#endif

	saveNumber(buf);
}

Unsigned::Unsigned(const long n) {
	char	buf[100];

#ifdef __BORLANDC__
	sprintf(buf, "%ld", n);
#else
	snprintf(buf, sizeof(buf), "%ld", n);
#endif

	saveNumber(buf);
}

Unsigned::Unsigned(const long long n) {
	char	buf[100];

#ifdef __BORLANDC__
	sprintf(buf, "%lld", n);
#else
	snprintf(buf, sizeof(buf), "%lld", n);
#endif

	saveNumber(buf);
}

Unsigned::Unsigned(const bool b) {
	set((int)b);
}

int Unsigned::compare(const Unsigned &aIn, const Unsigned &bIn) {
	const Unsigned *pA = &aIn;
	const Unsigned *pB = &bIn;
	Unsigned padded;

	padShorterNumber(pA, pB, padded);

	for (size_t i = 0; i < pA->size(); i++) {
		if (pA->mDigits[i] < pB->mDigits[i]) return -1;
		if (pA->mDigits[i] > pB->mDigits[i]) return 1;
	}

	return 0;
}

// a += b
// Both a and b will be modified so if you don't want that send in a copy
inline void Unsigned::addMutable(Unsigned &workingA, Unsigned &workingB) {
	padShorterNumber(workingA, workingB);

	int carry = 0;
	for (long i = workingA.length() - 1; i >= 0; i--) {
		int digit = workingA[i] + workingB[i] + carry;
		if (digit > 9) {
			carry = 1;
			digit -= 10;
		}
		else {
			carry = 0;
		}
		workingA[i] = digit;
	}

	if (carry > 0) {
		workingA.prepend(carry);
	}

	workingA.trimLeading();
}

// out = a + b
Unsigned Unsigned::add(const Unsigned &a, const Unsigned &b) {
	Unsigned workingA(a);
	Unsigned workingB(b);

	addMutable(workingA, workingB);

	return workingA;
}

// a -= b
// Both a and b will be modified so if you don't want that send in a copy
void Unsigned::subtractMutable(Unsigned &workingA, Unsigned &workingB) {
	padShorterNumber(workingA, workingB);

	int borrow = 0;
	for (long i = workingA.length() - 1; i >= 0; i--) {
		workingA[i] = workingA[i] - borrow;
		if (workingB[i] > workingA[i]) {
			workingA[i] = workingA[i] + 10;
			borrow = 1;
		}
		else {
			borrow = 0;
		}

		const int digit = workingA[i] - workingB[i];
		workingA[i] = digit;
	}

	if (borrow > 0) {
		fprintf(stderr, "Unsigned::subtract: Should not have a borrow\n");
		workingA.mkError();
	}

	workingA.trimLeading();
}

Unsigned Unsigned::subtract(const Unsigned &a, const Unsigned &b) {
	Unsigned workingA(a);
	Unsigned workingB(b);

	subtractMutable(workingA, workingB);

	return workingA;
}

Unsigned Unsigned::multiply(const Unsigned &a, const Unsigned &b) {
	Unsigned sum(0);
	Unsigned partProduct;

	partProduct.pad(a.length() + b.length());
	for (long i = 0; i < b.length(); i++) {
		const long bIndex = b.length() - 1 - i;
		const int bDigit = b.mDigits[(size_t)bIndex];

		int carry = 0;
		partProduct.zero();
		long j = 0;
		for (; j < a.length(); j++) {
			const long aIndex = a.length() -1 - j;
			const int aDigit = a.mDigits[(size_t)aIndex];
			const int digit = bDigit * aDigit + carry;
			const long ppIndex = partProduct.length() -1 - j - i;
			partProduct.mDigits[(size_t)ppIndex] = digit % 10;
			carry = digit / 10;
		}

		for(; carry > 0; j++) {
			const long ppIndex = partProduct.length() -1 - j - i;
			partProduct.mDigits[(size_t)ppIndex] = carry % 10;
			carry /= 10;
		}

		sum.add(partProduct);
	}

	sum.trimLeading();
	return sum;
}

UnsignedDivide Unsigned::divideByOneWithRem(const Unsigned &dividend) {
	UnsignedDivide out(dividend, 0);
	return out;
}

// https://en.wikipedia.org/wiki/Division_by_two
UnsignedDivide Unsigned::divideByTwoWithRem(const Unsigned &dividend) {
	Unsigned working(dividend);
	UnsignedDivide out;

	working.prepend(0);

	const long len1 = working.length() - 1;
	int first = 0, second = 0;
	for (long i = 0; i < len1; i++) {
		first = working.mDigits[(size_t)i];
		second = working.mDigits[(size_t)i + 1];
		int save = 0;
		if (::isEven(first)) {
			switch(second) {
			case 0:
			case 1:
				 save = 0;
				 break;
			case 2:
			case 3:
				 save = 1;
				 break;
			case 4:
			case 5:
				 save = 2;
				 break;
			case 6:
			case 7:
				 save = 3;
				 break;
			case 8:
			case 9:
				 save = 4;
				 break;
			}
		}
		else {
			switch(second) {
			case 0:
			case 1:
				 save = 5;
				 break;
			case 2:
			case 3:
				 save = 6;
				 break;
			case 4:
			case 5:
				 save = 7;
				 break;
			case 6:
			case 7:
				 save = 8;
				 break;
			case 8:
			case 9:
				 save = 9;
				 break;
			}
		}
		out.quotient.append(save);
	}

	if (::isOdd(second)) {
		out.remainder = 1;
	}

	out.trimLeading();
	return out;
}

// Division by repeated subtraction
UnsignedDivide Unsigned::divideWithRemSlow(const Unsigned &dividend, const Unsigned &divisor) {
	UnsignedDivide out(0, dividend);

	if (divisor <= s_UnsignedTwo) {
		switch(divisor.toUnsignedInt()) {
		case 0:
			ArbNum_log("Division by zero\n");
			out.quotient.mkError();
			return out;
		case 1:
			return divideByOneWithRem(dividend);
		case 2:
			return divideByTwoWithRem(dividend);
		}
	}

	while (out.remainder >= divisor) {
		out.quotient++;
		out.remainder.subtract(divisor);
	}

	out.trimLeading();
	return out;
}

// Helper for divideWithRemFast()
Unsigned Unsigned::findDivisibleSizedChunk(const Unsigned &runningDividend, const Unsigned &dividend, const Unsigned &divisor, long &lDividendPos, bool &fits) {
	Unsigned chunk;
	lDividendPos = 0;
	for (; lDividendPos < dividend.length(); lDividendPos++) {
		int digit;
		if (runningDividend.mDigits[(size_t)lDividendPos] >= 0) {
			digit = runningDividend.mDigits[(size_t)lDividendPos];
		}
		else {
			digit = dividend.mDigits[(size_t)lDividendPos];
		}

		chunk.append(digit);

		if (chunk >= divisor) {
			fits = true;
			return chunk;
		}
	}

	fits = false;
	return chunk;
}

// https://en.wikipedia.org/wiki/Long_division#Example_with_multi-digit_divisor
UnsignedDivide Unsigned::divideWithRemFast(const Unsigned &dividend, const Unsigned &divisor) {
	Unsigned runningDividend;
	UnsignedDivide out;

	out.quotient.pad(dividend.length());
	runningDividend.append(UNASSIGNED, dividend.length());

	if (divisor <= s_UnsignedTwo) {
		switch(divisor.toUnsignedInt()) {
		case 0:
			ArbNum_log("Division by zero\n");
			out.quotient.mkError();
			return out;
		case 1:
			return divideByOneWithRem(dividend);
		case 2:
			return divideByTwoWithRem(dividend);
		}
	}

	long lDividendPos = 0;
	bool fits;
	for (;;) {
		const Unsigned chunk = findDivisibleSizedChunk(runningDividend, dividend, divisor, lDividendPos, fits);
		if (!fits) {
			out.remainder = chunk;
			break;
		}

		const UnsignedDivide partDivide = divideWithRemSlow(chunk, divisor);
		const Unsigned partProd = subtract(chunk, partDivide.remainder);
		out.quotient[lDividendPos] = partDivide.quotient.mDigits[0];

		runningDividend = subtract(chunk, partProd);
		runningDividend.append(UNASSIGNED, dividend.length() - lDividendPos - 1);
		runningDividend.pad(dividend.length()); 
	}

	out.trimLeading();
	return out;
}

UnsignedDivide Unsigned::divideWithRem(const Unsigned &a, const Unsigned &b) {
	return divideWithRemFast(a, b);
}

Unsigned Unsigned::divide(const Unsigned &a, const Unsigned &b) {
	return divideWithRemFast(a, b).quotient;
}

Unsigned Unsigned::half(const Unsigned &a) {
	return divideByTwoWithRem(a).quotient;
}

Unsigned Unsigned::mod(const Unsigned &a, const Unsigned &b) {
	return divideWithRemFast(a, b).remainder;
}

Unsigned Unsigned::pow(const Unsigned &base, const Unsigned &exp) {
	if (exp.isZero()) return s_UnsignedOne;

	const Unsigned x = pow(base, half(exp)); // Recursive

	if (exp.isEven()) {
		return x * x;
	}
	else {
		return base * x * x;
	}
}

// Greatest Common Divisor - Euler's method
Unsigned Unsigned::gcd(const Unsigned &a, const Unsigned &b) {
	if (a.isZero()) return b;
	return gcd(b % a, a); // Recursive
}

// Lowest Common Multiple (known as Lowest Common Denominator when adding fractions)
Unsigned Unsigned::lcm(const Unsigned &a, const Unsigned &b) {
	const Unsigned denom = gcd(a, b);
	if (denom.isZero()) return denom;
	return (a * b) / denom;
}

Unsigned Unsigned::factorial(const Unsigned &n) {
	if (n <= 1) return 1;
	return factorial(n - 1) * n;
}

// Square root - Newton's method
Unsigned Unsigned::sqrt(const Unsigned &s) {
	Unsigned x0 = half(s);

	if (x0.isZero()) return s;

	Unsigned x1 = half(x0 + s / x0);

	while (x1 < x0) {
		x0 = x1;
		x1 = half(x0 + s / x0);
	}

	return x0;
}

// Call rand() wantedDigits times and use the least significant digit each time
Unsigned Unsigned::random(const Unsigned &wantedDigits) {
	static bool bRandInitialized = false;

	if (!bRandInitialized) {
		bRandInitialized = true;
		srand((unsigned int)time(NULL));
	}

	Unsigned out;
	const long n = wantedDigits.toLong();
	if (n <= 0) return 0;
	for (long i = 0; i < n; i++) {
		const int r = rand();
		const int digit = r % 10;
		out.append(digit);
	}

	return out;
}

// By Soma Mbadiwe on https://stackoverflow.com/questions/15743192/check-if-number-is-prime-number
Unsigned Unsigned::isPrime(const Unsigned &a) {
	if (a <= s_UnsignedOne) return false;
	if (a.isTwo() || a.isThree() || a.isFive()) return true;
	if (a.isEven() || a % s_UnsignedThree == s_UnsignedZero || a % s_UnsignedFive == s_UnsignedZero) return false;

	const Unsigned boundary = sqrt(a);
	for (Unsigned i = s_UnsignedSix; i <= boundary; i += s_UnsignedSix) {
		if (a % (i + s_UnsignedOne) == s_UnsignedZero || a % (i + s_UnsignedFive) == s_UnsignedZero) {
			return false;
		}
	}

	return true;
}

void Unsigned::add(const Unsigned &other) {
	Unsigned workingB(other);

	addMutable(*this, workingB);
}

void Unsigned::subtract(const Unsigned &other) {
	Unsigned workingB(other);

	subtractMutable(*this, workingB);
}

void Unsigned::multiply(const Unsigned &other) {
	set(multiply(*this, other));
}

void Unsigned::divide(const Unsigned &other) {
	set(divide(*this, other));
}

void Unsigned::mod(const Unsigned &other) {
	set(mod(*this, other));
}

void Unsigned::pow(const Unsigned &other) {
	set(pow(*this, other));
}

Unsigned Unsigned::operator++(int) {
	add(s_UnsignedOne);
	return *this;
}

Unsigned Unsigned::operator--(int) {
	subtract(s_UnsignedOne);
	return *this;
}

//------------------------------------------------------------------------------
// Testing

bool Unsigned::testStoreStr(const char *in, const unsigned int correct) {
	const Unsigned myunsigned(in);
	const unsigned int result = myunsigned.toUnsignedInt();
	if (result != correct) {
		fprintf(stderr, "Store Str %u = %u (Unsigned) != %u (unsigned int) fail\n", correct, result, correct);
	}
	return result == correct;
}

bool Unsigned::testStoreUnsigned(const unsigned int correct) {
	const Unsigned myunsigned(correct);
	const unsigned int result = (unsigned int)atoi(myunsigned.toString().c_str());
	if (result != correct) {
		fprintf(stderr, "Store unsigned %u = %u (Unsigned) != %u (unsigned int) fail\n", correct, result, correct);
	}
	return result == correct;
}

bool Unsigned::testStoreLong(const long correct) {
	const Unsigned myunsigned(correct);
	const long result = atol(myunsigned.toString().c_str());
	if (result != correct) {
		fprintf(stderr, "Store Long %ld = %ld (Unsigned) != %ld (long) fail\n", correct, result, correct);
	}
	return result == correct;
}

bool Unsigned::testStoreLongLong(const long long lCorrect) {
	const Unsigned myunsigned(lCorrect);
	const long long lResult = atoll(myunsigned.toString().c_str());
	if (lResult != lCorrect) {
		fprintf(stderr, "Store Long Long %lld = %lld (Unsigned) != %lld (long long) fail\n", lCorrect, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

int Unsigned::compareNativeUnsigned(const unsigned int a, const unsigned int b) {
	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

bool Unsigned::testCompare(const unsigned int a, const unsigned int b) {
	const int result = Unsigned::compare(a, b);
	const int correct = compareNativeUnsigned(a, b);
	if (result != correct) {
		fprintf(stderr, "%u <> %u = %u (Unsigned) != %u (unsigned int) fail\n", a, b, result, correct);
		exit(0);
	}
	return result == correct;
}

bool Unsigned::testAdd(const unsigned int a, const unsigned int b) {
	const unsigned int result = Unsigned::add(a, b).toUnsignedInt();
	const unsigned int correct = a + b;
	if (result != correct) {
		fprintf(stderr, "%u + %u = %u (Unsigned) != %u (unsigned int) fail\n", a, b, result, correct);
	}
	return result == correct;
}

bool Unsigned::testSubtract(const unsigned int a, const unsigned int b) {
	const unsigned int result = Unsigned::subtract(a, b).toUnsignedInt();
	const unsigned int correct = a - b;
	if (result != correct) {
		fprintf(stderr, "%u - %u = %u (Unsigned) != %u (unsigned int) fail\n", a, b, result, correct);
	}
	return result == correct;
}

bool Unsigned::testMultiply(const unsigned int a, const unsigned int b) {
	const unsigned int result = Unsigned::multiply(a, b).toUnsignedInt();
	const unsigned int correct = a * b;
	if (result != correct) {
		fprintf(stderr, "%u * %u = %u (Unsigned) != %u (unsigned int) fail\n", a, b, result, correct);
	}
	return result == correct;
}

bool Unsigned::testDivide(const unsigned int a, const unsigned int b) {
	const unsigned int result = Unsigned::divide(a, b).toUnsignedInt();
	const unsigned int correct = a / b;
	if (result != correct) {
		fprintf(stderr, "%u / %u = %u (Unsigned) != %u (unsigned int) fail\n", a, b, result, correct);
	}
	return result == correct;
}

bool Unsigned::testMod(const unsigned int a, const unsigned int b) {
	const unsigned int result = Unsigned::mod(a, b).toUnsignedInt();
	const unsigned int correct = a % b;
	if (result != correct) {
		fprintf(stderr, "mod(%u, %u) = %u (Unsigned) != %u (unsigned int) fail\n", a, b, result, correct);
	}
	return result == correct;
}

bool Unsigned::testPow(const unsigned int a, const unsigned int b) {
	const unsigned int result = Unsigned::pow(a, b).toUnsignedInt();
	const unsigned int correct = (unsigned int) powLong(a, b);
	if (result != correct) {
		fprintf(stderr, "pow(%u, %u) = %u (Unsigned) != %u (unsigned int) fail\n", a, b, result, correct);
	}
	return result == correct;
}

unsigned int Unsigned::gcdNativeUnsigned(const unsigned int a, const unsigned int b) {
	if (a < 0 || b < 0) {
		ArbNum_log("gcdNativeUnsigned: got a negative number\n");
		return (unsigned int)-1;
	}
	if (a == 0) return b;
	return gcdNativeUnsigned(b % a, a);
}

bool Unsigned::testGcd(const unsigned int a, const unsigned int b) {
	const unsigned int result = Unsigned::gcd(a, b).toUnsignedInt();
#if __cplusplus >= 201703L
	const unsigned int correct = std::gcd(a, b);
#else
	const unsigned int correct = gcdNativeUnsigned(a, b);
#endif
	if (result != correct) {
		fprintf(stderr, "gcd(%u, %u) = %u (Unsigned) != %u (unsigned int) fail\n", a, b, result, correct);
	}
	return result == correct;
}

unsigned int Unsigned::lcmNativeUnsigned(const unsigned int a, const unsigned int b) {
	const unsigned int denom = gcdNativeUnsigned(a, b);
	if (denom == 0) return denom;
	return (a * b) / denom;
}

bool Unsigned::testLcm(const unsigned int a, const unsigned int b) {
	const unsigned int result = Unsigned::lcm(a, b).toUnsignedInt();
#if __cplusplus >= 201703L
	const unsigned int correct = std::lcm(a, b);
#else
	const unsigned int correct = lcmNativeUnsigned(a, b);
#endif
	if (result != correct) {
		fprintf(stderr, "lcm(%u, %u) = %u (Unsigned) != %u (unsigned int) fail\n", a, b, result, correct);
	}
	return result == correct;
}

unsigned int Unsigned::sqrtNativeUnsigned(const unsigned int a) {
	if (a < 0) {
		ArbNum_log("sqrtUnsigned: got a negative number\n");
		return (unsigned int)-1;
	}
	return (unsigned int)std::sqrt(a);
}

bool Unsigned::testSqrt(const unsigned int a) {
	const unsigned int result = Unsigned::sqrt(a).toUnsignedInt();
	const unsigned int correct = sqrtNativeUnsigned(a);
	if (result != correct) {
		fprintf(stderr, "sqrt(%u) = %u (Unsigned) != %u (unsigned int) fail\n", a, result, correct);
	}
	return result == correct;
}

// By Soma Mbadiwe on https://stackoverflow.com/questions/15743192/check-if-number-is-prime-number
unsigned int Unsigned::isPrimeNativeUnsigned(const unsigned int aIn) {
	const int a = (int)aIn; // Easier to deal with regular int
	if (a <= 1) return false;
	if (a == 2 || a == 3 || a == 5) return true;
	if (a % 2 == 0 || a % 3 == 0 || a % 5 == 0) return false;

	const int boundary = (int)std::sqrt(a);
	for (int i = 6; i <= boundary; i += 6) {
		if (a % (i + 1) == 0 || a % (i + 5) == 0) {
			return false;
		}
	}

	return true;
}

bool Unsigned::testIsPrime(const unsigned int a) {
	const unsigned int result = Unsigned::isPrime(a).toUnsignedInt();
	const unsigned int correct = isPrimeNativeUnsigned(a);
	if (result != correct) {
		fprintf(stderr, "isPrime(%u) = %u (Unsigned) != %u (unsigned int) fail\n", a, result, correct);
	}
	return result == correct;
}

unsigned int Unsigned::factorialNativeUnsigned(const unsigned int n) {
	if (n <= 1) return 1;
	return factorialNativeUnsigned(n - 1) * n;
}

bool Unsigned::testFactorial(const unsigned int a) {
	const unsigned int result = Unsigned::factorial(a).toUnsignedInt();
	const unsigned int correct = factorialNativeUnsigned(a);
	if (result != correct) {
		fprintf(stderr, "%u! = %u (Unsigned) != %u (unsigned int) fail\n", a, result, correct);
	}
	return result == correct;
}

void Unsigned::makeTestValues(const unsigned int start, const unsigned int end, const unsigned int inc, std::vector<unsigned int> &values) {
	values.clear();

	if (inc > 0) { // Counting up
		for (unsigned int n = start; n <= end; n += inc) {
			values.push_back(n);
		}

		// We want extra tests around zero
		for (unsigned int n = 0; n <= 10; n++) {
			values.push_back(n);
		}

		std::sort(values.begin(), values.end());
		(void) std::unique(values.begin(), values.end());
	} else { // Counting down
		for (unsigned int n = start; n >= end; n += inc) {
			values.push_back(n);
		}

		// We want extra tests around zero
		for (unsigned int n = 10; n >= 0; n--) {
			values.push_back(n);
		}

		// Reverse sort
		std::sort(values.begin(), values.end(), std::greater<>());
		(void) std::unique(values.begin(), values.end());
	}
}

bool Unsigned::testAll() {
	int nSuccess = 0;
	int nFail = 0;
	const long startMs = currentMillis();

	ArbNum_setLogging(AN_QUIET); // Don't log about invalid numbers
	{
	const char *checkStr[] = { "", "0000", "0004", "4000", "hello" };
	const unsigned int checkStrExpect[] = { 0, 0, 4, 4000, 0 };
	const int nCheckStr = sizeof(checkStr) / sizeof(checkStr[0]);
	printf("Testing store str with %d values\n", nCheckStr);
	for (int i = 0; i < nCheckStr; i++) {
		if (testStoreStr(checkStr[i], checkStrExpect[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	} }
	ArbNum_setLogging(AN_LOG);

	{
	const unsigned int checkUnsignedInt[] = { 9, 4, 3 };
	const int nCheckUnsignedInt = sizeof(checkUnsignedInt) / sizeof(checkUnsignedInt[0]);
	printf("Testing store unsigned int with %d values\n", nCheckUnsignedInt);
	for (int i = 0; i < nCheckUnsignedInt; i++) {
		if (testStoreUnsigned(checkUnsignedInt[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	} }

	{
	const long checkLong[] = { LONG_MIN, INT_MIN, SHRT_MIN, 0, SHRT_MAX, INT_MAX, LONG_MAX };
	const int nCheckLong = sizeof(checkLong) / sizeof(checkLong[0]);
	printf("Testing store long with %d values\n", nCheckLong);
	for (int i = 0; i < nCheckLong; i++) {
		if (checkLong[i] < 0) continue; // Skip negative (since we're unsigned)
		if (testStoreLong(checkLong[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printf("Testing store long long with %d values\n", nCheckLong);
	for (int i = 0; i < nCheckLong; i++) {
		if (checkLong[i] < 0) continue; // Skip negative (since we're unsigned)
		if (testStoreLongLong(checkLong[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	} }

	const unsigned int A_START = 0;
	const unsigned int A_END = 10000;
	const unsigned int A_INC = 77;
	const unsigned int B_START = 45;
	const unsigned int B_END = 4000;
	const unsigned int B_INC = 88;

	// Make vectors of test numbers
	std::vector<unsigned int> a_values, b_values;
	makeTestValues(A_START, A_END, A_INC, a_values);
	makeTestValues(B_START, B_END, B_INC, b_values);
	const long nValues = a_values.size() * b_values.size();

	printf("Testing compare with %ld values\n", nValues);
	for (unsigned int a : a_values) {
		for (unsigned int b : b_values) {
			if (testCompare(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		 }
	}

	printf("Testing add with %ld values\n", nValues);
	for (unsigned int a : a_values) {
		for (unsigned int b : b_values) {
			if (testAdd(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	printf("Testing subtract with %ld values\n", nValues);
	for (unsigned int a : a_values) {
		for (unsigned int b : b_values) {
			if (b > a) continue; // Avoid negative results (signed we're unsigned)
			if (testSubtract(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	printf("Testing multiply with %ld values\n", nValues);
	for (unsigned int a : a_values) {
		for (unsigned int b : b_values) {
			if (testMultiply(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	printf("Testing divide with %ld values\n", nValues);
	for (unsigned int a : a_values) {
		for (unsigned int b : b_values) {
			if (b == 0) continue; // Avoid divide by zero
			if (testDivide(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	printf("Testing mod with %ld values\n", nValues);
	for (unsigned int a : a_values) {
		for (unsigned int b : b_values) {
			if (b == 0) continue; // Avoid divide by zero
			if (testMod(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	printf("Testing pow with %ld values\n", nValues);
	for (unsigned int a = 0; a <= 10; a++) {
		for (unsigned int b = 1; b <= 9; b++) {
			if (testPow(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	printf("Testing gcd with %ld values\n", nValues);
	for (unsigned int a : a_values) {
		for (unsigned int b : b_values) {
			if (testGcd(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	printf("Testing lcm with %ld values\n", nValues);
	for (unsigned int a : a_values) {
		for (unsigned int b : b_values) {
			if (testLcm(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	printf("Testing sqrt with %d values\n", A_END / A_INC);
	for (unsigned int a = 0; a <= A_END; a += A_INC) {
		if (testSqrt(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printf("Testing isPrime with %d values\n", 100);
	for (unsigned int a = 0; a <= 100; a++) {
		if (testIsPrime(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printf("Testing factorial with %d values\n", 13);
	for (unsigned int a = 1; a < 13; a++) {
		if (testFactorial(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printTestResults("unsignedTest", nSuccess, nFail, startMs);
	return nFail == 0;
}
