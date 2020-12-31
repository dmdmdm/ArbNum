// A simple arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#ifndef BC_TOKENIZER_H
#define BC_TOKENIZER_H

#include <string>

struct Token {
  typedef enum {
    T_EOL,
    T_SPACE,
    T_MINUS,
    T_NUMBER,
    T_WORD,
    T_PUNCT,
    T_OTHER
  } TokType;

  TokType type;
  std::string string;

  Token() { type = T_OTHER; }

  std::string toString() const;
  bool isOpenBracket() const { return type == T_PUNCT && string == "("; }
  bool isCloseBracket() const { return type == T_PUNCT && string == ")"; }
  bool isBang() const { return type == T_PUNCT && string == "!"; }
};

class Tokenizer {
  typedef enum { EXPECT_ANY, EXPECT_OPERATOR } Expect;

  std::string mLine;
  int mPos;
  Token mUnget;

  bool isEol();
  int getChar();
  void ungetChar();

 public:
  Tokenizer(const char* line) {
    mLine = line;
    mPos = 0;
  }

  static Token::TokType type(const int);
  static std::string typeStr(const Token::TokType);

  Token getToken(const Expect expect = EXPECT_ANY);
  Token getOperatorToken() { return getToken(EXPECT_OPERATOR); }
  void ungetToken(const Token& tok);
};

#endif
