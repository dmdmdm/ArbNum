// An arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include <stdio.h>
#include <string.h>
#include <map>
#include "util.h"
#include "bc_calc.h"

//------------------------------------------------------------------------------
// Functions

struct Function {
	const char *name;
	int params;
	void *func;
};

// Alphabetical order
static const Function s_Funcs[] = {
	{"abs",			1, (void*) Scaled::abs},
	{"ceil",		1, (void*) Scaled::ceil},
	{"cos",			1, (void*) Scaled::cos},
	{"factorial",	1, (void*) Scaled::factorial},
	{"floor",		1, (void*) Scaled::floor},
	{"gcd",			2, (void*) Scaled::gcd},
	{"isprime",		1, (void*) Scaled::isPrime},
	{"lcm",			2, (void*) Scaled::lcm},
	{"max",			2, (void*) Scaled::max},
	{"min",			2, (void*) Scaled::min},
	{"random", 		1, (void*) Scaled::random},
	{"round", 		1, (void*) Scaled::round},
	{"sign",		1, (void*) Scaled::sign},
	{"sin",			1, (void*) Scaled::sin},
	{"sqrt",		1, (void*) Scaled::sqrt},
	{"tan",			1, (void*) Scaled::tan},
	{NULL,			0, NULL}
};

static const Function *findFunc(const char *name) {
	for (const Function *pEntry = s_Funcs; pEntry->name; pEntry++) {
		if (eq(pEntry->name, name)) {
			return pEntry;
		}
	}
	return NULL;
}

static const std::string getFuncs() {
	std::string result;

	for (const Function *pEntry = s_Funcs; pEntry->name; pEntry++) {
		if (!result.empty()) result += " ";
		result += (std::string)pEntry->name + (std::string)"()";
	}
	return result;
}

typedef Scaled (*func1_t)(Scaled &);
typedef Scaled (*func2_t)(Scaled &, Scaled &);

Scaled Calc::functionCall(Token funcName, Tokenizer &tokenizer) {
	Scaled result;

	const Function *pEntry = findFunc(funcName.string.c_str());
	if (pEntry == NULL) {
		ArbNum_log("Unknown function '%s'\n", funcName.string.c_str());
		result.mkError();
		return result;
	}

	if (pEntry->params == 1) {
		Scaled expr1 = expr(tokenizer);
		if (!expr1.isNormal()) {
			// Don't log anything here since a better error has probably already been written
			// printf("functionCall: expr1 is not normal\n");
			return expr1;
		}
		const func1_t pFunc1 = (func1_t)pEntry->func;
		result = (*pFunc1)(expr1);
	}
	else if (pEntry->params == 2) {
		Scaled expr1 = expr(tokenizer);
		if (!expr1.isNormal()) return expr1;
		const Token comma = tokenizer.getToken();
		Scaled expr2 = expr(tokenizer);
		if (!expr2.isNormal()) return expr2;
		const func2_t pFunc2 = (func2_t)pEntry->func;
		result = (*pFunc2)(expr1, expr2);
	}
	else {
		ArbNum_log("Can not handle functions with more than 2 parameters, got '%s'\n", funcName.string.c_str());
		result.mkError();
		return result;
	}

	const Token closeBracket = tokenizer.getToken();
	if (!closeBracket.isCloseBracket()) {
		ArbNum_log("Expected close bracket, got '%s'\n", closeBracket.string.c_str());
		result.mkError();
		return result;
	}

	return result;
}

//------------------------------------------------------------------------------
// Variables

static std::map<std::string, Scaled> s_Variables;

void Calc::saveVariableValue(const std::string &name, const Scaled &value) {
	// Special cases
	if (name == "pi" || name == "e") {
		ArbNum_log("Ignoring attempt to set '%s'\n", name.c_str());
		return;
	}
	else if (name == "scale") {
		const long n = value.toLong();
		if (n < 0) {
			ArbNum_log("Negative scale is not allowed\n");
			return;
		}
		if (n > 1000) {
			ArbNum_log("Not allowing a scale larger than 1000.  We could do it but everything would be really slow\n");
			return;
		}
		Scaled::setPrecisionGlobal(n);
		printf("You set the scale to %ld\n", n);
		return;
	}

	s_Variables[name] = value;
	printf("Saved %s = %s\n", name.c_str(), value.toString().c_str());
}

Scaled Calc::getVariableValue(const std::string &name) {
	// Special cases
	if (name == "pi") {
		return Scaled::getPi();
	}
	else if (name == "e") {
		return Scaled::getE();
	}
	else if (name == "scale") {
		return Scaled::getPrecisionGlobal();
	}

	const auto it = s_Variables.find(name);
	if (it == s_Variables.end()) {
		ArbNum_log("Variable '%s' not found\n", name.c_str());
		Scaled out;
		out.mkError();
		return out;
	}

	return it->second;
}

//------------------------------------------------------------------------------
// Help

void Calc::help() {
	printf("You can enter arithmetic expressions like:\n");
	printf("1 + 2 <enter>\n");
	printf("4 - 6 <enter>\n");
	printf("3 * 4 <enter>\n");
	printf("100 / 5 <enter>\n");
	printf("102 %% 5 <enter>\n");
	printf("2.1 ^ 10 <enter>\n");
	printf("pi <enter>\n");
	printf("scale = 50<enter> set number of decimal places\n");
	printf("\n");
	printf("Numbers of any length are supported, eg 2 ^ 100\n");
	printf("String things together: 2 + 5 + 5 * 80000000000000000000000000\n");
	printf("Use brackets: 1 + (2 * 3)\n");
	printf("Built-in functions: %s\n", getFuncs().c_str());
	printf("Examples:\n");
	printf("\tgcd(10 * 10, 6 + 7)\n");
	printf("\t! isprime(27)\n");
	printf("\n");
	printf("help <enter> for this\n");
	printf("tests <enter> to run checks\n");
	printf("license <enter> for the license\n");
	printf("exit <enter> to leave\n");
}

void Calc::license() {
	printf("License: MPL 2.0.  You CAN use it in your commerical product.\n");
	printf("More info: https://choosealicense.com/licenses/mpl-2.0/\n");
}

//------------------------------------------------------------------------------
// Calc main

Scaled Calc::simpleExpr(Tokenizer &tokenizer) {
	Scaled a, ignore, err;

	ignore.mkIgnore();
	err.mkError();

	const Token tok1 = tokenizer.getToken();
	if (tok1.type == Token::T_EOL) return a;

	if (tok1.type == Token::T_NUMBER) {
		a.saveNumber(tok1.string.c_str());
	}
	else if (tok1.type == Token::T_WORD) {
		// Alphabetical order
		if (tok1.string == "exit") {
			exit(0);
			return ignore;
		}
		else if (tok1.string == "help") {
			help();
			return ignore;
		}
		else if (tok1.string == "license") {
			license();
			return ignore;
		}
		else if (tok1.string == "tests") {
			Scaled::testAll();
			return ignore;
		}

		const Token tok2 = tokenizer.getToken();
		if (tok2.isOpenBracket()) { // Function call
			a = functionCall(tok1, tokenizer);
			return a;
		}
		else if (tok2.isEquals()) { // Assignment
			const Scaled b = expr(tokenizer);
			if (b.isNormal()) {
				saveVariableValue(tok1.string, b);
				return ignore;
			}
			return b;
		}
		tokenizer.ungetToken(tok2);

		return getVariableValue(tok1.string);
	}
	else if (tok1.isOpenBracket()) {
		a = expr(tokenizer);

		const Token tok2 = tokenizer.getToken();
		if (!tok2.isCloseBracket()) {
			ArbNum_log("Expected close bracket, got '%s'\n", tok2.string.c_str());
			return err;
		}
	}
	else if (tok1.isBang()) {	// ! <expr>
		const Scaled b = expr(tokenizer);
		if (!b.isNormal()) {
			ArbNum_log("Expected ! <expr>\n");
			return err;
		}
		a = ! b;
	}
	else {
		ArbNum_log("Expected number, open bracket or function call, got '%s'.  Type 'help' for info.\n", tok1.string.c_str());
		return err;
	}

	return a;
}

Scaled Calc::expr(Tokenizer &tokenizer) {
	Scaled a = simpleExpr(tokenizer);
	if (!a.isNormal()) return a;

	for (;;) {
		const Token tok1 = tokenizer.getOperatorToken();
		if (tok1.type == Token::T_EOL) break;

		if (tok1.type != Token::T_PUNCT) {
			ArbNum_log("Expected operator, got '%s'\n", tok1.string.c_str());
			a.mkError();
			return a;
		}

		if (tok1.string[0] == ',' || tok1.string[0] == ')') {
			tokenizer.ungetToken(tok1);
			return a;
		}

		const Scaled b = simpleExpr(tokenizer);
		if (!b.isNormal()) return a;

		if (tok1.string[0] == '+') {
			a.add(b);
		}
		else if (tok1.string[0] == '-') {
			a.subtract(b);
		}
		else if (tok1.string[0] == '*') {
			a.multiply(b);
		}
		else if (tok1.string[0] == '/') {
			a.divide(b);
		}
		else if (tok1.string[0] == '%') {
			a.mod(b);
		}
		else if (tok1.string[0] == '^') {
			a.pow(b);
		}
		else {
			ArbNum_log("Unknown operator %s\n", tok1.string.c_str());
			a.mkError();
			return a;
		}
	}

	return a;
}

void Calc::run(const char *line) {
	Tokenizer tokenizer(line);

	const Scaled num = expr(tokenizer);

	if (num.isNormal()) {
		printf("%s\n", num.toString().c_str());
	}
}
