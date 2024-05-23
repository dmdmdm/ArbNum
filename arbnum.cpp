// A simple arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "arbnum.h"

//------------------------------------------------------------------------------
// Unsigned

static Unsigned gUnsignedZero(0);
static Unsigned gUnsignedOne(1);
static Unsigned gUnsignedTwo(2);
static const int UNASSIGNED = -1;

void Unsigned::pad(const size_t len) {
	while (mDigits.size() < len) {
		prepend(0);
	}
}

void Unsigned::appendRight(const int digit, const size_t numberOfDigits) {
	for (size_t i = 0; i < numberOfDigits; i++) {
		append(digit);
	}
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
	for (digits_t::iterator it = mDigits.begin(); it != mDigits.end(); it++) {
		*it = 0;
	}
}

bool Unsigned::isZero() const {
	if (length() == 0) return true;

	for (digits_t::const_iterator it = mDigits.begin(); it != mDigits.end(); it++) {
		if (*it != 0) return false;
	}

	return true;
}

bool Unsigned::isOne() const {
	if (length() != 1) return false;
	return mDigits[0] == 1;
}

bool Unsigned::isTwo() const {
	if (length() != 1) return false;
	return mDigits[0] == 2;
}

inline bool isOdd(const int n) { return n % 2 != 0; }

inline bool isEven(const int n) { return ! isOdd(n); }

bool Unsigned::isOdd() const {
	if (length() == 0) return false;
	const int lastDigit = mDigits[length() - 1];
	return ::isOdd(lastDigit);
}

bool Unsigned::isEven() const {
	if (length() == 0) return true;
	const int lastDigit = mDigits[length() - 1];
	return ::isEven(lastDigit);
}

void Unsigned::trim() {
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

int Unsigned::charToInt(const char c) {
	return c - '0';
}

char Unsigned::intToChar(const int n) {
	if (n < 0) return '^';
	return (char)(n + '0');
}

void Unsigned::clear() {
	mDigits.erase(mDigits.begin(), mDigits.end());
}

void Unsigned::mkError() {
	clear();	// Doesn't really set an error.  But will case unit tests to fail which should raise a flag.
}

void Unsigned::saveNumber(const char *s) {
	for (const char *p = s; *p; p++) {
		if (*p == '.') {
			fprintf(stderr, "Decimals are not supported -- only integers: %s\n", s);
			mkError();
			return;
		}

		if (!isdigit(*p)) {
			fprintf(stderr, "Invalid number: %s\n", s);
			mkError();
			return;
		}

		append(charToInt(*p));
	}
	trim();
}

std::string Unsigned::toString() const {
	std::string out = "";

	for (digits_t::const_iterator it = mDigits.begin(); it != mDigits.end(); it++) {
		out += intToChar(*it);
	}

	return out;
}

int Unsigned::toInt() const {
	return atoi(toString().c_str());
}

long Unsigned::toLong() const {
	return atol(toString().c_str());
}

long long Unsigned::toLongLong() const {
#if defined(_MSC_VER) && _MSC_VER <= 1400
#pragma message("Sorry " __FUNCTION__ " isn't supported on this compiler")
	return atol(toString().c_str());
#else
	return atoll(toString().c_str());
#endif
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

// a += b
// Both a and b will be modified so if you don't want that send in a copy
inline void Unsigned::addMutable(Unsigned &workingA, Unsigned &workingB) {
	padShorterNumber(workingA, workingB);

	int carry = 0;
	for (int i = (int)workingA.length() - 1; i >= 0; i--) {
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

	workingA.trim();
}

// result = a + b
Unsigned Unsigned::add(const Unsigned &a, const Unsigned &b) {
	Unsigned workingA(a);
	Unsigned workingB(b);

	addMutable(workingA, workingB);

	return workingA;
}

// a -= b
//	Both a and b will be modified so if you don't want that send in a copy
void Unsigned::subtractMutable(Unsigned &workingA, Unsigned &workingB) {
	padShorterNumber(workingA, workingB);

	int borrow = 0;
	for (int i = (int)workingA.length() - 1; i >= 0; i--) {
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

	workingA.trim();
}

Unsigned Unsigned::subtract(const Unsigned &a, const Unsigned &b) {
	Unsigned workingA(a);
	Unsigned workingB(b);

	subtractMutable(workingA, workingB);

	return workingA;
}

Unsigned Unsigned::multiply(const Unsigned &a, const Unsigned &b) {
	Unsigned	sum(0);
	Unsigned	partProduct;

	partProduct.pad(a.length() + b.length());
	for (int i = 0; i < (int)b.length(); i++) {
		const int bIndex = (int)b.length() - 1 - i;
		const int bDigit = b.mDigits[bIndex];

		int carry = 0;
		partProduct.zero();
		int j = 0;
		for (; j < (int)a.length(); j++) {
			const int aIndex = (int)a.length() -1 - j;
			const int aDigit = a.mDigits[aIndex];
			const int digit = bDigit * aDigit + carry;
			const int ppIndex = (int)partProduct.length() -1 - j - i;
			partProduct.mDigits[ppIndex] = digit % 10;
			carry = digit / 10;
		}

		for(; carry > 0; j++) {
			const int ppIndex = (int)partProduct.length() -1 - j - i;
			partProduct.mDigits[ppIndex] = carry % 10;
			carry /= 10;
		}

		sum.add(partProduct);
	}

	sum.trim();
	return sum;
}

UnsignedDivide Unsigned::divideByOneWithRem(const Unsigned &dividend) {
	UnsignedDivide result(dividend, 0);
	return result;
}

// https://en.wikipedia.org/wiki/Division_by_two
UnsignedDivide Unsigned::divideByTwoWithRem(const Unsigned &dividend) {
	Unsigned working(dividend);
	UnsignedDivide result;

	working.prepend(0);

	const int len1 = (int)working.length() - 1;
	int first, second;
	for (int i = 0; i < len1; i++) {
		first = working.mDigits[i];
		second = working.mDigits[i + 1];
		int save;
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
		result.quotient.append(save);
	}

	if (::isOdd(second)) {
		result.remainder = 1;
	}

	result.trim();
	return result;
}

// Division by repeated subtraction
UnsignedDivide Unsigned::divideWithRemSlow(const Unsigned &dividend, const Unsigned &divisor) {
	UnsignedDivide result(0, dividend);

	if (divisor <= gUnsignedTwo) {
		switch(divisor.toInt()) {
	  case 0:
		  fprintf(stderr, "Division by zero\n");
		  result.quotient.mkError();
		  return result;
	  case 1:
		  return divideByOneWithRem(dividend);
	  case 2:
		  return divideByTwoWithRem(dividend);
		}
	}

	while (result.remainder >= divisor) {
		result.quotient++;
		result.remainder.subtract(divisor);
	}

	result.trim();
	return result;
}

// Helper for divideWithRemFast()
Unsigned Unsigned::findDivisibleSizedChunk(const Unsigned &runningDividend, const Unsigned &dividend, const Unsigned &divisor, int &iDividendPos, bool &fits) {
	Unsigned chunk;
	iDividendPos = 0;
	for (; iDividendPos < (int)dividend.length(); iDividendPos++) {
		int digit;
		if (runningDividend.mDigits[iDividendPos] >= 0) {
			digit = runningDividend.mDigits[iDividendPos];
		}
		else {
			digit = dividend.mDigits[iDividendPos];
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
	UnsignedDivide	result;

	result.quotient.pad(dividend.length());
	runningDividend.appendRight(UNASSIGNED, dividend.length());

	if (divisor <= gUnsignedTwo) {
		switch(divisor.toInt()) {
	  case 0:
		  fprintf(stderr, "Division by zero\n");
		  result.quotient.mkError();
		  return result;
	  case 1:
		  return divideByOneWithRem(dividend);
	  case 2:
		  return divideByTwoWithRem(dividend);
		}
	}

	int iDividendPos = 0;
	bool fits;
	for (;;) {
		const Unsigned chunk = findDivisibleSizedChunk(runningDividend, dividend, divisor, iDividendPos, fits);
		if (!fits) {
			result.remainder = chunk;
			break;
		}

		const UnsignedDivide partDivide = divideWithRemSlow(chunk, divisor);
		const Unsigned partProd = subtract(chunk, partDivide.remainder);
		result.quotient[iDividendPos] = partDivide.quotient.mDigits[0];

		runningDividend = subtract(chunk, partProd);
		runningDividend.appendRight(UNASSIGNED, dividend.length() - iDividendPos - 1);
		runningDividend.pad(dividend.length()); 
	}

	result.trim();
	return result;
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

Unsigned Unsigned::pow(const Unsigned &a, const Unsigned &n) {
	if (n.isZero()) return gUnsignedOne;

	const Unsigned x = pow(a, half(n));

	if (n.isEven()) {
		return x * x;
	}
	else {
		return a * x * x;
	}
}

// Euler's method
Unsigned Unsigned::gcd(const Unsigned &a, const Unsigned &b) {
	if (a.isZero()) return b;
	return gcd(b % a, a);
}

// Newton's method
Unsigned Unsigned::sqrt(const Unsigned &s) {
	Unsigned x0 = half(s);

	if (x0.isZero()) return s;

	Unsigned x1 = half(x0 + s / x0);

	while(x1 < x0) {
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

	Unsigned result;
	const long n = wantedDigits.toLong();
	if (n <= 0) return 0;
	for (long i = 0; i < n; i++) {
		const int r = rand();
		const int digit = r % 10;
		result.append(digit);
	}

	return result;
}

// By Soma Mbadiwe on https://stackoverflow.com/questions/15743192/check-if-number-is-prime-number
Unsigned Unsigned::isPrime(const Unsigned &a) {
	if (a <= 1) return false;
	if (a == 2 || a == 3 || a == 5) return true;
	if (a.isEven() || a % 3 == 0 || a % 5 == 0) return false;

	const Unsigned boundary = sqrt(a);
	for (Unsigned i = 6; i <= boundary; i += 6) {
		if (a % (i + 1) == 0 || a % (i + 5) == 0) {
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

int Unsigned::compare(const Unsigned &aIn, const Unsigned &bIn) {
	const Unsigned *pA = &aIn;
	const Unsigned *pB = &bIn;
	Unsigned padded;

	padShorterNumber(pA, pB, padded);

	for (size_t i = 0; i < pA->length(); i++) {
		if (pA->mDigits[i] < pB->mDigits[i]) return -1;
		if (pA->mDigits[i] > pB->mDigits[i]) return 1;
	}

	return 0;
}

Unsigned Unsigned::operator++(int) {
	add(gUnsignedOne);
	return *this;
}

Unsigned Unsigned::operator--(int) {
	subtract(gUnsignedOne);
	return *this;
}

//------------------------------------------------------------------------------
// ArbNum

static ArbNum gArbNumZero(0);
static ArbNum gArbNumOne(1);
static ArbNum gArbNumIntMin(INT_MIN);
static ArbNum gArbNumIntMax(INT_MAX);
static ArbNum gArbNumLongMin(LONG_MIN);
static ArbNum gArbNumLongMax(LONG_MAX);

void ArbNum::set(const ArbNum &in) {
	mSpecial = in.mSpecial;
	mSign = in.mSign;
	mUnsigned = in.mUnsigned;
}

ArbNum::ArbNum(const Unsigned &in) {
	clear();
	mUnsigned = in;
}

ArbNum::ArbNum(const ArbNum &in) {
	set(in);
}

ArbNum::ArbNum(const char *s) {
	saveNumber(s);
}

ArbNum::ArbNum(const int n) {
	char	buf[100];

#ifdef __BORLANDC__
	sprintf(buf, "%d", n);
#else
	snprintf(buf, sizeof(buf), "%d", n);
#endif

	saveNumber(buf);
}

ArbNum::ArbNum(const long n) {
	char	buf[100];

#ifdef __BORLANDC__
	sprintf(buf, "%ld", n);
#else
	snprintf(buf, sizeof(buf), "%ld", n);
#endif

	saveNumber(buf);
}

ArbNum::ArbNum(const long long n) {
	char	buf[100];

#ifdef __BORLANDC__
	sprintf(buf, "%lld", n);
#else
	snprintf(buf, sizeof(buf), "%lld", n);
#endif

	saveNumber(buf);
}

ArbNum::ArbNum(const bool b) {
	set((int) b);
}

void ArbNum::clear() {
	mSpecial = SPEC_NORMAL;
	mkPositive();
	mUnsigned.clear();
}

void ArbNum::saveNumber(const char *s) {
	clear();

	if (*s == '-') {
		mkNegative();
		s++;
	}

	mUnsigned.saveNumber(s);
}

ArbNum ArbNum::abs(const ArbNum &a) {
	return a.mUnsigned;
}

ArbNum ArbNum::sign(const ArbNum &a) {
	return a.mSign;
}

ArbNum ArbNum::add(const ArbNum &a, const ArbNum &b) {
	ArbNum result;
	if (a.isPositiveOrZero() && b.isPositiveOrZero()) {
		// Both positive => will stay positive
		result.mUnsigned = Unsigned::add(a.mUnsigned, b.mUnsigned);
	}
	else if (a.isPositiveOrZero() && b.isNegativeOrZero()) {
		// Remove sign from other, subtract other from this
		ArbNum workingB(b);
		workingB.mkPositive();
		result = ArbNum::subtract(a, workingB);  // Need to use ArbNum::subtract() because it checks to see which larger
	}
	else if (a.isNegativeOrZero() && b.isPositiveOrZero()) {
		// -a + b = b - a
		ArbNum workingA(a);
		workingA.mkPositive();
		result = ArbNum::subtract(b, workingA); // Need to use ArbNum::subtract() because it checks to see which larger
	}
	else if (a.isNegativeOrZero() && b.isNegativeOrZero()) {
		// Both negative => will stay negative
		result.mkNegative();
		result.mUnsigned = Unsigned::add(a.mUnsigned, b.mUnsigned);

	}
	else {
		fprintf(stderr, "add: unknown case\n");
		result.mkError();
	}
	return result;
}

ArbNum ArbNum::subtract(const ArbNum &a, const ArbNum &b) {
	ArbNum result;
	if (a.isPositiveOrZero() && b.isPositiveOrZero()) {
		if (a.mUnsigned < b.mUnsigned) {
			// a - b = - (b - a)
			result.mkNegative();
			result.mUnsigned = Unsigned::subtract(b.mUnsigned, a.mUnsigned);
		}
		else {
			// Easy a - b
			result.mUnsigned = Unsigned::subtract(a.mUnsigned, b.mUnsigned);
		}
	}
	else if (a.isPositiveOrZero() && b.isNegativeOrZero()) {
		// a - (-b) = a + b
		// Sign is already positive
		result.mUnsigned = Unsigned::add(a.mUnsigned, b.mUnsigned);
	}
	else if (a.isNegativeOrZero() && b.isPositiveOrZero()) {
		// (-a) - b = - (a + b)
		result.mkNegative();
		result.mUnsigned = Unsigned::add(a.mUnsigned, b.mUnsigned);
	}
	else if (a.isNegativeOrZero() && b.isNegativeOrZero()) {
		// (-a) - (-b) = b - a
		ArbNum workingA(a);
		ArbNum workingB(b);
		workingA.mkPositive();
		workingB.mkPositive();
		result = ArbNum::subtract(workingB, workingA);	// Recursive
	}
	else{
		fprintf(stderr, "subtract: unknown case\n");
		result.mkError();
	}
	return result;
}

ArbNum ArbNum::multiply(const ArbNum &a, const ArbNum &b) {
	ArbNum result;
	result.mUnsigned = Unsigned::multiply(a.mUnsigned, b.mUnsigned);
	result.mSign = a.mSign * b.mSign;
	return result;
}

// https://en.wikipedia.org/wiki/Division_algorithm
ArbNumDivide ArbNum::divideWithRem(const ArbNum &dividend, const ArbNum &divisor) {
	ArbNumDivide result;

	if (divisor.isZero()) {
		fprintf(stderr, "Division by zero\n");
		result.quotient.mkError();
		return result;
	}

	if (divisor.isNegative()) {
		ArbNum workingDivisor(divisor);
		workingDivisor.mkPositive();
		result = divideWithRem(dividend, workingDivisor);	// Recursive
		result.quotient.flipSign();
		return result;
	}

	if (dividend.isNegative()) {
		ArbNum workingDividend(dividend);
		workingDividend.mkPositive();
		result = divideWithRem(workingDividend, divisor);	// Recursive
		if (result.remainder.isZero()) {
			result.quotient.flipSign();
			return result;
		}
		else {
			result.quotient.flipSign();
			// We differ from the page Wikipedia page here
			result.remainder.flipSign();
			return result;
		}
	}

	// Both are positive
	const UnsignedDivide udivide = Unsigned::divideWithRemFast(dividend.mUnsigned, divisor.mUnsigned);
	result.quotient.mUnsigned = udivide.quotient;
	result.remainder.mUnsigned = udivide.remainder;
	return result;
}

ArbNum ArbNum::divide(const ArbNum &dividend, const ArbNum &divisor) {
	return divideWithRem(dividend, divisor).quotient;
}

ArbNum ArbNum::mod(const ArbNum &dividend, const ArbNum &divisor) {
	return divideWithRem(dividend, divisor).remainder;
}

ArbNum ArbNum::min(const ArbNum &a, const ArbNum &b) {
	return a < b ? a : b;
}

ArbNum ArbNum::max(const ArbNum &a, const ArbNum &b) {
	return a > b ? a : b;
}

ArbNum ArbNum::doNot(const ArbNum &a) {
	return a.isZero() ? gArbNumOne : gArbNumZero;
}

ArbNum ArbNum::pow(const ArbNum &a, const ArbNum &n) {
	ArbNum result = Unsigned::pow(a.mUnsigned, n.mUnsigned);

	if (a.isNegative()) {
		result.mSign = n.mUnsigned.isEven() ? 1 : -1;
	}

	return result;
}

ArbNum ArbNum::gcd(const ArbNum &a, const ArbNum &b) {
	return Unsigned::gcd(a.mUnsigned, b.mUnsigned);
}

ArbNum ArbNum::sqrt(const ArbNum &a) {
	if (a.isNegative()) {
		ArbNum result;
		result.mkError();
		return result;
	}
	return Unsigned::sqrt(a.mUnsigned);
}

ArbNum ArbNum::random(const ArbNum &n) {
	return Unsigned::random(n.mUnsigned);
}

ArbNum ArbNum::isPrime(const ArbNum &a) {
	if (a.isNegative()) return false;
	return Unsigned::isPrime(a.mUnsigned);
}

ArbNum ArbNum::factorial(const ArbNum &count) {
	if (count <= gArbNumOne) return gArbNumOne;
	return factorial(count - gArbNumOne) * count;
}

void ArbNum::add(const ArbNum &other) {
	set(add(*this, other));
}

void ArbNum::subtract(const ArbNum &other) {
	set(subtract(*this, other));
}

void ArbNum::multiply(const ArbNum &other) {
	set(multiply(*this, other));
}

void ArbNum::divide(const ArbNum &other) {
	set(divide(*this, other));
}

void ArbNum::mod(const ArbNum &other) {
	set(mod(*this, other));
}

void ArbNum::pow(const ArbNum &other) {
	set(pow(*this, other));
}

std::string ArbNum::toString() const {
	std::string out = "";

	if (isIgnore()) {
		return "ignore";
	}
	else if (isError()) {
		return "error";
	}

	if (mSign < 0) {
		out += "-";
	}

	out += mUnsigned.toString();

	return out;
}

int ArbNum::toInt() const {
	if (isZero()) return 0;

	if (*this < gArbNumIntMin) {
		fprintf(stderr, "Too small for an int: %s\n", toString().c_str());
		return -1;
	}

	if (*this > gArbNumIntMax) {
		fprintf(stderr, "Too large for an int: %s\n", toString().c_str());
		return -1;
	}

	return atoi(toString().c_str());
}

long ArbNum::toLong() const {
	if (isZero()) return 0;

	if (*this < gArbNumLongMin) {
		fprintf(stderr, "Too small for a long: %s\n", toString().c_str());
		return -1;
	}

	if (*this > gArbNumLongMax) {
		fprintf(stderr, "Too large for a long: %s\n", toString().c_str());
		return -1;
	}

	return atol(toString().c_str());
}

long long ArbNum::toLongLong() const {
#if defined(_MSC_VER) && _MSC_VER <= 1400
#pragma message("Sorry " __FUNCTION__ " isn't supported on this compiler")
	return atol(toString().c_str());
#else
	return atoll(toString().c_str());
#endif
}

int ArbNum::compare(const ArbNum &a, const ArbNum &b) {
	if (a.mSign < b.mSign) return -1;
	if (a.mSign > b.mSign) return 1;

	if (a.isNegative()) {
		// Both are negative
		return Unsigned::compare(b.mUnsigned, a.mUnsigned);
	}	
	else {
		// Both are postive
		return Unsigned::compare(a.mUnsigned, b.mUnsigned);
	}
}

ArbNum ArbNum::operator++(int) {
	add(gArbNumOne);
	return *this;
}

ArbNum ArbNum::operator--(int) {
	subtract(gArbNumOne);
	return *this;
}

//------------------------------------------------------------------------------
// Testing

bool ArbNum::testStoreStr(const char *in, const int iCorrect) {
	const ArbNum arb(in);
	const int iResult = arb.toInt();
	if (iResult != iCorrect) {
		fprintf(stderr, "Store Str %d = %d (ArbNum) = %d (int) fail\n", iCorrect, iResult, iCorrect);
	}
	return iResult == iCorrect;
}

bool ArbNum::testStoreInt(const int iCorrect) {
	const ArbNum arb(iCorrect);
	const int iResult = arb.toInt();
	if (iResult != iCorrect) {
		fprintf(stderr, "Store Int %d = %d (ArbNum) = %d (int) fail\n", iCorrect, iResult, iCorrect);
	}
	return iResult == iCorrect;
}

bool ArbNum::testStoreLong(const long lCorrect) {
	const ArbNum arb(lCorrect);
	const long lResult = arb.toLong();
	if (lResult != lCorrect) {
		fprintf(stderr, "Store Long %ld = %ld (ArbNum) = %ld (long) fail\n", lCorrect, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool ArbNum::testStoreLongLong(const long long lCorrect) {
	const ArbNum arb(lCorrect);
	const long long lResult = arb.toLongLong();
	if (lResult != lCorrect) {
		fprintf(stderr, "Store Long Long %lld = %lld (ArbNum) = %lld (long) fail\n", lCorrect, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

int ArbNum::compareLong(const long a, const long b) {
	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

bool ArbNum::testCompare(const long a, const long b) {
	const int lResult = ArbNum::compare(a, b);
	const int lCorrect = compareLong(a, b);
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld <> %ld = %d (ArbNum) != %d (long) fail\n", a, b, lResult, lCorrect);
		exit(0);
	}
	return lResult == lCorrect;
}

bool ArbNum::testAdd(const long a, const long b) {
	const long lResult = ArbNum::add(a, b).toLong();
	const long lCorrect = a + b;
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld + %ld = %ld (ArbNum) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool ArbNum::testSubtract(const long a, const long b) {
	const long lResult = ArbNum::subtract(a, b).toLong();
	const long lCorrect = a - b;
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld - %ld = %ld (ArbNum) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool ArbNum::testMultiply(const long a, const long b) {
	const long lResult = ArbNum::multiply(a, b).toLong();
	const long lCorrect = a * b;
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld * %ld = %ld (ArbNum) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool ArbNum::testDivide(const long a, const long b) {
	const long lResult = ArbNum::divide(a, b).toLong();
	const long lCorrect = a / b;
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld / %ld = %ld (ArbNum) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool ArbNum::testMod(const long a, const long b) {
	const long lResult = ArbNum::mod(a, b).toLong();
	const long lCorrect = a % b;
	if (lResult != lCorrect) {
		fprintf(stderr, "mod(%ld, %ld) = %ld (ArbNum) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool ArbNum::testPow(const long a, const long b) {
	const long lResult = ArbNum::pow(a, b).toLong();
	const long lCorrect = (long) ::pow((double)a, (double)b);
	if (lResult != lCorrect) {
		fprintf(stderr, "pow(%ld, %ld) = %ld (ArbNum) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

long ArbNum::gcdLong(const long a, const long b) {
	if (a < 0 || b < 0) {
		return gcdLong(::abs(a), ::abs(b));
	}
	if (a == 0) return b;
	return gcdLong(b % a, a);
}

bool ArbNum::testGcd(const long a, const long b) {
	const long lResult = ArbNum::gcd(a, b).toLong();
	const long lCorrect = gcdLong(a, b);
	if (lResult != lCorrect) {
		fprintf(stderr, "gcd(%ld, %ld) = %ld (ArbNum) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

long ArbNum::sqrtLong(const long s) {
	if (s < 0) {
		return sqrtLong(::abs(s));
	}
	return (long) ::sqrt((double)s);
}

bool ArbNum::testSqrt(const long a) {
	const long lResult = ArbNum::sqrt(a).toLong();
	const long lCorrect = sqrtLong(a);
	if (lResult != lCorrect) {
		fprintf(stderr, "sqrt(%ld) = %ld (ArbNum) != %ld (long) fail\n", a, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

// By Soma Mbadiwe on https://stackoverflow.com/questions/15743192/check-if-number-is-prime-number
long ArbNum::isPrimeLong(const long a) {
	if (a <= 1) return false;
	if (a == 2 || a == 3 || a == 5) return true;
	if (a % 2 == 0 || a % 3 == 0 || a % 5 == 0) return false;

	const long boundary = (long)::sqrt((double)a);
	for (long i = 6; i <= boundary; i += 6) {
		if (a % (i + 1) == 0 || a % (i + 5) == 0) {
			return false;
		}
	}

	return true;
}

bool ArbNum::testIsPrime(const long a) {
	const long lResult = ArbNum::isPrime(a).toLong();
	const long lCorrect = isPrimeLong(a);
	if (lResult != lCorrect) {
		fprintf(stderr, "isPrime(%ld) = %ld (ArbNum) != %ld (long) fail\n", a, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

long ArbNum::factorialLong(const long n) {
	if (n <= 1) return 1;
	return factorialLong(n - 1) * n;
}

bool ArbNum::testFactorial(const long a) {
	const long lResult = ArbNum::factorial(a).toLong();
	const long lCorrect = factorialLong(a);
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld! = %ld (ArbNum) != %ld (long) fail\n", a, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool ArbNum::testAll() {
	int nSuccess = 0;
	int nFail = 0;

	const char *checkStr[] = { "", "0000", "0004", "4000", "hello", "-25" };
	const int checkStrExpect[] = { 0, 0, 4, 4000, 0, -25 };
	const int nCheckStr = sizeof(checkStr) / sizeof(checkStr[0]);
	for (int i = 0; i < nCheckStr; i++) {
		printf("Testing store str %s\n", checkStr[i]);
		if (testStoreStr(checkStr[i], checkStrExpect[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	const int checkInt[] = { INT_MIN, SHRT_MIN, 0, SHRT_MAX, INT_MAX };
	const int nCheckInt = sizeof(checkInt) / sizeof(checkInt[0]);
	for (int i = 0; i < nCheckInt; i++) {
		printf("Testing store int %d\n", checkInt[i]);
		if (testStoreInt(checkInt[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	const long checkLong[] = { LONG_MIN, INT_MIN, SHRT_MIN, 0, SHRT_MAX, INT_MAX, LONG_MAX };
	const long nCheckLong = sizeof(checkLong) / sizeof(checkLong[0]);
	for (int i = 0; i < nCheckLong; i++) {
		printf("Testing store long %ld\n", checkLong[i]);
		if (testStoreLong(checkLong[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	for (int i = 0; i < nCheckLong; i++) {
		printf("Testing store long long %ld\n", checkLong[i]);
		if (testStoreLongLong(checkLong[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	const long A_START = -10000;
	const long A_END = 10000;
	const long A_INC = 77;
	const long B_START = 10000;
	const long B_END = -10000;
	const long B_DEC = 88;
	long a, b;

	for (a = A_START; a <= A_END; a += A_INC) {
		printf("Testing compare with %ld on the left\n", a);
		for (b = B_START; b >= B_END; b -= B_DEC) {
			if (testCompare(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
		 }
		}
	}

	for (a = A_START; a <= A_END; a += A_INC) {
		printf("Testing add with %ld on the left\n", a);
		for (b = B_START; b >= B_END; b -= B_DEC) {
			if (testAdd(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (a = A_START; a <= A_END; a += A_INC) {
		printf("Testing subtract with %ld on the left\n", a);
		for (b = B_START; b >= B_END; b -= B_DEC) {
			if (testSubtract(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (a = A_START; a <= A_END; a += A_INC) {
		printf("Testing multiply with %ld on the left\n", a);
		for (b = B_START; b >= B_END; b -= B_DEC) {
			if (testMultiply(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (a = -10; a < 10; a++) {
		printf("Testing pow with %ld on the left\n", a);
		for (b = 1; b <= 9; b++) {
			if (testPow(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (a = A_START; a <= A_END; a += A_INC) {
		printf("Testing divide with %ld on the left\n", a);
		for (b = B_START; b >= B_END; b -= B_DEC) {
			if (testDivide(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (a = A_START; a <= A_END; a += A_INC) {
		printf("Testing mod with %ld on the left\n", a);
		for (b = B_START; b >= B_END; b -= B_DEC) {
			if (testMod(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (a = A_START; a <= A_END; a += A_INC) {
		printf("Testing gcd with %ld on the left\n", a);
		for (b = B_START; b >= B_END; b -= B_DEC) {
			if (testGcd(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (a = 0; a <= A_END; a += A_INC) {
		printf("Testing sqrt with %ld\n", a);
		if (testSqrt(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	for (a = 0; a <= 100; a++) {
		printf("Testing isPrime with %ld\n", a);
		if (testIsPrime(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printf("Testing factorials\n");
	for (a = 1; a < 13; a++) {
		if (testFactorial(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printf("Success: %d  Fail: %d\n", nSuccess, nFail);
	return nFail == 0;
}
