// An arbitrary precision library and interactive text program
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#pragma once

#include <string>
#include <string.h>

//------------------------------------------------------------------------------
// Constants

enum { ARBNUM_BUF_SIZE = 1024 };

enum { ARBNUM_LINE_SIZE = 1024 * 5 };

// pre-defined in Windows only
#ifndef MAX_PATH
#define MAX_PATH (256)
#endif

#if __cplusplus >= 201703L
#define INLINE inline
#else
#define INLINE static
#endif

//------------------------------------------------------------------------------
// Covert

#define MKLONG(_a) static_cast<long>(_a)

//------------------------------------------------------------------------------
// Logging

typedef enum { AN_QUIET, AN_LOG } ArbNum_Logging;
void ArbNum_setLogging(const ArbNum_Logging);
ArbNum_Logging ArbNum_getLogging();

void ArbNum_log(const char *format, ...);

bool isHome();

//------------------------------------------------------------------------------
// Strings

char *terminatingNul(char *);

bool isSpace(const char *);
bool hasContent(const char *);

void portable_strlcpy(char *dest, const char *src, const size_t size);

int portable_stricmp(const char *, const char *);

inline bool eq(const char *a, const char *b) {
	return strcmp(a,b) == 0;
}

inline bool ne(const char *a, const char *b) {
	return strcmp(a,b) != 0;
}

inline bool portable_ceq(const char *a, const char *b) {
	return portable_stricmp(a,b) == 0;
}

inline bool portable_cne(const char *a, const char *b) {
	return portable_stricmp(a,b) != 0;
}

char *shuffle(char *dest, const char *src);

//------------------------------------------------------------------------------
// Time

long currentMillis();

double getDurationSeconds(const long startMs);

void printTestResults(const char *testName, const int nSuccess, const int nFail, const long startMs);

//------------------------------------------------------------------------------
// Math

long powLong(const long base, const long exp);

long sqrtLong(const long);

bool utilTest();
