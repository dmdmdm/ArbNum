// An arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#pragma once

#include "scaled.h"
#include "bc_tokenizer.h"

class Calc {
	Scaled functionCall(Token, Tokenizer &);
	void help();
	void license();
	Scaled simpleExpr(Tokenizer &);
	Scaled expr(Tokenizer &);
	void saveVariableValue(const std::string &name, const Scaled &value);
	Scaled getVariableValue(const std::string &name);

public:
	Calc() { }
	void run(const char *);
};
