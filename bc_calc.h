// A simple arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#ifndef BC_CALC_H
#define BC_CALC_H

#include "arbnum.h"
#include "bc_tokenizer.h"

class Calc {
  ArbNum functionCall(Token, Tokenizer&);
  void help();
  void license();
  ArbNum simpleExpr(Tokenizer&);
  ArbNum expr(Tokenizer&);

 public:
  Calc() {}
  void run(const char*);
};

#endif
