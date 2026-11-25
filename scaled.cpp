// Operations with scaled numbers
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include <string.h>
#include <cmath>
#include <climits>
#include <map>
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

INLINE const Integer s_IntegerTen = 10;
INLINE const Scaled s_ScaledZero = 0;
INLINE const Scaled s_ScaledPointFive = ".5";
INLINE const Scaled s_ScaledOne = 1;
INLINE const Scaled s_ScaledTwo = 2;
INLINE const Scaled s_ScaledFour = 4;
INLINE const Scaled s_ScaledMinusOne = -1;
INLINE const Scaled s_ScaledPi = "3.1415926535897932384626433832795"; // π
INLINE const Scaled s_ScaledE = "2.718281828459045235360287471352"; // e
INLINE const Scaled s_ScaledPi2 = s_ScaledPi * 2; //  2π
INLINE const Scaled s_ScaledMinusPi2 = s_ScaledPi * -2; // -2π
INLINE const Scaled s_ScaledLn2 = "0.69314718055994530941723212145818"; // ln(2)
INLINE const Scaled s_ScaledLn10 = "2.3025850929940456840179914546844"; // ln(10)

static Scaled quickError() {
	Scaled out;
	out.mkError();
	return out;
}

// Precision and epsilon

INLINE Scaled s_ScaledEpsilon("1e-7");
INLINE long s_lDividePrecision = 7 * 3;
INLINE double s_dCheckingEpsilon = 1e-6;

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

	// We can have:
	// 1234 - only digits
	// 1.234 - decimal and digits
	// -1.234 - minus, decimal and digits
	// 1234e-4 - scientific notation with no decimal
	// 1.234e+4 - scientific notation with decimal
	
	char buf[ARBNUM_BUF_SIZE];
	portable_strlcpy(buf, in, sizeof(buf));
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
		const long afterDecimal = (long)strlen(p + 1);
		shuffle(p, p + 1); // Delete the decimal
		mUnscaled.saveNumber(buf);
		mExp = afterDecimal;
	}
	else {
		mUnscaled.saveNumber(buf);
		mExp = 0;
	}

	if (hasContent(afterE)) {
		const long exp = atoi(afterE); // Needs to deal with +/ at start
		if (exp > 0) {
			mUnscaled.multiplyByTen(exp);
		}
		else {
			mExp -= exp;  // Subtracting a negative adds to mExp
		}
	}

	simplify();
}

Scaled Scaled::getPi() {
	Scaled scaled = s_ScaledPi;
	scaled.trimToPrecision();
	return scaled;
}

Scaled Scaled::getE() {
	Scaled scaled = s_ScaledE;
	scaled.trimToPrecision();
	return scaled;
}

void Scaled::setPrecisionGlobal(const long n) {
	s_ScaledEpsilon.set(1, n);
	s_lDividePrecision = n * 3;
	s_dCheckingEpsilon = s_ScaledEpsilon.toDouble() * 10; // 1 digit less accurate
}

long Scaled::getPrecisionGlobal() {
	return s_ScaledEpsilon.mExp;
}

void Scaled::copySign(const Scaled &a) {
	mUnscaled.copySign(a.mUnscaled);
}

void Scaled::trimToPrecision() {
	const long prec = getPrecisionGlobal();
	if (mExp < prec) return;
	const long n = mExp - prec;
	const long nRemoved = mUnscaled.removeAtEnd(n);
	mExp -= nRemoved;
}

Scaled::Scaled(const long &unscaled, const long &exp) {
	mUnscaled = unscaled;
	mExp = exp;

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

Scaled::Scaled(const long double in) {
#if __cplusplus >= 202002L
	saveNumber(std::format("{:.34Lg}", in).c_str());
#else
	char buf[ARBNUM_BUF_SIZE];
	snprintf(buf, sizeof(buf), "%.34Lg", in);
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
	return mUnscaled.isZero() && mExp == 0;
}

bool Scaled::isOne() const {
	return mUnscaled.isOne() && mExp == 0;
}

bool Scaled::isCloseToZero() const {
	return Scaled::abs(*this) < s_ScaledEpsilon;
}

bool Scaled::isNegative() const {
	return mUnscaled.isNegative();
}

bool Scaled::isNegativeOrZero() const {
	return mUnscaled.isNegativeOrZero();
}

bool Scaled::isPositive() const {
	return mUnscaled.isPositive();
}

bool Scaled::isInteger() const {
	// 10 ^ -0 == 1
	return mExp == 0;
}

bool Scaled::isOdd() const {
	return mUnscaled.isOdd();
}

bool Scaled::isEven() const {
	return mUnscaled.isEven();
}

void Scaled::multiplyByTen() {
	if (mExp > 0) {
		mExp--;
	}
	else {
		shiftLeft(1);
	}
}

void Scaled::multiplyByTen(const long exp) {
	for (long i = 0; i < exp; i++) {
		multiplyByTen();
	}
}

void Scaled::simplify() {
	if (s_Simplify == S_KEEP) return;
	if (isInteger()) return;
	mExp -= mUnscaled.trimTrailing(mExp);
}

std::string Scaled::toString() const {
	if (isInteger()) {
		return mUnscaled.toString();
	}
	else {
		const long digits = mExp;
		if (digits < 0) return "negative-exp";
		std::string str = mUnscaled.toStringNoSign();

		// Ensure enough leading zeros
		const long zerosNeeded = digits - (long)str.length();
		if (zerosNeeded > 0) {
			str.insert(str.begin(), (size_t)zerosNeeded, '0');
		}

		// Insert decimal point
		str.insert(str.length() - (size_t)digits, 1, '.');

		if (mUnscaled.isNegative()) {
			str = '-' + str;
		}

		return str;
	}
}

double Scaled::toDouble() const {
	// We need to use std::pow() here (and not powLong()) because its a negative exponent
	return mUnscaled.toDouble() * std::pow(10.0, - mExp);
}

long double Scaled::toLongDouble() const {
	// Some platforms don't handle this
	return mUnscaled.toLongDouble() * std::pow(10.L, (long double) - mExp);
}

Integer Scaled::toInteger() const {
	if (mExp >= mUnscaled.length()) return 0; // Smaller than zero
	Integer out(mUnscaled);
	out.shiftRight(mExp);
	return out;
}

std::string Scaled::toDump() const {
	switch(mSpecial) {
	case SPEC_NORMAL:
		{
		const std::string strExp = mExp == 0 ? "0" : "-" + std::to_string(mExp);
		return "{unscaled=" + mUnscaled.toString() + ", exp=" + strExp + "}";
		}
		break;
	case SPEC_ERROR:
		return "error";
		break;
	case SPEC_IGNORE:
		return "ignore";
		break;
	}
	return "unknown";
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

	const long aExp = a.mExp;
	const long bExp = b.mExp;

	a.shiftLeft(bExp);
	a.mExp += bExp;

	b.shiftLeft(aExp);
	b.mExp += aExp;
}

// ie Convert to a whole number
void Scaled::convertToZeroExponent(Scaled &a, Scaled &b) {
	const long places = std::max(a.mExp, b.mExp);
	if (places > 0) {
		a.multiplyByTen(places);
		b.multiplyByTen(places);
	}
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
	return Scaled::multiply(a, s_ScaledPointFive);
}

Scaled Scaled::divide(const Scaled &dividendIn, const Scaled &divisorIn) {
	Scaled out;

	// Handle division by zero
	if (divisorIn.isZero()) {
		ArbNum_log("Division by zero.  dividend=%s\n", dividendIn.toString().c_str());
		return quickError();
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
	convertToZeroExponent(a1, b1);

	const Unsigned a = a1.toUnsigned();
	const Unsigned b = b1.toUnsigned();

	// Integer division for the whole number part
	const UnsignedDivide div0 = Unsigned::divideWithRem(a, b);
	std::string outString = div0.quotient.toString() + ".";
	Unsigned remainder = div0.remainder;

	// Compute decimal places up to the requested precision
	for (long i = 1; i <= s_lDividePrecision; i++) {
		remainder.multiplyByTen();
		const UnsignedDivide div1 = Unsigned::divideWithRem(remainder, b);
		outString += div1.quotient.firstDigitAsChar();
		remainder = div1.remainder;
		if (remainder.isZero()) break;
	}

	out.saveNumber(outString.c_str());

	if (isNegative) {
		out.mkNegative();
	}

	return out;
}

Scaled Scaled::mod(const Scaled &a, const Scaled &b) {
	if (b.isZero()) {
		ArbNum_log("mod: Cannot divide by zero\n");
		return quickError();
	}

	return a - b * Scaled::trunc(a / b);
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

Scaled Scaled::floor(const Scaled &x) {
	Scaled t = Scaled::trunc(x); // Integer part, toward zero
	const Scaled frac = x - t; // Fractional part, same sign as x 

	if (frac > 0) { /* do nothing */ }
	else if (frac < 0) { t--; }

	return t;
}

Scaled Scaled::ceil(const Scaled &x) {
	Scaled t = Scaled::trunc(x); // Integer part, toward zero
	const Scaled frac = x - t; // Fractional part, same sign as x 

	if (frac > 0) { t++; }
	else if (frac < 0) { /* do nothing */ }

	return t;
}

Scaled Scaled::round(const Scaled &x) {
	Scaled t = Scaled::trunc(x); // Integer part, toward zero
	const Scaled frac = x - t; // Fractional part, same sign as x 

	if (frac >= 0.5) { t++; }
	else if (frac <= -0.5) { t--; }

	return t;
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

static std::map<Scaled,Scaled> s_FactorialCache;
static long s_FactorialCacheHitCount = 0;

Scaled Scaled::factorial(const Scaled &n) {
	if (n <= s_ScaledOne) return s_ScaledOne;

	auto it = s_FactorialCache.find(n);
	if (it != s_FactorialCache.end()) {
		s_FactorialCacheHitCount++;
		return it->second;
	}

	const Scaled out = factorial(n - s_ScaledOne) * n;

	s_FactorialCache[n] = out;
	return out;
}

// Newton's method
Scaled Scaled::sqrt(const Scaled &a) {
	if (a.isNegative()) {
		ArbNum_log("Attempt to take the square root of a negative number\n");
		return quickError();
	}
	if (a.isOne()) return a;

	Scaled x0 = half(a);
	if (x0.isCloseToZero()) return a;
	Scaled x1 = half(x0 + a / x0);

	for (;;) {
		x0 = x1;
		x1 = half(x0 + a / x0);
		Scaled diff = x1 - x0;
		if (diff.isCloseToZero()) break;
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

// In radians, Taylor series
// https://en.wikipedia.org/wiki/Taylor_series
Scaled Scaled::sin(const Scaled &xIn) {
	Scaled x(xIn);
	if (x > s_ScaledPi2) {
		x %= s_ScaledPi2;
	}

	Scaled total, chunk, sign(1), term;
	for (long n = 0;; n++) {
		chunk = 2 * n + 1;
		term = powInt(x, chunk) / factorial(chunk);
		total += sign * term;
		if (term.isCloseToZero()) break;
		sign.flipSign();
	}
	return total;
}

// In radians, Tayor series
Scaled Scaled::cos(const Scaled &xIn) {
	Scaled x(xIn);
	if (x > s_ScaledPi2) {
		x %= s_ScaledPi2;
	}

	Scaled total, chunk, sign(1), term;
	for (long n = 0;; n++) {
		chunk = 2 * n;
		term = powInt(x, chunk) / factorial(chunk);
		total += sign * term;
		if (term.isCloseToZero()) break;
		sign.flipSign();
	}
	return total;
}

// In radians.
// Slower than other trig functions because does sin() and cos()
Scaled Scaled::tan(const Scaled &xIn) {
	Scaled x(xIn);

	// Range reduction to [-pi/2, pi/2]
	x -= s_ScaledPi * Scaled::round(x / s_ScaledPi);

	// Handle sign (tan is odd)
	int sign = 1;
	if (x.isNegative()) {
		x.flipSign(); // x = -x
		sign = -1;
	}

	static Scaled PiDiv2, PiDiv4;
	if (PiDiv2.isZero()) {
		PiDiv2 = s_ScaledPi / 2;
		PiDiv4 = s_ScaledPi / 4;
	}

	// Reduce to [0, pi/4] using tan(x) = cot(pi/2 - x)
	bool invert = false;
	if (x > PiDiv4) {
		x = PiDiv2 - x;
		invert = true;
	}

	const Scaled s = sin(x);
	const Scaled c = cos(x);

	// Combine
	Scaled out;
	if (invert) {
		out = c / s;
	}
	else {
		out = s / c;
	}
	return sign * out;
}

Scaled Scaled::expTaylor(const Scaled &r) {
    // Taylor series: exp(r) = 1 + r + r²/2! + ...
    Scaled sum = 1;
    Scaled term = 1;

	const Scaled epsilon = s_ScaledEpsilon / 100; // 2 digits more accurate
	for (long n = 1;; n++) {
        term *= r / n; // term = term * r / n;
		sum += term;
		if (abs(term) < epsilon) break;
	}

	return sum;
}

// exp(a) = e ^ a.  Taylor services
Scaled Scaled::exp(const Scaled &a) {
	if (a.isError()) return a;
	if (a.isZero()) { return s_ScaledOne; }
	if (a.isOne()) { return s_ScaledE; }

    // Choose k so r = a - k*ln(2) is small.
    Scaled k = round(a / s_ScaledLn2);
    const Scaled r = a - k * s_ScaledLn2;

	/*
    // Taylor series: exp(r) = 1 + r + r²/2! + ...
    Scaled sum = 1;
    Scaled term = 1;

	const Scaled epsilon = s_ScaledEpsilon / 100; // 2 digits more accurate
	for (long n = 1;; n++) {
        term *= r / n; // term = term * r / n;
		sum += term;
		if (abs(term) < epsilon) break;
	}
	*/
	Scaled sum = expTaylor(r);

    // Since exp(a) = exp(r) * 2^k, scale by powers of 2.
    while (k > 0) {
        sum = sum * 2;
        k--; // k = k - 1
	}

    while (k < 0) {
        sum = half(sum); // sum = sum / 2
        k++; // k = k + 1
	}

    return sum;
}

// The parameters are Scaled which can hold real numbers but this function requires
// that they are integer.
Scaled Scaled::powInt(const Scaled &base, const Scaled &expon) {
	if (base.isZero()) { return s_ScaledZero; }
	if (base.isOne() || expon.isZero()) { return s_ScaledOne; }
	if (!(expon.isInteger() && expon.isPositive())) {
		ArbNum_log("powInt: expon must be a postive integer\n");
		// We could call Scaled::pow() here but would be too circular (it calls us)
		return quickError();
	}

	const Scaled x = Scaled::powInt(base, halfInt(expon)); // Recursive

	if (expon.isEven()) {
		return x * x;
	}
	else {
		return base * x * x;
	}
}

// Uses powInt(), exp(), ln()
Scaled Scaled::pow(const Scaled &base, const Scaled &expon) {
	if (base.isZero()) { return s_ScaledZero; }
	if (base.isOne() || expon.isZero()) { return s_ScaledOne; }

	if (expon.isInteger() && expon.isPositive()) {
		return powInt(base, expon);
	}

	//
	// Real values (or expon is a negative integer)
	//

    if (base.isCloseToZero()) {
        if (expon.isPositive()) {
            return s_ScaledZero;
		}
		else if (expon.isCloseToZero()) {
            return s_ScaledOne;
		}
		ArbNum_log("pow: 0 raised to a negative power\n");
		return quickError();
	}
	else if (base.isPositive()) {
        return Scaled::exp(expon * Scaled::ln(base));
	}
	else if (base.isNegative()) {
		ArbNum_log("pow: negative base is not yet supported\n");
		return quickError();
	}

	ArbNum_log("pow: unexpected flow\n");
	return quickError();
}

// Taylor (Maclaurin) series
Scaled Scaled::ln(const Scaled &x) {
	if (x.isNegativeOrZero()) {
		ArbNum_log("ln() is only defined for positive numbers.  x=%s\n", x.toString().c_str());
		return quickError();
	}

	// 
	// Range reduction
	//

	Scaled k; // k = 0
	Scaled m = x;

	while (m >= 2) {
		m = half(m); // m = m / 2;
		k++; // k = k + 1;
	}

	while (m < 1) {
		m *= 2; // m = m * 2
		k--; // k = k - 1;
	}

	// 
	// Compute
	//

	const Scaled z = (m - 1) / (m + 1);
	const Scaled zSquared = z * z;
	Scaled term = z;
	Scaled total; // total = 0
	Scaled denom = 1;
	Scaled fraction;

	/*
	while (abs(term / denom) > s_ScaledEpsilon) {
		total += term / denom; // total = total + term / denom
		term *= zSquared; // term = term * zSquared;
		denom += 2; // denom = demon + 2
	}
	*/

	for (;;) {
		fraction = term / denom;
		total += fraction; // total = total + term / denom
		// Since we have to calculate fraction to see if we're done - might as well add it then check
		if (fraction.isCloseToZero()) break;
		term *= zSquared; // term = term * zSquared;
		denom += 2; // denom = demon + 2
	}

	return 2 * total + k * s_ScaledLn2;
}

Scaled Scaled::log(const Scaled &x) {
    if (x.isNegativeOrZero()) {
		ArbNum_log("log() is only defined for positive numbers.  x=%s\n", x.toString().c_str());
		return quickError();
	}

    return Scaled::ln(x) / s_ScaledLn10;
}

void Scaled::set(const Scaled &other) {
	mSpecial = other.mSpecial;
	mUnscaled = other.mUnscaled;
	mExp = other.mExp;
}

void Scaled::set(const long unscaled, const long exp) {
	mUnscaled = unscaled;
	mExp = exp;
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

void Scaled::operator++(int) {
	add(s_ScaledOne);
}

void Scaled::operator--(int) {
	subtract(s_ScaledOne);
}

//------------------------------------------------------------------------------
// Testing

// For comparing doubles
static bool eq(const double a, const double b, double epsilon = 1) {
	if (epsilon == 1) {
		epsilon = s_dCheckingEpsilon;
	}

	return std::abs(a - b) < epsilon;
}

static bool ne(const double a, const double b, double epsilon = 1) {
	return ! eq(a, b, epsilon);
}

static bool eqz(const double a, double epsilon = 1) {
	return eq(a, 0.0, epsilon);
}

// For comparing long doubles
// non-static to avoid warning
bool EQ(const long double a, const long double b, long double epsilon = 1) {
	if (epsilon == 1) {
		epsilon = s_dCheckingEpsilon;
	}

	return std::abs(a - b) < epsilon;
}

// non-static to avoid warning
bool NE(const long double a, const long double b, long double epsilon = 1) {
	return ! EQ(a, b, epsilon);
}

// non-static to avoid warning
bool EQZ(const long double a, long double epsilon = 1) {
	return EQ(a, 0.0L, epsilon);
}

bool Scaled::testStoreStr(const char *in, const char *correct) {
	const Scaled scaled(correct);
	const std::string result = scaled.toString();
	if (ne(result.c_str(), correct)) {
		printf("Store Str %s = %s (Scaled) != %s (string) fail\n", in, result.c_str(), correct);
	}
	return eq(result.c_str(), correct);
}

bool Scaled::testStoreDouble(const double dCorrect) {
	const Scaled scaled(dCorrect);
	const double dResult = scaled.toDouble();
	if (ne(dResult, dCorrect)) {
		printf("Store Double %g = %g (Scaled) != %g (double) fail\n", dCorrect, dResult, dCorrect);
	}
	return eq(dResult, dCorrect);
}

bool Scaled::testStoreLongDouble(const long double dCorrect) {
	const Scaled scaled(dCorrect);
	const long double dResult = scaled.toLongDouble();
	if (NE(dResult, dCorrect)) {
		printf("Store Long Double %Lg = %Lg (Scaled) != %Lg (double) fail\n", dCorrect, dResult, dCorrect);
	}
	return EQ(dResult, dCorrect);
}

bool Scaled::testStoreLong(const long lCorrect) {
	const Scaled scaled(lCorrect);
	const long lResult = scaled.toLong();
	if (lResult != lCorrect) {
		printf("Store Long %ld = %ld (Scaled) != %ld (long) fail\n", lCorrect, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool Scaled::testStoreLongLong(const long long lCorrect) {
	const Scaled scaled(lCorrect);
	const long long lResult = scaled.toLongLong();
	if (lResult != lCorrect) {
		printf("Store LongLong %lld = %lld (Scaled) != %lld (long long) fail\n", lCorrect, lResult, lCorrect);
	}
	return lResult == lCorrect;
}

bool Scaled::testFloor(const double in) {
	const double result = Scaled::floor(in).toDouble();
	const double correct = std::floor(in);
	if (result != correct) {
		printf("floor %g = %g (Scaled) != %g (double) fail\n", in, result, correct);
	}
	return result == correct;
}

bool Scaled::testCeil(const double in) {
	const double result = Scaled::ceil(in).toDouble();
	const double correct = std::ceil(in);
	if (result != correct) {
		printf("ceil %g = %g (Scaled) != %g (double) fail\n", in, result, correct);
	}
	return result == correct;
}

bool Scaled::testRound(const double in) {
	const double result = Scaled::round(in).toDouble();
	const double correct = std::round(in);
	if (result != correct) {
		printf("round %g = %g (Scaled) != %g (double) fail\n", in, result, correct);
	}
	return result == correct;
}

int Scaled::compareDouble(const double a, const double b) {
	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

bool Scaled::testCompare(const double a, const double b) {
	const double result = Scaled::compare(a, b);
	const double correct = compareDouble(a, b);
	if (ne(result, correct)) {
		printf("%g <> %g = %g (Scaled) != %g (long) fail\n", a, b, result, correct);
	}
	return eq(result, correct);
}

bool Scaled::testAdd(const double a, const double b) {
	const double result = Scaled::add(a, b).toDouble();
	const double correct = a + b;
	if (ne(result, correct)) {
		printf("%g + %g = %g (Scaled) != %g (double) fail\n", a, b, result, correct);
	}
	return eq(result, correct);
}

bool Scaled::testSubtract(const double a, const double b) {
	const double result = Scaled::subtract(a, b).toDouble();
	const double correct = a - b;
	if (ne(result, correct)) {
		printf("%g - %g = %g (Scaled) != %g (double) fail\n", a, b, result, correct);
	}
	return eq(result, correct);
}

bool Scaled::testMultiply(const double a, const double b) {
	const double result = Scaled::multiply(a, b).toDouble();
	const double correct = a * b;
	if (ne(result, correct)) {
		printf("%g * %g = %g (Scaled) != %g (double) fail\n", a, b, result, correct);
	}
	return eq(result, correct);
}

bool Scaled::testDivide(const double a, const double b) {
	const double result = Scaled::divide(a, b).toDouble();
	const double correct = a / b;
	if (ne(result, correct)) {
		printf("%g / %g = %g (Scaled) != %g (double) fail\n", a, b, result, correct);
	}
	return eq(result, correct);
}

bool Scaled::testMod(const double a, const double b) {
	const double result = Scaled::mod(a, b).toDouble();
	const double correct = std::fmod(a, b);
	if (ne(result, correct)) {
		printf("mod(%.17g, %.17g) = %.17g (Scaled) != %.17g (double) fail\n", a, b, result, correct);
	}
	return eq(result, correct);
}

bool Scaled::testPiMath(const double mult, const double div) {
	const double aDouble = (s_ScaledPi * mult).toDouble();
	const double bDouble = (s_ScaledPi / div).toDouble();
	if (ne(aDouble, bDouble)) {
		printf("pi * %g = %g != pi / %g = %g fail\n", mult, aDouble, div, bDouble);
	}
	return eq(aDouble, bDouble);
}

bool Scaled::testExp(const double a) {
	const double result = Scaled::exp(a).toDouble();
	const double correct = std::exp(a);
	if (ne(result, correct)) {
		const double diff = std::abs(result - correct);
		printf("exp %g = %g (Scaled) != %g (double) diff=%.17g fail\n", a, result, correct, diff);
	}
	return eq(result, correct);
}

bool Scaled::testPow(const double base, const double expon) {
	const double result = Scaled::pow(base, expon).toDouble();
	const double correct = std::pow(base, expon);

	const double epsilon = s_dCheckingEpsilon * 10000;

	if (ne(result, correct, epsilon)) {
		const double diff = std::abs(result - correct);
		printf("pow(%g, %g) = %.17f (Scaled) != %.17f (double) diff=%.17f fail\n", base, expon, result, correct, diff);
	}
	return eq(result, correct, epsilon);
}

double Scaled::gcdDouble(const double a, const double b) {
	if (a < 0 || b < 0) {
		return gcdDouble(std::abs(a), std::abs(b)); // Recursive
	}
	if (eqz(a)) return b;
	return gcdDouble(std::fmod(b, a), a); // Recursive
}

long double Scaled::gcdLongDouble(const long double a, const long double b) {
	if (a < 0 || b < 0) {
		return gcdLongDouble(std::abs(a), std::abs(b)); // Recursive
	}
	if (EQZ(a)) return b;
	return gcdLongDouble(std::fmod(b, a), a); // Recursive
}

bool Scaled::testGcd(const double a, const double b) {
	const double result = Scaled::gcd(a, b).toDouble();
	const double correct = gcdDouble(a, b); // std::gcd() does't do double
	if (ne(result, correct)) {
		printf("gcd(%g, %g) = %g (Scaled) != %g (double) fail\n", a, b, result, correct);
	}
	return eq(result, correct);
}

double Scaled::lcmDouble(const double a, const double b) {
	const double denom = gcdDouble(a, b);
	if (eqz(denom)) return denom;
	return (a * b) / denom;
}

long double Scaled::lcmLongDouble(const long double a, const long double b) {
	const long double denom = gcdLongDouble(a, b);
	if (EQZ(denom)) return denom;
	return (a * b) / denom;
}

// We need to use long double for make our results match.
bool Scaled::testLcm(const long double a, const long double b) {
	const long double result = Scaled::lcm(a, b).toLongDouble();
	const long double correct = lcmLongDouble(a, b); // std::lcm() doesn't do double

	if (NE(result, correct)) {
		printf("lcm(%Lg, %Lg) = %.17Lg (Scaled) != %.17Lg (double) fail\n", a, b, result, correct);
	}
	return EQ(result, correct);
}


bool Scaled::testSqrt(const double a) {
	const double result = Scaled::sqrt(a).toDouble();
	const double correct = std::sqrt(a);
	if (ne(result, correct)) {
		printf("sqrt(%g) = %g (Scaled) != %g (double) fail\n", a, result, correct);
	}
	return eq(result, correct);
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
	const double result = Scaled::isPrime(a).toDouble();
	const double correct = isPrimeDouble(a);
	if (ne(result, correct)) {
		printf("isPrime(%g) = %g (Scaled) != %g (double) fail\n", a, result, correct);
	}
	return eq(result, correct);
}

double Scaled::factorialDouble(const double n) {
	if (n <= 1) return 1;
	return factorialDouble(n - 1) * n;
}

bool Scaled::testFactorial(const double a) {
	const double result = Scaled::factorial(a).toDouble();
	const double correct = factorialDouble(a);
	if (ne(result, correct)) {
		printf("%g! = %g (Scaled) != %g (double) fail\n", a, result, correct);
	}
	return eq(result, correct);
}

bool Scaled::testSin(const double a) {
	const double result = Scaled::sin(a).toDouble();
	const double correct = std::sin(a);
	if (ne(result, correct)) {
		printf("sin %g = %g (Scaled) != %g (double) fail\n", a, result, correct);
	}
	return eq(result, correct);
}

bool Scaled::testCos(const double a) {
	const double result = Scaled::cos(a).toDouble();
	const double correct = std::cos(a);
	if (ne(result, correct)) {
		printf("cos %g = %g (Scaled) != %g (double) fail\n", a, result, correct);
	}
	return eq(result, correct);
}

bool Scaled::testTan(const double a) {
	const double result = Scaled::tan(a).toDouble();
	const double correct = std::tan(a);
	if (ne(result, correct)) {
		printf("tan %g = %g (Scaled) != %g (double) fail\n", a, result, correct);
	}
	return eq(result, correct);
}

bool Scaled::testLn(const double a) {
	const double result = Scaled::ln(a).toDouble();
	const double correct = std::log(a);
	if (ne(result, correct)) {
		printf("ln %g = %g (Scaled) != %g (double) fail\n", a, result, correct);
	}
	return eq(result, correct);
}

bool Scaled::testLog(const double a) {
	const double result = Scaled::log(a).toDouble();
	const double correct = std::log10(a);
	if (ne(result, correct)) {
		printf("log %g = %g (Scaled) != %g (double) fail\n", a, result, correct);
	}
	return eq(result, correct);
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
			values.push_back((double)n);
		}

		std::sort(values.begin(), values.end());
		(void) std::unique(values.begin(), values.end());
	} else { // Counting down
		for (long n = start; n >= end; n += inc) {
			values.push_back((double)n / 10.0);
		}

		// We want extra tests around zero
		for (long n = 10; n >= -10; n--) {
			values.push_back((double)n);
		}

		// Reverse sort
		std::sort(values.begin(), values.end(), std::greater<>());
		(void) std::unique(values.begin(), values.end());
	}
}

void Scaled::testAllInner(const std::string &testName, const long precision, int &nSuccess, int &nFail, const long startMs) {
	const bool is_home = isHome();

	if (precision < 3 || precision > 9) {
		ArbNum_log("Sorry, scale must be 3 <= scale <= 9 for tests\n");
		return;
	}

	printf("Starting %s\n", testName.c_str());

	// Home settings
	int testLimit = 120; // Seconds
	double trigInc = .4;

	// Github settings
	if (!is_home) {
		testLimit = 15;
		trigInc *= 2;
	}

	ArbNum_setLogging(AN_QUIET); // Don't log about invalid numbers
	{
	const char *checkStr[] =       { "1e+4",  "1e-4",  "",  "0000", "0004", "4000", "hello", "-25", "123456789012345678901234567890" };
	const char *checkStrExpect[] = { "10000", ".0001", "0", "0",       "4", "4000",     "0", "-25", "123456789012345678901234567890" };
	const int nCheckStr = sizeof(checkStr) / sizeof(checkStr[0]);
	printf("Testing store str with %d values\n", nCheckStr);
	for	 (int i = 0; i < nCheckStr; i++) {
		if (testStoreStr(checkStr[i], checkStrExpect[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	} }
	ArbNum_setLogging(AN_LOG);

	{
	const double checkDouble[] = { 9.3, 9.000001, -4, 0.0003 };
	const int nCheckDouble = sizeof(checkDouble) / sizeof(checkDouble[0]);
	printf("Testing store double with %d values\n", nCheckDouble);
	for (int i = 0; i < nCheckDouble; i++) {
		if (testStoreDouble(checkDouble[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printf("Testing store long double with %d values\n", nCheckDouble);
	for (int i = 0; i < nCheckDouble; i++) {
		if (testStoreLongDouble(checkDouble[i])) {
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
		if (testStoreLong(checkLong[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printf("Testing store long long with %d values\n", nCheckLong);
	for (int i = 0; i < nCheckLong; i++) {
		if (testStoreLongLong(checkLong[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	} }

	{
	const double  checkRounding[] = { -2, -1.6 -1.5, -1.4, 0, 1.4, 1.5, 1.6, 2 };
	const int nCheckRounding = sizeof(checkRounding) / sizeof(checkRounding[0]);

	printf("Testing floor with %d values\n", nCheckRounding);
	for (int i = 0; i < nCheckRounding; i++) {
		if (testFloor(checkRounding[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printf("Testing ceil with %d values\n", nCheckRounding);
	for (int i = 0; i < nCheckRounding; i++) {
		if (testCeil(checkRounding[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printf("Testing round with %d values\n", nCheckRounding);
	for (int i = 0; i < nCheckRounding; i++) {
		if (testRound(checkRounding[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	} }

	// These numbers are divided by 10 and changed into doubles
	const long A_START = -10000;
	const long A_END = 10000;
	long A_INC = 77;
	const long B_START = 10000;
	const long B_END = -10000;
	long B_INC = -88;

	// Go faster
	if (is_home) {
		A_INC *= 9;
		B_INC *= 9;
	}
	else {
		A_INC *= 18;
		B_INC *= 18;
	}

	// Make vectors of test numbers
	std::vector<double> a_values, b_values;
	makeTestValues(A_START, A_END, A_INC, a_values);
	makeTestValues(B_START, B_END, B_INC, b_values);
	const long nValues = MKLONG(a_values.size() * b_values.size());

	printf("Testing compare with %ld values\n", nValues);
	for (double a : a_values) {
		for (double b : b_values) {
			if (testCompare(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		 }
	}

	printf("Testing add with %ld values\n", nValues);
	for (double a : a_values) {
		for (double b : b_values) {
			if (testAdd(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	printf("Testing subtract with %ld values\n", nValues);
	for (double a : a_values) {
		for (double b : b_values) {
			if (testSubtract(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	printf("Testing multiply with %ld values\n", nValues);
	for (double a : a_values) {
		for (double b : b_values) {
			if (testMultiply(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	printf("Testing divide with %ld values\n", nValues);
	for (double a : a_values) {
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

	printf("Testing mod with %ld values\n", nValues);
	for (double a : a_values) {
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

	if (getDurationSeconds(startMs) > testLimit) return;
	{
	const double testPiMult[] = { .5, -.5, .25, -.25 };
	const double testPiDiv[] = { 2, -2, 4, -4 };
	const int nTestPi = sizeof(testPiMult) / sizeof(testPiMult[0]);
	printf("Testing pi math with %d values\n", nTestPi);
	for (int i = 0; i < nTestPi; i++) {
		if (testPiMath(testPiMult[i], testPiDiv[i])) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	} }		

	if (getDurationSeconds(startMs) > testLimit) return;
	printf("Testing gcd with %ld values\n", nValues);
	for (double a : a_values) {
		for (double b : b_values) {
			if (testGcd(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	if (getDurationSeconds(startMs) > testLimit) return;
	printf("Testing lcm with %ld values\n", nValues);
	for (double a : a_values) {
		for (double b : b_values) {
			if (testLcm(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	if (getDurationSeconds(startMs) > testLimit) return;
	printf("Testing sqrt with %ld values\n", nValues);
	for (double a = 0; a <= A_END; a += (double)A_INC) {
		if (testSqrt(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	if (getDurationSeconds(startMs) > testLimit) return;
	printf("Testing isPrime with %d values\n", 50 / 3);
	for (double a = 0; a <= 50; a+= 3) {
		if (testIsPrime(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	if (getDurationSeconds(startMs) > testLimit) return;
	printf("Testing factorial with %d values\n", 13);
	for (double a = 1; a < 13; a++) {
		if (testFactorial(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	if (getDurationSeconds(startMs) > testLimit) return;
	printf("Testing sin with %g values\n", (7 + 7) / trigInc);
	for (double a = -7; a < 7; a += trigInc) {
		if (testSin(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	if (getDurationSeconds(startMs) > testLimit) return;
	printf("Testing cos with %g values\n", (7 + 7) /  trigInc);
	for (double a = -7; a < 7; a += trigInc) {
		if (testCos(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	if (getDurationSeconds(startMs) > testLimit) return;
	printf("Testing tan with %g values\n", (7 + 7) /  trigInc);
	for (double a = -7; a < 7; a += trigInc) {
		if (testTan(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	if (getDurationSeconds(startMs) > testLimit) return;
	printf("Testing exp with %d values\n", 10 + 10);
	for (double a = -10; a < 10; a++) {
		if (testExp(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	if (getDurationSeconds(startMs) > testLimit) return;
	{
	const double nPow = (7 + 7) / trigInc;
	printf("Testing pow with %ld values\n", MKLONG(nPow * nPow));
	for (double a = -7; a < 7; a += trigInc) {
		if (getDurationSeconds(startMs) > testLimit) return;
		for (double b = -7; b < 7; b += trigInc) {
			if (a < 0) continue; // Negative base not supported yet
			if (testPow(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}
	}

	if (getDurationSeconds(startMs) > testLimit) return;
	printf("Testing ln with %g values\n", (7 + 7) / trigInc);
	for (double a = -7; a < 7; a += trigInc) {
		if (a <= 0) continue; // Not defined
		if (testLn(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	if (getDurationSeconds(startMs) > testLimit) return;
	printf("Testing log with %ld values\n", a_values.size());
	for (double a : a_values) {
		if (a <= 0) continue; // Not defined
		if (testLog(a)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}
}

bool Scaled::testAll() {
	int nSuccess = 0;
	int nFail = 0;
	const long precision = getPrecisionGlobal();
	const std::string testName = "scaledTest scale=" + std::to_string(precision);
	const long startMs = currentMillis();
	testAllInner(testName, precision, nSuccess, nFail, startMs);
	printTestResults(testName.c_str(), nSuccess, nFail, startMs);
	return nFail == 0;
}
