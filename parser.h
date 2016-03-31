/**
 * File: parser.h
 * --------------
 * This file acts as the interface to the parser module.
 * File modified to provide support for cell names in spreadsheet and range functions in input expression
 */

#ifndef _parser_
#define _parser_

#include <string>
#include "exp.h"
#include "tokenscanner.h"
#include "ssmodel.h"

/**
 * Function: parseExp
 * Usage: Expression *exp = parseExp(scanner, model);
 * -------------------------------------------
 * Parses a complete expression from the specified TokenScanner object,
 * making sure that there are no tokens left in the scanner at the end.
 */

Expression *parseExp(TokenScanner& scanner, SSModel* model);

#endif
