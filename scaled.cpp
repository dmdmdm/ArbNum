// Operations with scaled numbers
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include <string.h>
#include <cmath>
#include <climits>
#include <algorithm> // For std::sort()

#if __cplusplus >= 202002L
#include <format>
#endif

#include "util.h"
#include "scaled.h"

//------------------------------------------------------------------------------
// Control simplifying at construction and after some operations

typedef enum { S_KEEP, S_SIMP } Scaled_Simplify;
static Scaled_Simplify s_Simplify = S_SIMP;

void Scaled_setSimplify(const Scaled_Simplify s) {
	s_Simplify = s;
}

//------------------------------------------------------------------------------
// Scaled

static const Integer s_IntegerTen(10);
static const Scaled s_ScaledZero(0);
static const Scaled s_ScaledOne(1);
static const Scaled s_ScaledTwo(2);
static const Scaled s_ScaledMinusOne(-1);
static const Scaled s_ScaledPi("3.1415926535897932384626433832795");

// Precision / epsilon

static const Scaled s_ScaledEpsilon("0.000001"); // 1e-6
// We calculate divide to 7 decimals and check to one less
static int s_iPrecision = 20;
#define CHECKING_EPSILON 1e-6

void Scaled::clear() {
	mSpecial = SPEC_NORMAL;
	mUnscaled.clear();
	mExp = 0;
}

void Scaled::saveNumber(const char *in) {
	if (strlen(in) == 0) {
		set(s_ScaledZero);
		return;
	}

#ifdef _MSC_VER
	if (stricmp(in, "pi") == 0) {
#else
	if (strcasecmp(in, "pi") == 0) {
#endif
		set(s_ScaledPi);
		return;
	}

	if (strchr(in, ':')) {
		fprintf(stderr, "Invalid colon in scaled number: %s\n", in);
		return;
	}

	// We can have:
	// 1234 - only digits
	// 1.234 - decimal and digits
	// 1234e-4 - scientific notation with no decimal
	// 1.234e+4 - scientific notation with decimal
	
	char buf[ARBNUM_BUF_SIZE];
#ifdef _MSC_VER
	lstrcpyn(buf, in, sizeof(buf));
#else
	strlcpy(buf, in, sizeof(buf));
#endif
	char *p;

	// Has an "e"?
	char *beforeE, *afterE;
	if ((p = strchr(buf, 'e')) != nullptr) {
		// Scientific notation
		afterE = p + 1;
		*p = '\0';
		beforeE = buf;
	}
	else { // No e
		afterE = terminatingNul(buf);
		beforeE = buf;
	}

	// Has a decimal?
	if ((p = strchr(beforeE, '.')) != nullptr) {
		const int afterDecimal = strlen(p + 1);
		shuffle(p, p + 1); // Delete the decimal
		mUnscaled.saveNumber(buf);
		mExp = afterDecimal;
	}
	else {
		mUnscaled.saveNumber(buf);
		mExp = 0;
	}

	if (hasContent(afterE)) {
		const int exp = atoi(afterE); // Needs to deal with +/ at start
		if (exp > 0) {
			mUnscaled *= (int) std::pow(10, exp);
		}
		else {
			mExp -= exp;  // Subtracting a negative adds to mExp
		}
	}

	simplify();
}

void Scaled::setSign(const Scaled &a) {
	mUnscaled.setSign(a.mUnscaled);
}

void Scaled::appendRight(const int digit, const int numberOfDigits) {
	mUnscaled.appendRight(digit, numberOfDigits);
}

Scaled::Scaled(const Integer &unscaled, const Integer &exp) {
	mUnscaled = unscaled;
	mExp = exp.toInt();

	simplify();
}

Scaled::Scaled(const Integer &a) {
	mUnscaled = a;
	mExp = 0;
}

Scaled::Scaled(const double in) {
#if __cplusplus >= 202002L
	saveNumber(std::format("{:.17g}", in).c_str());
#else
	char buf[ARBNUM_BUF_SIZE];
	snprintf(buf, sizeof(buf), "%.17g", in);
	saveNumber(buf);
#endif
}

Scaled::Scaled(const int i) {
	mUnscaled = i;
	mExp = 0;
}

Scaled::Scaled(const long l) {
	mUnscaled = l;
	mExp = 0;
}

Scaled::Scaled(const long long ll) {
	mUnscaled = ll;
	mExp = 0;
}

bool Scaled::isZero() const {
	return mUnscaled.isZero();
}

bool Scaled::isCloseToZero() const {
	return Scaled::abs(*this) < s_ScaledEpsilon;
}

bool Scaled::isNegative() const {
	return mUnscaled.isNegative();
}

bool Scaled::isPositive() const {
	return mUnscaled.isPositive();
}

bool Scaled::isInteger() const {
	// 10 ^ -0 == 1
#ifdef USE_INTEGER_FOR_EXP
	return mExp.isZero();
#else
	return mExp == 0;
#endif
}

Integer Scaled::getDenom() const {
	return Integer::pow(s_IntegerTen, mExp);
	// Not minus mExp because this is the denominator
}

bool Scaled::isOdd() const {
	return mUnscaled.isOdd();
}

bool Scaled::isEven() const {
	return mUnscaled.isEven();
}

void Scaled::simplify() {
	if (s_Simplify == S_KEEP) return;
	if (isInteger()) return;
#ifdef USE_INTEGER_FOR_EXP
	mExp -= mUnscaled.trimTrailing(mExp.toInt());
#else
	mExp -= mUnscaled.trimTrailing(mExp);
#endif
}

std::string Scaled::toString() const {
	if (isInteger()) {
		return mUnscaled.toString();
	}
	else {
#ifdef USE_INTEGER_FOR_EXP
		const int digits = mExp.toInt();
#else
		const int digits = mExp;
#endif
		if (digits < 0) return "negative-exp";
		std::string str = mUnscaled.toString();

		while ((int)str.length() < digits) {
			str = '0' + str;
		}

		str.insert(str.length() - digits, 1, '.');
		return str;
	}
}

double Scaled::toDouble() const {
#ifdef USE_INTEGER_FOR_EXP
	return mUnscaled.toDouble() * std::pow(10.0, - mExp.toDouble());
#else
	return mUnscaled.toDouble() * std::pow(10.0, - mExp);
#endif
}

Integer Scaled::toInteger() const {
	if (isInteger()) return mUnscaled;
	return mUnscaled / getDenom();
}

Unsigned Scaled::toUnsigned() const {
	return toInteger().toUnsigned();
}

int Scaled::toInt() const {
	return toInteger().toInt();
}

Scaled Scaled::abs(const Scaled &a) {
	Scaled out(a);
	out.mUnscaled.mkPositive();
	return out;
}

Scaled Scaled::sign(const Scaled &a) {
	return a.isNegative() ? s_ScaledMinusOne : s_ScaledOne;
}

int Scaled::compare(const Scaled &aIn, const Scaled &bIn) {
	Scaled a(aIn);
	Scaled b(bIn);

	convertToSameExponent(a, b);
	return Integer::compare(a.mUnscaled, b.mUnscaled);
}

void Scaled::convertToSameExponent(Scaled &a, Scaled &b) {
	if (a.mExp == b.mExp) {
		return;
	}

	const Integer aDenom = a.getDenom();
	const Integer bDenom = b.getDenom();

	a.mUnscaled *= bDenom;
	a.mExp += b.mExp;

	b.mUnscaled *= aDenom;
	b.mExp += a.mExp;
}

Scaled Scaled::add(const Scaled &aIn, const Scaled &bIn) {
	Scaled a(aIn);
	Scaled b(bIn);

	convertToSameExponent(a, b);

	Scaled out;
	out.mUnscaled = a.mUnscaled + b.mUnscaled;
	out.mExp = a.mExp;
	out.simplify();
	return out;
}

Scaled Scaled::subtract(const Scaled &aIn, const Scaled &bIn) {
	Scaled a(aIn);
	Scaled b(bIn);

	convertToSameExponent(a, b);

	Scaled out;
	out.mUnscaled = a.mUnscaled - b.mUnscaled;
	out.mExp = a.mExp;
	out.simplify();

	return out;
}

Scaled Scaled::multiply(const Scaled &a, const Scaled &b) {
	Scaled out;

	out.mUnscaled = a.mUnscaled * b.mUnscaled;
	out.mExp = a.mExp + b.mExp;
	out.simplify();

	return out;
}

Scaled Scaled::halfInt(const Scaled &a) {
	return Integer::half(a.toInteger());
}

Scaled Scaled::half(const Scaled &a) {
	return Scaled::divide(a, s_ScaledTwo);
}

#define DIVIDE_WITH_UNSIGNED

Scaled Scaled::divide(const Scaled &dividendIn, const Scaled &divisorIn) {
	Scaled out;

    // Handle division by zero
    if (divisorIn.isZero()) {
		out.mkError();
        ArbNum_log("Division by zero.  dividend=%s\n", dividendIn.toString().c_str());
		return out;
	}

    // Determine the sign of the result
	bool isNegative = false;
    if ((dividendIn.isNegative() && divisorIn.isPositive()) || (dividendIn.isPositive() && divisorIn.isNegative())) {
		isNegative = true;
	}

    // Make both numbers positive for easier calculation
	Scaled a1 = Scaled::abs(dividendIn);
	Scaled b1 = Scaled::abs(divisorIn);

    // Scale both numbers to whole integers by finding decimal places
#ifdef USE_INTEGER_FOR_EXP
	const int places = std::max(a1.mExp.toInt(), b1.mExp.toInt());
#else
	const int places = std::max(a1.mExp, b1.mExp);
#endif
	if (places > 0) {
		// Using int pow() reduces the possible precision a bit.
		// But its faster and we don't want to rely on the higher level Scaled::pow() here in Scaled::divide()
		a1 *= std::pow(10, places);
		b1 *= std::pow(10, places);
	}

#ifdef DIVIDE_WITH_UNSIGNED
	const Unsigned a = a1.toUnsigned();
	const Unsigned b = b1.toUnsigned();
    
    // Integer division for the whole number part
	const UnsignedDivide div0 = Unsigned::divideWithRem(a, b);
	std::string resultString = div0.quotient.toString() + ".";
	Unsigned remainder = div0.remainder;
    
    // Compute decimal places up to the requested precision
	for (int i = 1; i <= s_iPrecision; i++) {
		remainder.appendRight(0, 1); // remainder *= 10;
		const UnsignedDivide div1 = Unsigned::divideWithRem(remainder, b);
		resultString += div1.quotient.firstDigitAsChar();
		remainder = div1.remainder;
		if (remainder == 0) break;
	}
#else
	const Integer a = a1.toInteger();
	const Integer b = b1.toInteger();
    
    // Integer division for the whole number part
	const IntegerDivide div0 = Integer::divideWithRem(a, b);
	std::string resultString = div0.quotient.toString() + ".";
	Integer remainder = div0.remainder;
    
    // Compute decimal places up to the requested precision
	for (int i = 1; i <= s_iPrecision; i++) {
		remainder.appendRight(0, 1); // remainder *= 10;
		const IntegerDivide div1 = Integer::divideWithRem(remainder, b);
		resultString += div1.quotient.firstDigitAsChar();
		remainder = div1.remainder;
		if (remainder == 0) break;
	}
#endif

    // Apply the correct sign
	if (isNegative) {
		resultString = "-" + resultString;
	}

	out.saveNumber(resultString.c_str());
	return out;
}

Scaled Scaled::mod(const Scaled &a, const Scaled &b) {
	if (b.isZero()) {
		ArbNum_log("mod: Cannot divide by zero\n");
		Scaled out;
		out.mkError();
		return out;
	}

	return a - b * Scaled::floor(a / b);
}

Scaled Scaled::min(const Scaled &a, const Scaled &b) {
	return a < b ? a : b;
}

Scaled Scaled::max(const Scaled &a, const Scaled &b) {
	return a > b ? a : b;
}

Scaled Scaled::doNot(const Scaled &a) {
	return a.isZero() ? s_ScaledOne : s_ScaledZero;
}

Scaled Scaled::floor(const Scaled &a) {
	return a.toInteger();
}

Scaled Scaled::pow(const Scaled &a, const Scaled &exp) {
	if (exp.isZero()) return s_ScaledOne;

	if (exp.isInteger()) {
		const Scaled x = pow(a, halfInt(exp)); // Recursive

		if (exp.isEven()) {
			return x * x;
		}
		else {
			return a * x * x;
		}
	}
	else {
		Scaled result = 1;
		for (Scaled i = 0; i < exp; i++) {
			result *= a;
		}
		return result;
	}
}

Scaled Scaled::gcd(const Scaled &a, const Scaled &b) {
	if (a.isNegative() || b.isNegative()) {
		return gcd(Scaled::abs(a), Scaled::abs(b));
	}
	if (a.isCloseToZero()) return b;
	return gcd(b % a, a); // Recursive
}

Scaled Scaled::lcm(const Scaled &a, const Scaled &b) {
	const Scaled denom = gcd(a, b);
	if (denom.isCloseToZero()) return denom;
	return (a * b) / denom;
}

Scaled Scaled::factorial(const Scaled &count) {
	if (count <= s_ScaledOne) return s_ScaledOne;
	return factorial(count - s_ScaledOne) * count;
}

// Newton's method
Scaled Scaled::sqrt(const Scaled &a) {
	if (a.isNegative()) {
		ArbNum_log("sqrt: Attempt to take the square root of a negative number\n");
		Scaled result;
		result.mkError();
		return result;
	}

	Scaled x0 = half(a);

	if (x0.isCloseToZero()) return a;

	Scaled x1 = half(x0 + a / x0);

	while (x1 < x0) {
		x0 = x1;
		x1 = half(x0 + a / x0);
	}

	return x0;
}

Scaled Scaled::random(const Scaled &wantedDigits) {
	return Integer::random(wantedDigits.toInteger());
}

Scaled Scaled::isPrime(const Scaled &a) {
	if (!a.isInteger()) {
		ArbNum_log("isPrime: Only doing isPrime() on the integer part of the number\n");
		// Keep going
	}

	return Integer::isPrime(a.toInteger());
}

void Scaled::set(const Scaled &other) {
	mUnscaled = other.mUnscaled;
	mExp = other.mExp;
}

void Scaled::add(const Scaled &other) {
	set(add(*this, other));
}

void Scaled::subtract(const Scaled &other) {
	set(subtract(*this, other));
}

void Scaled::multiply(const Scaled &other) {
	set(multiply(*this, other));
}

void Scaled::divide(const Scaled &other) {
	set(divide(*this, other));
}

void Scaled::mod(const Scaled &other) {
	set(mod(*this, other));
}

void Scaled::pow(const Scaled &other) {
	set(pow(*this, other));
}

Scaled Scaled::operator++(int) {
	add(s_ScaledOne);
	return *this;
}

Scaled Scaled::operator--(int) {
	subtract(s_ScaledOne);
	return *this;
}

//------------------------------------------------------------------------------
// Testing

// For comparing doubles
static bool eq(const double a, const double b, const double epsilon = CHECKING_EPSILON) {
	return std::abs(a - b) < epsilon;
}

static bool ne(const double a, const double b, const double epsilon = CHECKING_EPSILON) {
	return ! eq(a, b, epsilon);
}

static bool eqz(const double a, const double epsilon = CHECKING_EPSILON) {
	return eq(a, 0.0, epsilon);
}

bool Scaled::testStoreStr(const char *in, const char *correct) {
	const Scaled scaled(in);
	const std::string result = scaled.toString();
	if (strcmp(result.c_str(), correct) != 0) {
		fprintf(stderr, "Store Str %s = %s (Scaled) != %s (string) fail\n", in, result.c_str(), correct);
	}
	return strcmp(result.c_str(), correct) == 0;
}

bool Scaled::testStoreDouble(const double dCorrect) {
	const Scaled scaled(dCorrect);
	const double dResult = scaled.toDouble();
	if (ne(dResult, dCorrect)) {
		fprintf(stderr, "Store Double %g = %g (Scaled) != %g (double) fail\n", dCorrect, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

bool Scaled::testStoreLong(const long lCorrect) {
	const Scaled scaled(lCorrect);
	const long lResult = atol(scaled.toString().c_str());
	if (lResult != lCorrect) {
		fprintf(stderr, "Store Long %ld = %ld (Scaled) != %ld (long) fail\n", lCorrect, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool Scaled::testStoreLongLong(const long long lCorrect) {
	const Scaled scaled(lCorrect);
	const long long lResult = atoll(scaled.toString().c_str());
	if (lResult != lCorrect) {
		fprintf(stderr, "Store LongLong %lld = %lld (Scaled) != %lld (long long) fail\n", lCorrect, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

int Scaled::compareDouble(const double a, const double b) {
	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

bool Scaled::testCompare(const double a, const double b) {
	const double dResult = Scaled::compare(a, b);
	const double dCorrect = compareDouble(a, b);
	if (ne(dResult, dCorrect)) {
		fprintf(stderr, "%g <> %g = %g (Scaled) != %g (long) fail\n", a, b, dResult, dCorrect);
		exit(0);
	}
	return eq(dResult, dCorrect);
}

bool Scaled::testAdd(const double a, const double b) {
	const double dResult = Scaled::add(a, b).toDouble();
	const double dCorrect = a + b;
	if (ne(dResult, dCorrect)) {
		fprintf(stderr, "%g + %g = %g (Scaled) != %g (double) fail\n", a, b, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

bool Scaled::testSubtract(const double a, const double b) {
	const double dResult = Scaled::subtract(a, b).toDouble();
	const double dCorrect = a - b;
	if (ne(dResult, dCorrect)) {
		fprintf(stderr, "%g - %g = %g (Scaled) != %g (double) fail\n", a, b, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

bool Scaled::testMultiply(const double a, const double b) {
	const double dResult = Scaled::multiply(a, b).toDouble();
	const double dCorrect = a * b;
	if (ne(dResult, dCorrect)) {
		fprintf(stderr, "%g * %g = %g (Scaled) != %g (double) fail\n", a, b, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

bool Scaled::testDivide(const double a, const double b) {
	const double dResult = Scaled::divide(a, b).toDouble();
	const double dCorrect = a / b;
	if (ne(dResult, dCorrect)) {
		fprintf(stderr, "%g / %g = %g (Scaled) != %g (double) fail\n", a, b, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

bool Scaled::testMod(const double a, const double b) {
	const double dResult = Scaled::mod(a, b).toDouble();
	const double dCorrect = std::fmod(a, b);
	if (ne(dResult, dCorrect)) {
		fprintf(stderr, "mod(%.17g, %.17g) = %.17g (Scaled) != %.17g (double) fail\n", a, b, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

bool Scaled::testPow(const double a, const double b) {
	const double dResult = Scaled::pow(a, b).toDouble();
	const double dCorrect = std::pow(a, b);
	if (ne(dResult, dCorrect)) {
		fprintf(stderr, "pow(%g, %g) = %g (Scaled) != %g (double) fail\n", a, b, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

double Scaled::gcdDouble(const double a, const double b) {
	if (a < 0 || b < 0) {
		return gcdDouble(std::abs(a), std::abs(b));
	}
	if (eqz(a)) return b;
	return gcdDouble(std::fmod(b, a), a);
}

bool Scaled::testGcd(const double a, const double b) {
	const double dResult = Scaled::gcd(a, b).toDouble();
	const double dCorrect = gcdDouble(a, b);
	if (ne(dResult, dCorrect)) {
		fprintf(stderr, "gcd(%g, %g) = %g (Scaled) != %g (double) fail\n", a, b, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

double Scaled::lcmDouble(const double a, const double b) {
	const double denom = gcdDouble(a, b);
	if (eqz(denom)) return denom;
	return (a * b) / denom;
}

bool Scaled::testLcm(const double a, const double b) {
	const double dResult = Scaled::lcm(a, b).toDouble();
	const double dCorrect = lcmDouble(a, b);

	// If it has a large number of decimal places we have to allow more difference with the double result
	double epsilon = 1e-6;
	if (std::to_string(a).length() > 6) epsilon = .1;

	if (ne(dResult, dCorrect, epsilon)) {
		fprintf(stderr, "lcm(%.17g, %.17g) = %.17g (Scaled) != %.17g (double) fail\n", a, b, dResult, dCorrect);
	}
	return eq(dResult, dCorrect, epsilon);
}

double Scaled::sqrtDouble(const double s) {
	if (s < 0) {
		return sqrtDouble(std::abs(s));
	}
	return std::sqrt(s);
}

bool Scaled::testSqrt(const double a) {
	const double dResult = Scaled::sqrt(a).toDouble();
	const double dCorrect = sqrtDouble(a);
	if (ne(dResult,dCorrect)) {
		fprintf(stderr, "sqrt(%g) = %g (Scaled) != %g (double) fail\n", a, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

// By Soma Mbadiwe on https://stackoverflow.com/questions/15743192/check-if-number-is-prime-number
double Scaled::isPrimeDouble(const double a) {
	if (a <= 1) return false;
	if (eq(a,2) || eq(a,3) || eq(a,5)) return true;
	if (eqz(std::fmod(a, 2)) || eqz(std::fmod(a, 3)) || eqz(std::fmod(a, 5))) return false;

	const double boundary = std::sqrt(a);
	for (double i = 6; i <= boundary; i += 6) {
		if (eqz(std::fmod(a, (i + 1))) || eqz(a,(i + 5))) {
			return false;
		}
	}

	return true;
}

bool Scaled::testIsPrime(const double a) {
	const double dResult = Scaled::isPrime(a).toDouble();
	const double dCorrect = isPrimeDouble(a);
	if (ne(dResult, dCorrect)) {
		fprintf(stderr, "isPrime(%g) = %g (Scaled) != %g (double) fail\n", a, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

double Scaled::factorialDouble(const double n) {
	if (n <= 1) return 1;
	return factorialDouble(n - 1) * n;
}

bool Scaled::testFactorial(const double a) {
	const double dResult = Scaled::factorial(a).toDouble();
	const double dCorrect = factorialDouble(a);
	if (ne(dResult, dCorrect)) {
		fprintf(stderr, "%g! = %g (Scaled) != %g (double) fail\n", a, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

// We iterate using long's and divide by 10 to get a double which we save.
// This is rather than interating thru double which can get weird values.
void Scaled::makeTestValues(const long start, const long end, const long inc, std::vector<double> &values) {
	values.clear();

	if (inc > 0) { // Counting up
		for (long n = start; n <= end; n += inc) {
			const double d = (double)n / 10.0;
			values.push_back(d);
		}

		// We want extra tests around zero
		for (long n = -10; n <= 10; n++) {
			values.push_back(n);
		}

		std::sort(values.begin(), values.end());
		(void) std::unique(values.begin(), values.end());
	} else { // Counting down
		for (long n = start; n >= end; n += inc) {
			const double d = (double)n / 10.0;
			values.push_back(d);
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

#undef ONLY_TEST_DIVIDE

bool Scaled::testAll() {
	int nSuccess = 0;
	int nFail = 0;
	const long start = currentMillis();

#ifndef ONLY_TEST_DIVIDE
	ArbNum_setLogging(AN_QUIET); // Don't log about invalid numbers
	const char *checkStr[] =       { "1e+4",  "1e-4",  "",  "0000", "0004", "4000", "hello", "-25" };
	const char *checkStrExpect[] = { "10000", ".0001", "0", "0",       "4", "4000",     "0", "-25" };
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

	const double checkDouble[] = { 9.3, 9.000001, -4, 0.0003 };
	const int nCheckDouble = sizeof(checkDouble) / sizeof(checkDouble[0]);
	for (int i = 0; i < nCheckDouble; i++) {
		printf("Testing store double %g\n", checkDouble[i]);
		if (testStoreDouble(checkDouble[i])) {
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
#endif

	const long A_START = -10000;
	const long A_END = 10000;
	long A_INC = 77;
	const long B_START = 10000;
	const long B_END = -10000;
	long B_INC = -88;

	// Go faster
	A_INC *= 9;
	B_INC *= 9;

	// Make vectors of test numbers
	std::vector<double> a_values, b_values;
	makeTestValues(A_START, A_END, A_INC, a_values);
	makeTestValues(B_START, B_END, B_INC, b_values);
	printf("Number of tests per function=%ld\n", a_values.size() * b_values.size());

#ifndef ONLY_TEST_DIVIDE
	for (double a : a_values) {
		printf("Testing compare with %g on the left\n", a);
		for (double b : b_values) {
			if (testCompare(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		 }
	}

	for (double a : a_values) {
		printf("Testing add with %g on the left\n", a);
		for (double b : b_values) {
			if (testAdd(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (double a : a_values) {
		printf("Testing subtract with %g on the left\n", a);
		for (double b : b_values) {
			if (testSubtract(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (double a : a_values) {
		printf("Testing multiply with %g on the left\n", a);
		for (double b : b_values) {
			if (testMultiply(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}
#endif

	for (double a : a_values) {
		printf("Testing divide with %g on the left\n", a);
		for (double b : b_values) {
			if (b == 0) continue; // Avoid divide by zero
			if (testDivide(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

#ifndef ONLY_TEST_DIVIDE
	for (double a : a_values) {
		printf("Testing mod with %g on the left\n", a);
		for (double b : b_values) {
			if (b == 0) continue; // Avoid divide by zero
			if (testMod(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (double a = -10; a <= 10; a++) {
		printf("Testing pow with %g on the left\n", a);
		for (double b = 1; b <= 9; b++) {
			if (testPow(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (double a : a_values) {
		printf("Testing gcd with %g on the left\n", a);
		for (double b : b_values) {
			if (testGcd(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	for (double a : a_values) {
		printf("Testing lcm with %g on the left\n", a);
		for (double b : b_values) {
			if (testLcm(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
		if (nFail > 50) break;
	}

	for (double a = 0; a <= A_END; a += A_INC) {
		printf("Testing sqrt with %g\n", a);
		if (testSqrt(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	for (double a = 0; a <= 100; a++) {
		printf("Testing isPrime with %g\n", a);
		if (testIsPrime(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printf("Testing factorials: 1 to 13\n");
	for (double a = 1; a < 13; a++) {
		if (testFactorial(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}
#endif

	const long durationMs = currentMillis() - start;
	const double durationSeconds = durationMs / 1000.0;
	printf("Success: %d  Fail: %d  Took: %.1f seconds\n", nSuccess, nFail, durationSeconds);
	return nFail == 0;
}
