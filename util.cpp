// A simple arbitrary precision library and interactive text program
// Copyright © 2026, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include <stdarg.h>
#include <stdio.h>
#include <cstring>
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
