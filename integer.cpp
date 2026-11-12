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
#include <algorithm>
#include "util.h"
#include "integer.h"

static const Integer s_IntegerZero(0);
static const Integer s_IntegerOne(1);
static const Integer s_IntegerTwo(2);
static const Integer s_IntegerIntMin(INT_MIN);
static const Integer s_IntegerIntMax(INT_MAX);
static const Integer s_IntegerLongMin(LONG_MIN);
static const Integer s_IntegerLongMax(LONG_MAX);

void Integer::set(const Integer &in) {
	mSpecial = in.mSpecial;
	mSign = in.mSign;
	mUnsigned = in.mUnsigned;
}

Integer::Integer(const Unsigned &in) {
	clear();
	mUnsigned = in;
}

Integer::Integer(const Integer &in) {
	set(in);
}

Integer::Integer(const char *s) {
	saveNumber(s);
}

Integer::Integer(const int n) {
	char	buf[100];

#ifdef __BORLANDC__
	sprintf(buf, "%d", n);
#else
	snprintf(buf, sizeof(buf), "%d", n);
#endif

	saveNumber(buf);
}

Integer::Integer(const long n) {
	char	buf[100];

#ifdef __BORLANDC__
	sprintf(buf, "%ld", n);
#else
	snprintf(buf, sizeof(buf), "%ld", n);
#endif

	saveNumber(buf);
}

Integer::Integer(const long long n) {
	char	buf[100];

#ifdef __BORLANDC__
	sprintf(buf, "%lld", n);
#else
	snprintf(buf, sizeof(buf), "%lld", n);
#endif

	saveNumber(buf);
}

Integer::Integer(const bool b) {
	set((int) b);
}

long Integer::trimTrailing(const long atMost) {
	return mUnsigned.trimTrailing(atMost);
}

void Integer::clear() {
	// A checker said mSign isn't set in this function but it is set in mkPositive()
	mSpecial = SPEC_NORMAL;
	mkPositive();
	mUnsigned.clear();
}

void Integer::multiplyByTen(const long exp) {
	for (long i = 0; i < exp; i++) {
		multiplyByTen();
	}
}

void Integer::saveNumber(const char *s) {
	clear();

	if (*s == '-') {
		mkNegative();
		s++;
	}
	else if (*s == '+') { // found in scientific notation
		// Don't need to set as positive
		s++;
	}

	mUnsigned.saveNumber(s);
}

Integer Integer::abs(const Integer &a) {
	return a.mUnsigned;
}

Integer Integer::sign(const Integer &a) {
	return a.mSign;
}

int Integer::compare(const Integer &a, const Integer &b) {
	if (a.mSign < b.mSign) return -1;
	if (a.mSign > b.mSign) return 1;

	if (a.isNegative()) {
		// Both are negative
		return Unsigned::compare(b.mUnsigned, a.mUnsigned);
	}
	else {
		// Both are positive
		return Unsigned::compare(a.mUnsigned, b.mUnsigned);
	}
}

Integer Integer::add(const Integer &a, const Integer &b) {
	Integer result;
	if (a.isPositiveOrZero() && b.isPositiveOrZero()) {
		// Both positive => will stay positive
		result.mUnsigned = Unsigned::add(a.mUnsigned, b.mUnsigned);
	}
	else if (a.isPositiveOrZero() && b.isNegativeOrZero()) {
		// Remove sign from other, subtract other from this
		Integer workingB(b);
		workingB.mkPositive();
		result = Integer::subtract(a, workingB); // Need to use Integer::subtract() because it checks to see which larger
	}
	else if (a.isNegativeOrZero() && b.isPositiveOrZero()) {
		// -a + b = b - a
		Integer workingA(a);
		workingA.mkPositive();
		result = Integer::subtract(b, workingA); // Need to use Integer::subtract() because it checks to see which larger
	}
	else if (a.isNegativeOrZero() && b.isNegativeOrZero()) {
		// Both negative => will stay negative
		result.mkNegative();
		result.mUnsigned = Unsigned::add(a.mUnsigned, b.mUnsigned);
	}
	else {
		ArbNum_log("add: unknown case\n");
		result.mkError();
	}
	return result;
}

Integer Integer::subtract(const Integer &a, const Integer &b) {
	Integer result;
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
		Integer workingA(a);
		Integer workingB(b);
		workingA.mkPositive();
		workingB.mkPositive();
		result = Integer::subtract(workingB, workingA);	// Recursive
	}
	else{
		ArbNum_log("subtract: unknown case\n");
		result.mkError();
	}
	return result;
}

Integer Integer::multiply(const Integer &a, const Integer &b) {
	Integer result;
	result.mUnsigned = Unsigned::multiply(a.mUnsigned, b.mUnsigned);
	result.mSign = a.mSign * b.mSign;
	return result;
}

Integer Integer::half(const Integer &a) {
	return divideWithRem(a, s_IntegerTwo).quotient;
}

// https://en.wikipedia.org/wiki/Division_algorithm
IntegerDivide Integer::divideWithRem(const Integer &dividend, const Integer &divisor) {
	IntegerDivide result;

	if (divisor.isZero()) {
		ArbNum_log("Division by zero\n");
		result.quotient.mkError();
		return result;
	}

	if (divisor.isNegative()) {
		Integer workingDivisor(divisor);
		workingDivisor.mkPositive();
		result = divideWithRem(dividend, workingDivisor);	// Recursive
		result.quotient.flipSign();
		return result;
	}

	if (dividend.isNegative()) {
		Integer workingDividend(dividend);
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

	// Now, both are positive
	const UnsignedDivide udivide = Unsigned::divideWithRem(dividend.mUnsigned, divisor.mUnsigned);
	result.quotient.mUnsigned = udivide.quotient;
	result.remainder.mUnsigned = udivide.remainder;
	return result;
}

Integer Integer::divide(const Integer &dividend, const Integer &divisor) {
	return divideWithRem(dividend, divisor).quotient;
}

Integer Integer::mod(const Integer &dividend, const Integer &divisor) {
	return divideWithRem(dividend, divisor).remainder;
}

Integer Integer::min(const Integer &a, const Integer &b) {
	return a < b ? a : b;
}

Integer Integer::max(const Integer &a, const Integer &b) {
	return a > b ? a : b;
}

Integer Integer::doNot(const Integer &a) {
	return a.isZero() ? s_IntegerOne : s_IntegerZero;
}

Integer Integer::pow(const Integer &base, const Integer &exp) {
	Integer result = Unsigned::pow(base.mUnsigned, exp.mUnsigned);

	if (base.isNegative()) {
		result.mSign = exp.mUnsigned.isEven() ? 1 : -1;
	}

	return result;
}

Integer Integer::gcd(const Integer &a, const Integer &b) {
	return Unsigned::gcd(a.mUnsigned, b.mUnsigned);
}

Integer Integer::lcm(const Integer &a, const Integer &b) {
	const Integer denom = gcd(a, b);
	if (denom.isZero()) return denom;
	return (a * b) / denom;
}

Integer Integer::sqrt(const Integer &a) {
	if (a.isNegative()) {
		Integer result;
		result.mkError();
		return result;
	}
	return Unsigned::sqrt(a.mUnsigned);
}

Integer Integer::random(const Integer &n) {
	return Unsigned::random(n.mUnsigned);
}

Integer Integer::isPrime(const Integer &a) {
	if (a.isNegative()) return false;
	return Unsigned::isPrime(a.mUnsigned);
}

Integer Integer::factorial(const Integer &count) {
	if (count <= s_IntegerOne) return s_IntegerOne;
	return factorial(count - s_IntegerOne) * count;
}

void Integer::add(const Integer &other) {
	set(add(*this, other));
}

void Integer::subtract(const Integer &other) {
	set(subtract(*this, other));
}

void Integer::multiply(const Integer &other) {
	set(multiply(*this, other));
}

void Integer::divide(const Integer &other) {
	set(divide(*this, other));
}

void Integer::mod(const Integer &other) {
	set(mod(*this, other));
}

void Integer::pow(const Integer &other) {
	set(pow(*this, other));
}

std::string Integer::toString(const SIGN_t eSign) const {
	std::string out = "";

	if (isIgnore()) {
		return "ignore";
	}
	else if (isError()) {
		return "error";
	}

	if (eSign == WITH_SIGN) {
		if (mSign < 0) {
			out += "-";
		}
	}

	out += mUnsigned.toString();

	return out;
}

char Integer::firstDigitAsChar() const {
	return mUnsigned.firstDigitAsChar();
}

int Integer::firstDigitAsInt() const {
	return mUnsigned.firstDigitAsInt();
}

int Integer::toInt() const {
	if (isZero()) return 0;

	if (*this < s_IntegerIntMin) {
		ArbNum_log("Too small for an int: %s\n", toString().c_str());
		return -1;
	}

	if (*this > s_IntegerIntMax) {
		ArbNum_log("Too large for an int: %s\n", toString().c_str());
		return -1;
	}

	return atoi(toString().c_str());
}

long Integer::toLong() const {
	if (isZero()) return 0;

	if (*this < s_IntegerLongMin) {
		ArbNum_log("Too small for a long: %s\n", toString().c_str());
		return -1;
	}

	if (*this > s_IntegerLongMax) {
		ArbNum_log("Too large for a long: %s\n", toString().c_str());
		return -1;
	}

	return atol(toString().c_str());
}

long long Integer::toLongLong() const {
#if defined(_MSC_VER) && _MSC_VER <= 1400
#pragma message("Sorry " __FUNCTION__ " isn't supported on this compiler")
	return atol(toString().c_str());
#else
	return atoll(toString().c_str());
#endif
}

double Integer::toDouble() const {
	return atof(toString().c_str());
}

Integer Integer::operator++(int) {
	add(s_IntegerOne);
	return *this;
}

Integer Integer::operator--(int) {
	subtract(s_IntegerOne);
	return *this;
}

//------------------------------------------------------------------------------
// Testing

bool Integer::testStoreStr(const char *in, const int iCorrect) {
	const Integer myint(in);
	const int iResult = myint.toInt();
	if (iResult != iCorrect) {
		fprintf(stderr, "Store Str %d = %d (Integer) != %d (int) fail\n", iCorrect, iResult, iCorrect);
	}
	return iResult == iCorrect;
}

bool Integer::testStoreInt(const int iCorrect) {
	const Integer myint(iCorrect);
	const int iResult = myint.toInt();
	if (iResult != iCorrect) {
		fprintf(stderr, "Store Int %d = %d (Integer) != %d (int) fail\n", iCorrect, iResult, iCorrect);
	}
	return iResult == iCorrect;
}

bool Integer::testStoreLong(const long lCorrect) {
	const Integer myint(lCorrect);
	const long lResult = myint.toLong();
	if (lResult != lCorrect) {
		fprintf(stderr, "Store Long %ld = %ld (Integer) != %ld (long) fail\n", lCorrect, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool Integer::testStoreLongLong(const long long lCorrect) {
	const Integer myint(lCorrect);
	const long long lResult = myint.toLongLong();
	if (lResult != lCorrect) {
		fprintf(stderr, "Store Long Long %lld = %lld (Integer) != %lld (long long) fail\n", lCorrect, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

int Integer::compareLong(const long a, const long b) {
	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

bool Integer::testCompare(const long a, const long b) {
	const int lResult = Integer::compare(a, b);
	const int lCorrect = compareLong(a, b);
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld <> %ld = %d (Integer) != %d (long) fail\n", a, b, lResult, lCorrect);
		exit(0);
	}
	return lResult == lCorrect;
}

bool Integer::testAdd(const long a, const long b) {
	const long lResult = Integer::add(a, b).toLong();
	const long lCorrect = a + b;
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld + %ld = %ld (Integer) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool Integer::testSubtract(const long a, const long b) {
	const long lResult = Integer::subtract(a, b).toLong();
	const long lCorrect = a - b;
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld - %ld = %ld (Integer) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool Integer::testMultiply(const long a, const long b) {
	const long lResult = Integer::multiply(a, b).toLong();
	const long lCorrect = a * b;
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld * %ld = %ld (Integer) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool Integer::testDivide(const long a, const long b) {
	const long lResult = Integer::divide(a, b).toLong();
	const long lCorrect = a / b;
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld / %ld = %ld (Integer) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool Integer::testMod(const long a, const long b) {
	const long lResult = Integer::mod(a, b).toLong();
	const long lCorrect = a % b;
	if (lResult != lCorrect) {
		fprintf(stderr, "mod(%ld, %ld) = %ld (Integer) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool Integer::testPow(const long a, const long b) {
	const long lResult = Integer::pow(a, b).toLong();
	const long lCorrect = powLong(a, b);
	if (lResult != lCorrect) {
		fprintf(stderr, "pow(%ld, %ld) = %ld (Integer) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

long Integer::gcdLong(const long a, const long b) {
	if (a < 0 || b < 0) {
		return gcdLong(std::abs(a), std::abs(b));
	}
	if (a == 0) return b;
	return gcdLong(b % a, a);
}

bool Integer::testGcd(const long a, const long b) {
	const long lResult = Integer::gcd(a, b).toLong();
	const long lCorrect = gcdLong(a, b);
	if (lResult != lCorrect) {
		fprintf(stderr, "gcd(%ld, %ld) = %ld (Integer) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

long Integer::lcmLong(const long a, const long b) {
	const long denom = gcdLong(a, b);
	if (denom == 0) return denom;
	return (a * b) / denom;
}

bool Integer::testLcm(const long a, const long b) {
	const long lResult = Integer::lcm(a, b).toLong();
	const long lCorrect = lcmLong(a, b);
	if (lResult != lCorrect) {
		fprintf(stderr, "lcm(%ld, %ld) = %ld (Integer) != %ld (long) fail\n", a, b, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool Integer::testSqrt(const long a) {
	const long lResult = Integer::sqrt(a).toLong();
	const long lCorrect = sqrtLong(a);
	if (lResult != lCorrect) {
		fprintf(stderr, "sqrt(%ld) = %ld (Integer) != %ld (long) fail\n", a, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

// By Soma Mbadiwe on https://stackoverflow.com/questions/15743192/check-if-number-is-prime-number
long Integer::isPrimeLong(const long a) {
	if (a <= 1) return false;
	if (a == 2 || a == 3 || a == 5) return true;
	if (a % 2 == 0 || a % 3 == 0 || a % 5 == 0) return false;

	const long boundary = sqrtLong(a);
	for (long i = 6; i <= boundary; i += 6) {
		if (a % (i + 1) == 0 || a % (i + 5) == 0) {
			return false;
		}
	}

	return true;
}

bool Integer::testIsPrime(const long a) {
	const long lResult = Integer::isPrime(a).toLong();
	const long lCorrect = isPrimeLong(a);
	if (lResult != lCorrect) {
		fprintf(stderr, "isPrime(%ld) = %ld (Integer) != %ld (long) fail\n", a, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

long Integer::factorialLong(const long n) {
	if (n <= 1) return 1;
	return factorialLong(n - 1) * n;
}

bool Integer::testFactorial(const long a) {
	const long lResult = Integer::factorial(a).toLong();
	const long lCorrect = factorialLong(a);
	if (lResult != lCorrect) {
		fprintf(stderr, "%ld! = %ld (Integer) != %ld (long) fail\n", a, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

void Integer::makeTestValues(const long start, const long end, const long inc, std::vector<long> &values) {
	values.clear();

	if (inc > 0) { // Counting up
		for (long n = start; n <= end; n += inc) {
			values.push_back(n);
		}

		// We want extra tests around zero
		for (long n = -10; n <= 10; n++) {
			values.push_back(n);
		}

		std::sort(values.begin(), values.end());
		(void) std::unique(values.begin(), values.end());
	} else { // Counting down
		for (long n = start; n >= end; n += inc) {
			values.push_back(n);
		}

		// We want extra tests around zero
		for (long n = 10; n >= -10; n--) {
			values.push_back(n);
		}

		// Reverse sort
		std::sort(values.begin(), values.end(), std::greater<>());
		(void) std::unique(values.begin(), values.end());
	}
}

bool Integer::testAll() {
	int nSuccess = 0;
	int nFail = 0;
	const long startMs = currentMillis();

	ArbNum_setLogging(AN_QUIET); // Don't log about invalid numbers
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
	ArbNum_setLogging(AN_LOG);

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
	const long B_INC = -88;

	// Make vectors of test numbers
	std::vector<long> a_values, b_values;
	makeTestValues(A_START, A_END, A_INC, a_values);
	makeTestValues(B_START, B_END, B_INC, b_values);

	for (long a : a_values) {
		printf("Testing compare with %ld on the left\n", a);
		for (long b : b_values) {
			if (testCompare(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		 }
	}

	for (long a : a_values) {
		printf("Testing add with %ld on the left\n", a);
		for (long b : b_values) {
			if (testAdd(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (long a : a_values) {
		printf("Testing subtract with %ld on the left\n", a);
		for (long b : b_values) {
			if (testSubtract(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (long a : a_values) {
		printf("Testing multiply with %ld on the left\n", a);
		for (long b : b_values) {
			if (testMultiply(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (long a : a_values) {
		printf("Testing divide with %ld on the left\n", a);
		for (long b : b_values) {
			if (b == 0) continue; // Avoid divide by zero
			if (testDivide(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (long a : a_values) {
		printf("Testing mod with %ld on the left\n", a);
		for (long b : b_values) {
			if (b == 0) continue; // Avoid divide by zero
			if (testMod(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (long a = -10; a <= 10; a++) {
		printf("Testing pow with %ld on the left\n", a);
		for (long b = 1; b <= 9; b++) {
			if (testPow(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (long a : a_values) {
		printf("Testing gcd with %ld on the left\n", a);
		for (long b : b_values) {
			if (testGcd(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (long a : a_values) {
		printf("Testing lcm with %ld on the left\n", a);
		for (long b : b_values) {
			if (testLcm(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (long a = 0; a <= A_END; a += A_INC) {
		printf("Testing sqrt with %ld\n", a);
		if (testSqrt(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	for (long a = 0; a <= 100; a++) {
		printf("Testing isPrime with %ld\n", a);
		if (testIsPrime(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printf("Testing factorials: 1 to 13\n");
	for (long a = 1; a < 13; a++) {
		if (testFactorial(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printTestResults("integerTest", nSuccess, nFail, startMs);
	return nFail == 0;
}
