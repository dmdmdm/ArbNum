// A simple arbitrary precision library and interactive text program
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#pragma once

#include <string>

//
// const
//

enum { ARBNUM_BUF_SIZE = 1024 };

//
// Covert
//
#define MKLONG(_a) (long)(_a)

//
// Logging
//
typedef enum { AN_QUIET, AN_LOG } ArbNum_Logging;
void ArbNum_setLogging(const ArbNum_Logging);
ArbNum_Logging ArbNum_getLogging();

void ArbNum_log(const char *format, ...);

//
// Strings
//

char *terminatingNul(char *);

bool isSpace(const char *);
bool hasContent(const char *);

void safe_strcpy(char *dest, const char *src, const size_t size);

char *shuffle(char *dest, const char *src);

//
// Time
//

long currentMillis();

double getDurationSeconds(const long startMs);

void printTestResults(const char *testName, const int nSuccess, const int nFail, const long startMs);

//
// Math
//

long powLong(const long base, const long exp);

long sqrtLong(const long);

void utilTest();
