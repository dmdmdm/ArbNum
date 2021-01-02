// A simple arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#ifndef ARBNUM_H
#define ARBNUM_H

#include <string>
#include <vector>

class UnsignedDivide;
class ArbNumDivide;

class Unsigned {
  typedef std::vector<int> digits_t;
  digits_t mDigits;

  void zero();
  void pad(const size_t wantedLength);
  static void padShorterNumber(Unsigned&, Unsigned&);
  static void padShorterNumber(const Unsigned*& pA, const Unsigned*& pB,
                               Unsigned& padded);
  void appendRight(const int digit, const size_t numberOfDigits);

  static int charToInt(const char);
  static char intToChar(const int);

  void mkError();
  size_t length() const { return mDigits.size(); }
  void prepend(const int digit) { mDigits.insert(mDigits.begin(), digit); }
  void append(const int digit) { mDigits.push_back(digit); }
  static Unsigned findDivisibleSizedChunk(const Unsigned& runningDividend,
                                          const Unsigned& dividend,
                                          const Unsigned& divisor,
                                          int& iDividendPos, bool& fits);

 public:
  void trim();
  void clear();
  void saveNumber(const char*);
  std::string toString() const;
  int toInt() const;
  long toLong() const;
  void set(const Unsigned&);
  bool isZero() const;
  bool isOne() const;
  bool isTwo() const;
  bool isOdd() const;
  bool isEven() const;

  inline static void addMutable(Unsigned&, Unsigned&);
  static Unsigned add(const Unsigned&, const Unsigned&);

  inline static void subtractMutable(Unsigned&, Unsigned&);
  static Unsigned subtract(const Unsigned&, const Unsigned&);

  static Unsigned multiply(const Unsigned&, const Unsigned&);
  static UnsignedDivide divideByOneWithRem(const Unsigned&);
  static UnsignedDivide divideByTwoWithRem(const Unsigned&);
  static UnsignedDivide divideWithRemSlow(const Unsigned&, const Unsigned&);
  static UnsignedDivide divideWithRemFast(const Unsigned&, const Unsigned&);
  static Unsigned half(const Unsigned&);
  static Unsigned divide(const Unsigned&, const Unsigned&);
  static Unsigned mod(const Unsigned&, const Unsigned&);
  static Unsigned pow(const Unsigned&, const Unsigned&);
  static Unsigned gcd(const Unsigned&, const Unsigned&);
  static Unsigned sqrt(const Unsigned&);
  static Unsigned random(const Unsigned&);
  static Unsigned isPrime(const Unsigned&);
  static int compare(const Unsigned&, const Unsigned&);

  void add(const Unsigned&);
  void subtract(const Unsigned&);
  void multiply(const Unsigned&);
  void divide(const Unsigned&);
  void mod(const Unsigned&);
  void pow(const Unsigned&);

  Unsigned() {}
  Unsigned(const Unsigned&);
  Unsigned(const char*);
  Unsigned(const int);
  Unsigned(const long);
  Unsigned(const bool);

  int& operator[](const int i) { return mDigits[i]; }
  Unsigned operator+=(const Unsigned& other) {
    add(other);
    return *this;
  }
  Unsigned operator-=(const Unsigned& other) {
    subtract(other);
    return *this;
  }
  Unsigned operator*=(const Unsigned& other) {
    multiply(other);
    return *this;
  }
  Unsigned operator/=(const Unsigned& other) {
    divide(other);
    return *this;
  }
  Unsigned operator%=(const Unsigned& other) {
    mod(other);
    return *this;
  }
  Unsigned operator^=(const Unsigned& other) {
    pow(other);
    return *this;
  }
  Unsigned operator++(int);
  Unsigned operator--(int);
};

class UnsignedDivide {
public:
  Unsigned quotient, remainder;

  UnsignedDivide() {
    quotient = 0;
    remainder = 0;
  }
  UnsignedDivide(const Unsigned& q, const Unsigned& r) {
    quotient = q;
    remainder = r;
  }
  void trim() {
    quotient.trim();
    remainder.trim();
  }
};

inline bool operator<(const Unsigned& a, const Unsigned& b) {
  return Unsigned::compare(a, b) < 0;
}
inline bool operator>(const Unsigned& a, const Unsigned& b) {
  return Unsigned::compare(a, b) > 0;
}
inline bool operator==(const Unsigned& a, const Unsigned& b) {
  return Unsigned::compare(a, b) == 0;
}
inline bool operator<=(const Unsigned& a, const Unsigned& b) {
  return Unsigned::compare(a, b) <= 0;
}
inline bool operator>=(const Unsigned& a, const Unsigned& b) {
  return Unsigned::compare(a, b) >= 0;
}
inline Unsigned operator+(const Unsigned& a, const Unsigned& b) {
  return Unsigned::add(a, b);
}
inline Unsigned operator-(const Unsigned& a, const Unsigned& b) {
  return Unsigned::subtract(a, b);
}
inline Unsigned operator*(const Unsigned& a, const Unsigned& b) {
  return Unsigned::multiply(a, b);
}
inline Unsigned operator/(const Unsigned& a, const Unsigned& b) {
  return Unsigned::divide(a, b);
}
inline Unsigned operator%(const Unsigned& a, const Unsigned& b) {
  return Unsigned::mod(a, b);
}
inline Unsigned operator^(const Unsigned& a, const Unsigned& b) {
  return Unsigned::pow(a, b);
}

class ArbNum {
  typedef enum { SPEC_NORMAL, SPEC_ERROR, SPEC_IGNORE } SpecialType;
  SpecialType mSpecial;
  int mSign;  // -1 or 1
  Unsigned mUnsigned;

  void clear();
  bool isZero() const { return mUnsigned.isZero(); }
  bool isPositiveOrZero() const { return mSign >= 0; }
  bool isNegativeOrZero() const { return mSign <= 0; }
  bool isPostive() const { return mSign > 0 && !isZero(); }
  bool isNegative() const { return mSign < 0 && !isZero(); }

  void mkNegative() { mSign = -1; }
  void mkPositive() { mSign = 1; }
  void flipSign() { mSign *= -1; }

 public:
  void saveNumber(const char*);

  void mkError() {
    clear();
    mSpecial = SPEC_ERROR;
  }
  bool isError() const { return mSpecial == SPEC_ERROR; }

  void mkIgnore() {
    clear();
    mSpecial = SPEC_IGNORE;
  }
  bool isIgnore() const { return mSpecial == SPEC_IGNORE; }

  bool isNormal() const { return mSpecial == SPEC_NORMAL; }

  ArbNum() { clear(); }
  ArbNum(const Unsigned&);
  ArbNum(const ArbNum&);
  ArbNum(const char*);
  ArbNum(const int);
  ArbNum(const long);
  ArbNum(const bool);

  static ArbNum abs(const ArbNum&);
  static ArbNum sign(const ArbNum&);
  static ArbNum add(const ArbNum&, const ArbNum&);
  static ArbNum subtract(const ArbNum&, const ArbNum&);
  static ArbNum multiply(const ArbNum&, const ArbNum&);
  static ArbNumDivide divideWithRem(const ArbNum&, const ArbNum&);
  static ArbNum divide(const ArbNum&, const ArbNum&);
  static ArbNum mod(const ArbNum&, const ArbNum&);
  static ArbNum min(const ArbNum&, const ArbNum&);
  static ArbNum max(const ArbNum&, const ArbNum&);
  static ArbNum doNot(const ArbNum&);
  static ArbNum pow(const ArbNum&, const ArbNum&);
  static ArbNum gcd(const ArbNum&, const ArbNum&);
  static ArbNum factorial(const ArbNum&);
  static ArbNum sqrt(const ArbNum&);
  static ArbNum random(const ArbNum&);
  static ArbNum isPrime(const ArbNum&);
  static int compare(const ArbNum&, const ArbNum&);

  void set(const ArbNum&);
  void add(const ArbNum&);
  void subtract(const ArbNum&);
  void multiply(const ArbNum&);
  void divide(const ArbNum&);
  void mod(const ArbNum&);
  void pow(const ArbNum&);

  std::string toString() const;
  int toInt() const;
  long toLong() const;

  ArbNum operator+=(const ArbNum& other) {
    add(other);
    return *this;
  }
  ArbNum operator-=(const ArbNum& other) {
    subtract(other);
    return *this;
  }
  ArbNum operator*=(const ArbNum& other) {
    multiply(other);
    return *this;
  }
  ArbNum operator/=(const ArbNum& other) {
    divide(other);
    return *this;
  }
  ArbNum operator%=(const ArbNum& other) {
    mod(other);
    return *this;
  }
  ArbNum operator^=(const ArbNum& other) {
    pow(other);
    return *this;
  }
  ArbNum operator++(int);
  ArbNum operator--(int);

  static bool testStoreStr(const char*, const int);
  static bool testStoreInt(const int);
  static bool testStoreLong(const long);
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
  static long sqrtLong(const long);
  static bool testSqrt(const long);
  static long isPrimeLong(const long);
  static bool testIsPrime(const long);
  static long factorialLong(const long);
  static bool testFactorial(const long);
  static bool testAll();
};

class ArbNumDivide {
public:
  ArbNum quotient, remainder;

  ArbNumDivide() {
    quotient = 0;
    remainder = 0;
  }
  ArbNumDivide(const ArbNum& q, const ArbNum& r) {
    quotient = q;
    remainder = r;
  }
};

inline bool operator<(const ArbNum& a, const ArbNum& b) {
  return ArbNum::compare(a, b) < 0;
}
inline bool operator>(const ArbNum& a, const ArbNum& b) {
  return ArbNum::compare(a, b) > 0;
}
inline bool operator==(const ArbNum& a, const ArbNum& b) {
  return ArbNum::compare(a, b) == 0;
}
inline bool operator<=(const ArbNum& a, const ArbNum& b) {
  return ArbNum::compare(a, b) <= 0;
}
inline bool operator>=(const ArbNum& a, const ArbNum& b) {
  return ArbNum::compare(a, b) >= 0;
}
inline ArbNum operator+(const ArbNum& a, const ArbNum& b) {
  return ArbNum::add(a, b);
}
inline ArbNum operator-(const ArbNum& a, const ArbNum& b) {
  return ArbNum::subtract(a, b);
}
inline ArbNum operator*(const ArbNum& a, const ArbNum& b) {
  return ArbNum::multiply(a, b);
}
inline ArbNum operator/(const ArbNum& a, const ArbNum& b) {
  return ArbNum::divide(a, b);
}
inline ArbNum operator%(const ArbNum& a, const ArbNum& b) {
  return ArbNum::mod(a, b);
}
inline ArbNum operator^(const ArbNum& a, const ArbNum& b) {
  return ArbNum::pow(a, b);
}
inline ArbNum operator!(const ArbNum& a) { return ArbNum::doNot(a); }

#endif
