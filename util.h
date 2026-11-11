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

char *shuffle(char *dest, const char *src);

//
// Time
//

long currentMillis();
