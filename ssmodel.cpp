/**
 * CS-106X, Assignment-8, Stanford 1-2-3
 * Modified by Akanksha Jain
 * File: ssmodel.cpp
 * ------------------
 *
 * Submission-2, GUI extension
 * Submission-1, No extension
 * This submission is with extension to Submission-1
 * This file contains the implementation of the SSModel class member functions
 * GUI Extension: SSView and SSController are modified for GUI extension.
 * Check ssview.h, ssview.cpp and sscontroller.cpp for added methods and descriptions
 */
 
#include "ssmodel.h"
#include "exp.h"
#include "parser.h"
#include "strlib.h"
#include "filelib.h"
#include <cctype>

using namespace std;

/**
 * Initializes member variables and calls setUpRangeTable in ssutil to initialize rangeFunction map
 */
SSModel::SSModel(int nRows, int nCols, SSView *view) {
    this->totalRows = nRows;
    this->totalCols = nCols;
    this->view = view;
    setUpRangeTable(fnTable);
}

/**
 * Empty; no new variables are created in this model
 * Expression classes have their own destructor
 */
SSModel::~SSModel() {

}

/**
 * @brief SSModel::nameIsValid
 * @param cellname: spreadsheet cell name
 * returns true of name is valid cell name i.e. lies between the valid row and column range of spreadsheet; else returns false
 * calls stringToLocation in ssutil to do basic cell name check
 */
bool SSModel::nameIsValid(const string& cellname) const {
    location loc;
    if (!stringToLocation(cellname, loc)) {
        return false;
    }
    if (loc.col < 'A' || loc.col >= 'A' + totalCols) {
        return false;
    }
    if (loc.row < 0 || loc.row >= totalRows) {
        return false;
    }
    return true;
}

/**
 * @brief SSModel::rangeFnIsValid
 * @param name: range function name
 * return true if input name is a valid range function i.e. present as a key in rangeFunction map
 */
bool SSModel::rangeFnIsValid(const std::string& name) const {
    if (fnTable.containsKey(toLowerCase(name))) {
        return true;
    } else {
        return false;
    }
}

/**
 * @brief SSModel::validRange
 * @param startCellName
 * @param endCellName
 * return true if the range between start cell and end cell refernce is valid, else return false
 * Range is valid if:  (1) name of both start and end cell is valid
 *                     (2) endCell.row >= startCell.row
 *                     (3) endCell.col >= startCell.col
 */
bool SSModel::validRange(const std::string& startCellName, const std::string& endCellName) const {
    if (!nameIsValid(startCellName) || !nameIsValid(endCellName)) {
        return false;
    }
    location startLoc;
    stringToLocation(startCellName, startLoc);
    location endLoc;
    stringToLocation(endCellName, endLoc);
    if (endLoc.col < startLoc.col) {
        return false;
    }
    if (endLoc.row < startLoc.row) {
        return false;
    }
    return true;
}

/**
 * @brief SSModel::setCellFromScanner
 * @param cellname: lhs spreadsheet cell
 * @param scanner
 * Parses the input expression from token scanner by calling parseExp() on parser.cpp
 * Collect all parent cells on which this cell is directly dependent by calling getDependent() method on exp.cpp
 * Checks if evaluation of this expression would create a cycle in graph and if it does then throws error
 * Adds Data to graph i.e. cell vertices and dependency arcs by calling addDataToGraph
 * Calls evaluateExpression() method to evaluate value of expression from exp.cpp
 * Does topological sorting on graph starting from input cellname vertex and update all vertices dependent on this cell value
 * by evaluation their expression value again.
 */
void SSModel::setCellFromScanner(const string& cellname, TokenScanner& scanner) {
    Expression* exp = parseExp(scanner, this);
    Vector<string> dependents;
    exp->getDependent(dependents, this);
    string cellNameUpper = toUpperCase(cellname);
    if (checkForCycle(cellNameUpper, dependents)) {
        error("Invalid action: Cell formula would introduce cycle.");
    }
    addDataToGraph(cellNameUpper, dependents);
    celldata data;
    evaluateExpression(cellNameUpper, exp);
    Stack<string> topologicalOrder;
    Vertex* startNode = graph.getVertex(cellNameUpper);
    graph.resetData();
    topologicalSort(startNode, topologicalOrder);
    topologicalOrder.pop();
    while (!topologicalOrder.isEmpty()) {
        string nodeName = topologicalOrder.pop();
        evaluateExpression(nodeName, spreadsheet[nodeName].exp);
    }
}

/**
 * @brief SSModel::evaluateExpression
 * @param cellname: lhs spreadsheet cell
 * @param exp
 * Evaluates the value of cell expression by calling eval() method on exp.cpp
 * Also gets the evaluated value and string display value from exp.cpp and stores them in celldata struct
 * Adds the key and its value in spreadsheet model map
 * Updates the display in spreadsheet by calling displayCell() method on ssview
 */
void SSModel::evaluateExpression(const string& cellname, Expression* exp) {
    string cellNameUpper = toUpperCase(cellname);
    double value = exp->eval(this);
    celldata data;
    data.value = value;
    if (exp->getType() == TEXTSTRING) {
        data.displayValue = ((TextStringExp*) exp)->getTextStringValue();
    } else {
        data.displayValue = doubleToString(value);
    }
    data.exp = exp;
    if (spreadsheet.containsKey(cellNameUpper)) {
        spreadsheet.remove(cellNameUpper);
    }
    spreadsheet[cellNameUpper] = data;
    view->displayCell(cellNameUpper, data.displayValue);
}

/**
 * @brief SSModel::addDataToGraph
 * @param cellname: input cellname vertex(i.e. lhs spreadsheet cell)
 * @param dependents: Vector of cell name on which lhs is dependent
 * Creates a new vertex for cell which are not present already
 * Removes existing dependencies for already existing vertex
 * Adds edges originating from vertices in vector of dependents to input cell vertex to represent dependencies
 * Also adds vertices from vector of dependents to incomingNeighbors map with key as input cellname to
 * represent incoming dependencies
 */
void SSModel::addDataToGraph(const string& cellname, Vector<string>& dependents) {
    if (!graph.containsVertex(cellname)) {
        Vertex* lhs = new Vertex(cellname);
        graph.addVertex(lhs);
    } else {
        for (string neighbor : incomingNeighbors[cellname]) {
            if (graph.containsEdge(neighbor, cellname)) {
                graph.removeEdge(neighbor, cellname);
            }
        }
        incomingNeighbors[cellname].clear();
    }
    Vertex* lhsNode = graph.getVertex(cellname);
    for (string depCell : dependents) {
        Vertex* neighbor;
        if (graph.containsVertex(depCell)) {
            neighbor = graph.getVertex(depCell);
        } else {
            neighbor = new Vertex(depCell);
            graph.addVertex(neighbor);
        }
        Edge* e = new Edge(neighbor, lhsNode);
        graph.addEdge(e, true);
        incomingNeighbors[cellname].add(depCell);
    }
}

/**
 * @brief SSModel::checkForCycle
 * @param cellname: input cellname vertex(i.e. lhs spreadsheet cell)
 * @param dependents: Vector of cell name on which lhs is dependent
 * return true if the expression creates a cycle in graph else false
 * DFS is done from each vertex in dependent vector to see if input cellname vertex can be reached
 * If it can be reached then cycle exists else not
 * Calls dfsRecursive() to do DFS
 */
bool SSModel::checkForCycle(const string& cellname, const Vector<string>& dependents) {
    graph.resetData();
    for (string s : dependents) {
        if (graph.containsVertex(s) && dfsRecursive(s, cellname)) {
            return true;
        }
    }
    //Special case of vertex directly depending on itself
    if (dependents.size() == 1 && dependents[0] == cellname) {
        return true;
    }
    return false;
}

/**
 * @brief SSModel::dfsRecursive
 * @param start: each vertex in dependent vector of vertices
 * @param end: input cellname vertex(i.e. lhs spreadsheet cell)
 * DFS is done recursively from each vertex in dependent vector to input cellname vertex
 * If end can be reached then it returns true which implies cycle will be created in graph
 */
bool SSModel::dfsRecursive(const string& start, const string& end) {
    Vertex* startNode = graph.getVertex(start);
    if (start == end) {
        return true;
    }
    startNode->visited = true;
    for (string neighbor : incomingNeighbors[start]) {
        Vertex* vertex = graph.getVertex(neighbor);
        if (!vertex->visited) {
            if (dfsRecursive(neighbor, end)) {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief SSModel::topologicalSort
 * @param startNode: input cellname vertex(i.e. lhs spreadsheet cell)
 * @param topologicalOrder: order in which vertices will be visited when topological sort is done from startNode
 * Same as DFS except that vertex is pushed to stack after all neighboring vertices are processed
 */
void SSModel::topologicalSort(Vertex* startNode, Stack<string>& topologicalOrder) {
    startNode->visited = true;
    for (Vertex* vertex : graph.getNeighbors(startNode)) {
        if (!vertex->visited) {
            topologicalSort(vertex, topologicalOrder);
        }
    }
    topologicalOrder.push(startNode->name);
}

/**
 * Described in ssmodel.h
 */
double SSModel::getCellData(const string& cellname) const {
    string cellNameUpper = toUpperCase(cellname);
    if (spreadsheet.containsKey(cellNameUpper)) {
        return spreadsheet[cellNameUpper].value;
    } else {
        return 0.0;
    }
}

/**
 * Described in ssmodel.h
 */
double SSModel::applyRangeFunction(const string rangeFunctionName, const string startCellLocation, const string endCellLocation) {
    Vector<double> cellValues;
    collectCellValues(cellValues, startCellLocation, endCellLocation);
    return fnTable[toLowerCase(rangeFunctionName)](cellValues);
}

/**
 * Described in ssmodel.h
 */
void SSModel::collectCellValues(Vector<double>& cellValues, const string startCellLocation, const string endCellLocation) {
    Vector<string> cellRefs;
    collectCellRef(cellRefs, startCellLocation, endCellLocation);
    for (string key : cellRefs) {
        cellValues.add(getCellData(key));
    }
}

/**
 * Described in ssmodel.h
 */
void SSModel::collectCellRef(Vector<string>& cellRefs, const string startCellLocation, const string endCellLocation) {
    if (validRange(startCellLocation, endCellLocation)) {
        location startLoc;
        stringToLocation(startCellLocation, startLoc);
        location endLoc;
        stringToLocation(endCellLocation, endLoc);
        location loc;
        for (char c = startLoc.col; c <= endLoc.col; c++) {
            for (int r = startLoc.row; r <= endLoc.row; r++) {
                loc.col = c;
                loc.row = r;
                string cellKey = locationToString(loc);
                cellRefs.add(cellKey);
            }
        }
    }
}

/**
 * @brief SSModel::printCellInformation
 * @param cellname: input cellname for which information needs to be retrieved
 * If cellname key doesnot exist in map, then cell is empty, else
 * numeric value is retrieved from spreadsheet map
 * Cells on which cellname directly depends is retrieved from incomingNeighbors map
 * Cells which directly depends on cellname is retrieved by getting adjacent neighboring vertices of cellname from graph
 */
void SSModel::printCellInformation(const string& cellname) {
    string key = toUpperCase(cellname);
    if (spreadsheet.containsKey(key)) {
        cout << key << " = " << spreadsheet[key].exp->toString() << endl;
        string incoming = "";
        string outgoing = "";
        for (string neighbor : incomingNeighbors[key]) {
            incoming += neighbor + " ";
        }
        for (Vertex* v : graph.getNeighbors(key)) {
            outgoing += v->name + " ";
        }
        trimInPlace(incoming);
        trimInPlace(outgoing);
        cout << "Cells that " << key << " directly depends on: " << incoming << endl;
        cout << "Cells that directly depend on " << key << ": " << outgoing << endl;
    } else {
        cout << key << " is empty." << endl;
    }
}

/**
 * @brief SSModel::writeToStream
 * @param outfile
 * writes cached value of expression in spreadsheet map to outfile stream
 */
void SSModel::writeToStream(ostream& outfile) const {
    for (string cellname : spreadsheet) {
        string line = cellname + " = " + spreadsheet[cellname].exp->toString();
        outfile << line << endl;
    }
}

/**
 * @brief SSModel::readFromStream
 * @param infile
 * reads each line from file in token scanner and passes it to setLinesFromFile() for processing
 * Details in ssmodel.h
 */
void SSModel::readFromStream(istream& infile) {
    Vector<string> lines;
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.scanStrings();
    readEntireFile(infile, lines);
    for (string line : lines) {
        scanner.setInput(line);
        setLinesFromFile(scanner);
    }
}

/**
 * @brief SSModel::setLinesFromFile
 * @param scanner
 * Processes expression by using setCellFromScanner() method
 * Details in ssmodel.h
 */
void SSModel::setLinesFromFile(TokenScanner& scanner) {
    if (!scanner.hasMoreTokens())
        error("The set command requires a cell name and a value.");
    string cellname = scanner.nextToken();
    if (!nameIsValid(cellname))
        error("Invalid cell name " + cellname);
    if (scanner.nextToken() != "=")
        error("= expected.");
    setCellFromScanner(cellname, scanner);
}

/**
 * @brief SSModel::clear
 * Displays empty spreadsheet
 * Clears spreadsheet map
 * Clears incomingNeighbors map
 * Clears dependency graph
 */
void SSModel::clear() {
    view->displayEmptySpreadsheet();
    graph.clear();
    incomingNeighbors.clear();
    spreadsheet.clear();
}
