/*
 * File: parser.cpp
 * ----------------
 * This file implements the parser.h interface.
 */

#include <iostream>
#include <string>
#include "error.h"
#include "exp.h"
#include "parser.h"
#include "strlib.h"
#include "tokenscanner.h"
using namespace std;

static Expression *readE(TokenScanner& scanner, SSModel* model, int prec = 0);
static Expression *readT(TokenScanner& scanner, SSModel* model);
static int precedence(const std::string& token);

/**
 * Implementation notes: parseExp
 * ------------------------------
 * This code just reads an expression and then checks for extra tokens.
 */

Expression *parseExp(TokenScanner& scanner, SSModel* model) {
   Expression *exp = readE(scanner, model);
   if (scanner.hasMoreTokens()) {
      error("Unexpected token \"" + scanner.nextToken() + "\"");
   }
   return exp;
}

/**
 * Implementation notes: readE
 * Usage: exp = readE(scanner, prec);
 * ----------------------------------
 * The implementation of readE uses precedence to resolve the ambiguity in
 * the grammar.  At each level, the parser reads operators and subexpressions
 * until it finds an operator whose precedence is greater than the prevailing
 * one.  When a higher-precedence operator is found, readE calls itself
 * recursively to read that subexpression as a unit.
 */

Expression *readE(TokenScanner& scanner, SSModel* model, int prec) {
   Expression *exp = readT(scanner, model);
   string token;
   while (true) {
      token = scanner.nextToken();
      int tprec = precedence(token);
      if (tprec <= prec) break;
      Expression *rhs = readE(scanner, model, tprec);
      exp = new CompoundExp(token, exp, rhs);
   }
   scanner.saveToken(token);
   return exp;
}

/**
 * Implementation notes: readT
 * ---------------------------
 * This function scans a term, which is either an integer, an identifier,
 * or a parenthesized subexpression.
 * If token type is WORD: (1) if token is valid spreadsheet cell name, then Identifier expression is created.
 *                        (2) if token is valid range function, then it checks the cell references following range function and
 *                            if it is correct, then RangeExp is created.
 * Error is thrown for any malformed function
 */
Expression *readT(TokenScanner& scanner, SSModel* model) {
   string token = scanner.nextToken();
   TokenType type = scanner.getTokenType(token);
   if (type == WORD) {
      if (model->nameIsValid(token)) {
          return new IdentifierExp(toUpperCase(token));
      } else if (model->rangeFnIsValid(token)) {
          string rangeToken = scanner.nextToken();
          if (rangeToken != "(") {
             error("Unexpected token \"" + rangeToken + "\" following range function \"" + token + "\"");
          }
          string startCell = scanner.nextToken();
          if (scanner.getTokenType(startCell) != WORD || !model->nameIsValid(startCell)) {
              error("Missing valid spreadsheet start cell refernce");
          }
          rangeToken = scanner.nextToken();
          if (rangeToken != ":") {
             error("Unexpected token \"" + rangeToken + "\" following range function \"" + token + "\"");
          }
          string endCell = scanner.nextToken();
          if (scanner.getTokenType(endCell) != WORD || !model->nameIsValid(endCell)) {
              error("Missing valid spreadsheet end cell refernce");
          }
          rangeToken = scanner.nextToken();
          if (rangeToken != ")") {
             error("Unbalanced parentheses following range function");
          }
          if (!model->validRange(startCell, endCell)) {
              error("Invalid spreadsheet range input from " + startCell + " to " + endCell);
          }
          return new RangeExp(toLowerCase(token), toUpperCase(startCell), toUpperCase(endCell));
      } else {
          error("Unexpected token \"" + token + "\"");
      }
   }
   if (type == NUMBER) return new DoubleExp(stringToReal(token));
   if (type == STRING) return new TextStringExp(token.substr(1, token.length() - 2));
   if (token != "(") error("Unexpected token \"" + token + "\"");
   Expression *exp = readE(scanner, model, 0);
   if (scanner.nextToken() != ")") {
      error("Unbalanced parentheses");
   }
   return exp;
}

/**
 * Implementation notes: precedence
 * --------------------------------
 * This function checks the token against each of the defined operators
 * and returns the appropriate precedence value.
 */
int precedence(const string& token) {
   if (token == "+" || token == "-") return 1;
   if (token == "*" || token == "/") return 2;
   return 0;
}
