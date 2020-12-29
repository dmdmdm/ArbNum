// A simple arbitrary precision library and interactive text program
// Copyright © 2020, Dave McKellar
// Mozilla Public Licensed

#include "bc_tokenizer.h"

#include <stdio.h>

std::string Token::toString() const {
  return Tokenizer::typeStr(type) + " " + string;
}

bool Tokenizer::isEol() { return mPos >= (int)mLine.length(); }

int Tokenizer::getChar() {
  if (isEol()) return EOF;
  return mLine[mPos++];
}

void Tokenizer::ungetChar() { mPos--; }

Token::TokType Tokenizer::type(const int c) {
  if (c == EOF) return Token::T_EOL;
  if (isspace(c)) return Token::T_SPACE;
  if (c == '-') return Token::T_MINUS;
  if (isdigit(c) || c == '.') return Token::T_NUMBER;
  if (isalpha(c)) return Token::T_WORD;
  if (ispunct(c)) return Token::T_PUNCT;
  return Token::T_OTHER;
}

std::string Tokenizer::typeStr(const Token::TokType tt) {
  switch (tt) {
    case Token::T_EOL:
      return "eol";
    case Token::T_SPACE:
      return "space";
    case Token::T_MINUS:
      return "minus";
    case Token::T_NUMBER:
      return "number";
    case Token::T_WORD:
      return "word";
    case Token::T_PUNCT:
      return "punct";
    case Token::T_OTHER:
      return "other";
  }
  return "unknown";
}

Token Tokenizer::getToken(const Expect expect) {
  Token tok;

  if (mUnget.type != Token::T_OTHER) {
    tok = mUnget;
    mUnget.type = Token::T_OTHER;
    return tok;
  }

  if (isEol()) {
    tok.type = Token::T_EOL;
    tok.string = "EOL";
    return tok;
  }

  tok.type = Token::T_OTHER;
  for (;;) {
    const int c = getChar();
    if (c == EOF) {
      if (tok.string.length() == 0) {
        tok.type = Token::T_EOL;
        tok.string = "EOL";
        return tok;
      } else {
        return tok;
      }
    }

    const Token::TokType tt = type(c);

    if (tok.string.length() == 0) {  // First character of the token
      if (tt == Token::T_SPACE) continue;
      tok.string = (char)c;
      tok.type = tt;

      if (tok.type == Token::T_PUNCT) {
        return tok;
      }

      if (tok.type == Token::T_MINUS && expect == EXPECT_OPERATOR) {
        tok.type = Token::T_PUNCT;
        return tok;
      }

      continue;
    }

    if (tok.type == Token::T_MINUS) {
      if (tt == Token::T_NUMBER) {  // Handle negative numbers eg -123
        tok.type = Token::T_NUMBER;
      } else {
        tok.type = Token::T_PUNCT;
      }
    }

    if (tt != tok.type) {
      ungetChar();
      return tok;
    }
    tok.type = tt;

    tok.string += (char)c;
  }
}

void Tokenizer::ungetToken(const Token& tok) { mUnget = tok; }
