/**
 * File: exp.cpp
 * -------------
 * This file implements the exp.h interface.
 */

#include <string>
#include "exp.h"
#include "strlib.h"
#include "error.h"
using namespace std;

/**
 * Implementation notes: Expression
 * --------------------------------
 * The Expression class itself implements only those methods that
 * are not designated as pure virtual.
 */

Expression::Expression() {
   /* Empty */
}

Expression::~Expression() {
   /* Empty */
}

/**
 * Implementation notes: DoubleExp
 * -------------------------------
 * The ConstantExp subclass represents a numeric constant.  The eval
 * method simply returns that value.
 */

DoubleExp::DoubleExp(double value) {
   this->value = value;
}

double DoubleExp::eval(SSModel* model) const {
   return value;
}

string DoubleExp::toString() const {
   return realToString(value);
}

ExpressionType DoubleExp::getType() const {
   return DOUBLE;
}

void DoubleExp::getDependent(Vector<string>& dependents, SSModel* model) const {
    return;
}

double DoubleExp::getDoubleValue() const {
   return value;
}

/**
 * Implementation notes: TextStringExp
 * -----------------------------------
 * The IdentifierExp subclass represents a text string constant.  The
 * implementation of eval simply returns 0.0.
 */

TextStringExp::TextStringExp(const string& str) {
    this->str = str;
}

double TextStringExp::eval(SSModel* model) const {
    return 0.0;
}

string TextStringExp::toString() const {
    return '"' + str + '"';
}

ExpressionType TextStringExp::getType() const {
    return TEXTSTRING;
}

void TextStringExp::getDependent(Vector<string>& dependents, SSModel* model) const {
    return;
}

string TextStringExp::getTextStringValue() const {
    return str;
}

/**
 * Implementation notes: IdentifierExp
 * -----------------------------------
 * The IdentifierExp subclass represents a spreadsheet cell name.  The
 * implementation of eval looks up that name in the spreadsheet model.
 */

IdentifierExp::IdentifierExp(const string& name) {
   this->name = name;
}

double IdentifierExp::eval(SSModel* model) const {
   return model->getCellData(name);
}

string IdentifierExp::toString() const {
   return name;
}

ExpressionType IdentifierExp::getType() const {
   return IDENTIFIER;
}

/**
 * Adds cell name to vector of dependents
 */
void IdentifierExp::getDependent(Vector<string>& dependents, SSModel* model) const {
    dependents.add(name);
}

string IdentifierExp::getIdentifierName() const {
   return name;
}

/**
 * Implementation notes: CompoundExp
 * ---------------------------------
 * The implementation of eval for CompoundExp evaluates the left and right
 * subexpressions recursively and then applies the operator.  Assignment is
 * treated as a special case because it does not evaluate the left operand.
 */

CompoundExp::CompoundExp(const string& op, const Expression *lhs, const Expression *rhs) {
   this->op = op;
   this->lhs = lhs;
   this->rhs = rhs;
}

CompoundExp::~CompoundExp() {
   delete lhs;
   delete rhs;
}

double CompoundExp::eval(SSModel* model) const {
   double right = rhs->eval(model);
   double left = lhs->eval(model);
   if (op == "+") return left + right;
   if (op == "-") return left - right;  
   if (op == "*") return left * right;
   if (op == "/") return left / right; // divide by 0.0 gives ±INF
    
   error("Illegal operator in expression.");
   return 0.0;
}

string CompoundExp::toString() const {
   return '(' + lhs->toString() + ' ' + op + ' ' + rhs->toString() + ')';
}

ExpressionType CompoundExp::getType() const {
   return COMPOUND;
}

void CompoundExp::getDependent(Vector<string>& dependents, SSModel* model) const {
    lhs->getDependent(dependents, model);
    rhs->getDependent(dependents, model);
}

string CompoundExp::getOperator() const {
   return op;
}

const Expression *CompoundExp::getLHS() const {
   return lhs;
}

const Expression *CompoundExp::getRHS() const {
   return rhs;
}

/**
 * Implementation notes: RangeExp
 * ---------------------------------
 * The implementation of eval for RangeExp evaluates range function by calling
 * model->applyRangeFunction() which applies range function on vector of values from start to end cell range.
 */

RangeExp::RangeExp(const string rangeFunctionName, const string startCellLocation, const string endCellLocation) {
    this->rangeFunctionName = rangeFunctionName;
    this->startCellLocation = startCellLocation;
    this->endCellLocation = endCellLocation;
}

RangeExp::~RangeExp() {

}

double RangeExp::eval(SSModel* model) const {
    return model->applyRangeFunction(rangeFunctionName, startCellLocation, endCellLocation);
}

string RangeExp::toString() const {
   return rangeFunctionName + '(' + startCellLocation + ':' + endCellLocation + ')';
}

ExpressionType RangeExp::getType() const {
   return RANGE;
}

/**
 * Collects all cell names from start to end cell range using
 * model->collectCellRef() and adds it to vector of dependents
 */
void RangeExp::getDependent(Vector<string>& dependents, SSModel* model) const {
    model->collectCellRef(dependents, startCellLocation, endCellLocation);
}

string RangeExp::getRangeFunction() const {
    return rangeFunctionName;
}

string RangeExp::getStartCellName() const {
    return startCellLocation;
}

string RangeExp::getEndCellName() const {
    return endCellLocation;
}

/**
 * Implementation notes: EvaluationContext
 * ---------------------------------------
 * The methods in the EvaluationContext class simply call the appropriate
 * method on the map used to represent the symbol table.
 */

void EvaluationContext::setValue(const string& var, double value) {
   symbolTable.put(var, value);
}

double EvaluationContext::getValue(const string& var) const {
   return symbolTable.get(var);
}

bool EvaluationContext::isDefined(const string& var) const {
   return symbolTable.containsKey(var);
}
