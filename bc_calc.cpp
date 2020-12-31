// A simple arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#include "bc_calc.h"

#include <stdio.h>
#include <string.h>

//------------------------------------------------------------------------------
// Functions

typedef struct {
  const char* name;
  int params;
  void* func;
} Function;

// Alphabetical order
static const Function gFuncs[] = {{"abs", 1, (void*)ArbNum::abs},
                                  {"factorial", 1, (void*)ArbNum::factorial},
                                  {"gcd", 2, (void*)ArbNum::gcd},
                                  {"isprime", 1, (void*)ArbNum::isPrime},
                                  {"max", 2, (void*)ArbNum::max},
                                  {"min", 2, (void*)ArbNum::min},
                                  {"random", 1, (void*)ArbNum::random},
                                  {"sign", 1, (void*)ArbNum::sign},
                                  {"sqrt", 1, (void*)ArbNum::sqrt},
                                  {NULL, 0, NULL}};

static const Function* findFunc(const char* name) {
  for (const Function* pEntry = gFuncs; pEntry->name; pEntry++) {
    if (strcmp(pEntry->name, name) == 0) {
      return pEntry;
    }
  }
  return NULL;
}

static const std::string getFuncs() {
  std::string result;

  for (const Function* pEntry = gFuncs; pEntry->name; pEntry++) {
    if (!result.empty()) result += ", ";
    result += (std::string)pEntry->name + (std::string) "()";
  }
  return result;
}

typedef ArbNum (*func1_t)(ArbNum&);
typedef ArbNum (*func2_t)(ArbNum&, ArbNum&);

ArbNum Calc::functionCall(Token funcName, Tokenizer& tokenizer) {
  ArbNum result;

  const Function* pEntry = findFunc(funcName.string.c_str());
  if (pEntry == NULL) {
    fprintf(stderr, "Unknown function '%s'\n", funcName.string.c_str());
    result.mkError();
    return result;
  }

  if (pEntry->params == 1) {
    ArbNum expr1 = expr(tokenizer);
    if (!expr1.isNormal()) return expr1;
    const func1_t pFunc1 = (func1_t)pEntry->func;
    result = (*pFunc1)(expr1);
  } else if (pEntry->params == 2) {
    ArbNum expr1 = expr(tokenizer);
    if (!expr1.isNormal()) return expr1;
    const Token comma = tokenizer.getToken();
    ArbNum expr2 = expr(tokenizer);
    if (!expr2.isNormal()) return expr1;
    const func2_t pFunc2 = (func2_t)pEntry->func;
    result = (*pFunc2)(expr1, expr2);
  } else {
    fprintf(stderr,
            "Can not handle functions with more than 2 parameters, got '%s'\n",
            funcName.string.c_str());
    result.mkError();
    return result;
  }

  const Token closeBracket = tokenizer.getToken();
  if (!closeBracket.isCloseBracket()) {
    fprintf(stderr, "Expected close bracket, got '%s'\n",
            closeBracket.string.c_str());
    result.mkError();
    return result;
  }

  return result;
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
  printf("2 ^ 10 <enter>\n\n");
  printf("Integers of any length are supported, eg 2 ^ 100\n");
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

ArbNum Calc::simpleExpr(Tokenizer& tokenizer) {
  ArbNum a, ignore, err;

  ignore.mkIgnore();
  err.mkError();

  const Token tok1 = tokenizer.getToken();
  if (tok1.type == Token::T_EOL) return a;

  if (tok1.type == Token::T_NUMBER) {
    a.saveNumber(tok1.string.c_str());
  } else if (tok1.type == Token::T_WORD) {
    if (tok1.string == "exit") {
      exit(0);
      return ignore;
    } else if (tok1.string == "help") {
      help();
      return ignore;
    } else if (tok1.string == "license") {
      license();
      return ignore;
    } else if (tok1.string == "tests") {
      ArbNum::testAll();
      return ignore;
    }

    const Token tok2 = tokenizer.getToken();
    if (!tok2.isOpenBracket()) {
      fprintf(stderr, "Unknown command, try 'help'\n");
      return err;
    }

    a = functionCall(tok1, tokenizer);
  } else if (tok1.isOpenBracket()) {
    a = expr(tokenizer);

    const Token tok2 = tokenizer.getToken();
    if (!tok2.isCloseBracket()) {
      fprintf(stderr, "Expected close bracket, got '%s'\n",
              tok2.string.c_str());
      return err;
    }
  } else if (tok1.isBang()) {  // ! <expr>
    const ArbNum b = expr(tokenizer);
    if (!b.isNormal()) {
      fprintf(stderr, "Expected ! <expr>\n");
      return err;
    }
    a = !b;
  } else {
    fprintf(stderr,
            "Expected number, open bracket or function call, got '%s'\n",
            tok1.string.c_str());
    return err;
  }

  return a;
}

ArbNum Calc::expr(Tokenizer& tokenizer) {
  ArbNum a = simpleExpr(tokenizer);
  if (!a.isNormal()) return a;

  for (;;) {
    const Token tok1 = tokenizer.getOperatorToken();
    if (tok1.type == Token::T_EOL) break;

    if (tok1.type != Token::T_PUNCT) {
      fprintf(stderr, "Expected operator, got '%s'\n", tok1.string.c_str());
      a.mkError();
      return a;
    }

    if (tok1.string[0] == ',' || tok1.string[0] == ')') {
      tokenizer.ungetToken(tok1);
      return a;
    }

    const ArbNum b = simpleExpr(tokenizer);
    if (!b.isNormal()) return a;

    if (tok1.string[0] == '+') {
      a.add(b);
    } else if (tok1.string[0] == '-') {
      a.subtract(b);
    } else if (tok1.string[0] == '*') {
      a.multiply(b);
    } else if (tok1.string[0] == '/') {
      a.divide(b);
    } else if (tok1.string[0] == '%') {
      a.mod(b);
    } else if (tok1.string[0] == '^') {
      a.pow(b);
    } else {
      fprintf(stderr, "Unknown operator %s\n", tok1.string.c_str());
      a.mkError();
      return a;
    }
  }

  return a;
}

void Calc::run(const char* line) {
  Tokenizer tokenizer(line);

  const ArbNum num = expr(tokenizer);

  if (num.isNormal()) {
    printf("%s\n", num.toString().c_str());
  }
}
