// A simple arbitrary precision library and interactive text program
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include <stdarg.h>
#include <stdio.h>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <stdexcept>
#include <stddef.h>	// For NULL

#ifndef _MSC_VER
#include <sys/time.h>	// For gettimeofday()
#endif

#include "util.h"

//------------------------------------------------------------------------------
// Logging

static ArbNum_Logging s_Logging = AN_LOG;

void ArbNum_setLogging(const ArbNum_Logging log) {
	s_Logging = log;
}

ArbNum_Logging ArbNum_getLogging() {
	return s_Logging;
}

void ArbNum_log(const char *format, ...) {
	if (s_Logging != AN_LOG) return;
	va_list		ap;

	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

//------------------------------------------------------------------------------
// Strings

char *terminatingNul(char *s) {
	if (s == nullptr) return nullptr;
	return s + strlen(s);
}

bool isSpace(const char *s) {
	if (s == nullptr) return true;

	for (; *s; s++) {
		if (!isspace(*s)) return false;
	}

	return true;
}

bool hasContent(const char *s) {
	return ! isSpace(s);
}

void safe_strcpy(char *dest, const char *src, const size_t size) {
	if (dest == nullptr || size == 0) return;

    if (src == nullptr) {
        dest[0] = '\0';
        return;
    }

#ifdef _MSC_VER
	lstrcpyn(dest, src, size); // Correct
#elif defined(_WIN32)
	strncpy(dest, src, size); // Correct
	dest[size - 1] = '\0';
#elif defined(__APPLE__) && defined(__MACH__)
	strlcpy(dest, src, size); // Correct
#elif defined(__linux__)
	strlcpy(dest, src, size);
#elif defined(__FreeBSD__)
	strlcpy(dest, src, size);
#elif defined(__unix__)
	strlcpy(dest, src, size);
#else
#error "Unknown operating system"
#endif
}

char *shuffle(char *dest, const char *src) {
	return (char *) memmove(dest, src, strlen(src) + sizeof(char));
}

//------------------------------------------------------------------------------
// Time

long currentMillis() {
#ifdef _MSC_VER
	return GetTickCount();
#else
	struct timeval tp;

	gettimeofday(&tp, NULL);
	return tp.tv_sec * 1000 + tp.tv_usec / 1000;
#endif
}

double getDurationSeconds(const long startMs) {
	const long durationMs = currentMillis() - startMs;
	const double durationSeconds = (double)durationMs / 1000.0;
	return durationSeconds;
}

void printTestResults(const char *testName, const int nSuccess, const int nFail, const long startMs) {
#ifdef _MSC_VER
	const char *icon = (nFail == 0) ? "!" : "X";
#else
	// Icon is check mark or ballot X
	const char *icon = (nFail == 0) ? "\u2713" : "\u2717";
#endif
	printf("%s  %s  Success: %d  Fail: %d  Took: %.1f seconds\n", icon, testName, nSuccess, nFail, getDurationSeconds(startMs));
}

//------------------------------------------------------------------------------
// Math

long powLong(const long base, const long exp) {
	if (exp < 0) {
        ArbNum_log("powLog: negative exponents are not supported\n");
        return -1;
    }
	
	// Non-recursive so can handle really big numbers
	long result = 1;
	for (long i = 0; i < exp; i++) {
		result *= base;
	}
	return result;
}

long sqrtLong(const long a) {
	return (long) std::sqrt(a);
}

bool testPow(const long base, const long exp) {
	const long result = powLong(base, exp);
	const long correct = (long)std::pow(base, exp);
	if (result != correct) {
		printf("testUtil: powLong %ld ^ %ld = %ld != %ld fail\n", base, exp, result, correct);
	}
	return result == correct;
}

void utilTest() {
	int nSuccess = 0;
	int nFail = 0;
	const long startMs = currentMillis();

	// Regular numbers
	for (long a = 0; a < 10; a++) {
		for (long b = 16; b >= 0; b--) {
			if (testPow(a, b)) {
				nSuccess++;
			}
			else {
				nFail++;
			}
		}
	}

	// Powers of 10
	for (long exp = 0; exp < 18; exp++) {
		if (testPow(10, exp)) {
			nSuccess++;
		}
		else {
			nFail++;
		}
	}

	printTestResults("utilMath", nSuccess, nFail, startMs);
}
