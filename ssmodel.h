/**
 * CS-106X, Assignment-8, Stanford 1-2-3
 * Modified by Akanksha Jain
 * File: ssmodel.h
 * ------------------
 *
 * Submission-2, GUI extension
 * Submission-1, No extension
 * This submission is with extension to Submission-1
 * This file defines the interface for SSModel class used in the
 * Stanford 1-2-3 spreadsheet program.
 * GUI Extension: SSView and SSController are modified for GUI extension.
 * Check ssview.h, ssview.cpp and sscontroller.cpp for added methods and descriptions
 */

#ifndef _ssmodel_
#define _ssmodel_

#include <fstream>
#include "tokenscanner.h"
#include "ssview.h"
#include "ssutil.h"
#include "map.h"
#include "basicgraph.h"
using namespace std;

/**
 * This is a "forward reference" which informs the compiler there is a
 * class named Expression that we'll eventually get around to seeing the
 * full class interface for, but for now, we just need to know such a class
 * exists.
 */

class Expression;

/**
 * The celldata struct used to represent and store data(cache data) corresponding to valid cell in spreadsheet
 * Values Stored: (1): Expression* cellExpression, expression generated for spreadsheet cell from input equation
 *                (2): string displayValue: Value to be displayed on spreadsheet, evaluated value for a cell or string if it is string
 *                (3): double value: actual numeric value of a cell, 0.0 in case of string or empty cell, else evaluated expression value
 * Expression is stored so when parent cell changes its value, dependent cells can recalculate its expression value and cache them
 */

struct celldata {
    Expression* exp;
    string displayValue;
    double value;
} ;

/**
 * Class: SSModel
 * --------------
 * This is the interface for the spreadsheet data model class.  The
 * public member functions listed here are used by other parts of the given code 
 * (i.e. controller/view).
 */

class SSModel {
public:
		
/**
 * Constructor: SSModel
 * Usage: SSModel model(10, 20, vp);
 * --------------------------------
 * The constructor initializes a new empty data model of the given size 
 * which is to be displayed using the given SSView object. Whenever data 
 * in the model changes, this view is notified to re-display the
 * affected cells.
 */
    
    SSModel(int nRows, int nCols, SSView *view);

/**
 * Destructor: ~SSModel
 * Usage: delete mp;
 * -----------------
 * The destructor deallocates any storage for this model.
 */
    
	~SSModel();
		
/**
 * Member function: nameIsValid
 * Usage: if (model.nameIsValid(name))...
 * ------------------------------------------
 * This member function returns true if name is a valid string
 * name of a cell in this model, false otherwise. The string
 * must be in the proper format (column letter followed by row number)
 * and refer to a cell location within bounds for this model.
 */

    bool nameIsValid(const std::string& name) const;

/**
 * Member function: rangeFnIsValid
 * Usage: if (model.rangeFnIsValid(name))...
 * ------------------------------------------
 * This member function returns true if name passed as string is a valid range function from ssutil
 * Uses functionTable Map to check if the name is s valid key i.e. a valid range function
 * Used by parser.cpp to check if the input passed is a valid range function or not.
 */

    bool rangeFnIsValid(const std::string& name) const;

/**
 * Member function: validRange
 * Usage: if (model.validRange(startCellRefName, endCellRefName))...
 * ------------------------------------------
 * This member function returns true if range from start cell to end cell is a valid range
 * A range is valid if end row and column is at least equal to start row and column
 */

    bool validRange(const std::string& startCellName, const std::string& endCellName) const;

 /**
  * Member function: setCellFromScanner
  * Usage: model.setCellFromScanner("A1", scanner);
  * -----------------------------------------------
  * This member function reads an expression from the scanner and
  * stores it as the the contents for the named cell.  If there is
  * any problem with setting the cell's value (the expression is
  * malformed, contains a circular reference, etc.) error is called
  * and the cell's contents are unchanged.  If the contents were
  * successfully updated, the new cell is displayed in the view
  * and its dependent cells are updated as well.
  */
	
    void setCellFromScanner(const std::string& cellname, TokenScanner& scanner);

/**
 * Member function: printCellInformation
 * Usage: model.printCellInformation("A1");
 * ----------------------------------------
 * This member function prints the cell information for the named
 * cell to cout.  Cell information includes its current contents
 * and dependencies (and other information you choose to include).
 */

    void printCellInformation(const std::string& cellname);

/**
 * Member function: getCellData()
 * Usage: model.getCellData("A1");
 * ----------------------------------------
 * This member function returns numeric value to a corresponding valid spreadsheet cell
 * If cell is empty or contains string, value returned is 0.0 else cached numeric value is returned
 */

    double getCellData(const string& cellname) const;

/**
 * Member function: applyRangeFunction
 * Usage: model.applyRangeFunction(sum, "A1", "D4");
 * ----------------------------------------
 * This member function applies input range function to cells ranging from start to end spreadsheet cell.
 * After applying range function, the result of that is returned to the caller function.
 */

    double applyRangeFunction(const string rangeFunctionName, const string startCellLocation, const string endCellLocation);

/**
 * Member function: collectCellRef
 * Usage: model.collectCellRef(cellReferencesVector, "A1", "D4");
 * ----------------------------------------
 * This member function iterates through spreadsheet map(spreadsheet model) and
 * collects all references ranging from start cell to end cell.
 * The range of cells from start to end is returned as a vector of string representing cell names.
 */

    void collectCellRef(Vector<string>& cellRefs, const string startCellLocation, const string endCellLocation);

/**
 * Member functions: writeToStream, readFromStream
 * Usage: model.writeToStream(outfile);
 *        model.readFromStream(infile);
 * --------------------------------
 * These member functions read/write model contents
 * to/from a stream.  The stream is assumed to be valid and open.
 * The file format consists of cell values, one per line, like this:
 *
 *      A1 = 3
 *      A2 = 4 * (A1 + 8)
 *      A3 = "a string"
 *
 * error is called if there is any trouble reading/writing
 * the file.
 */

    void writeToStream(std::ostream &outfile) const;
	void readFromStream(std::istream &infile);

/**
 * Member function: clear
 * Usage: model.clear();
 * ----------------------------------------
 * Resets and clears spreadsheet model.
 * Clears the spreadsheet map as well as the dependency graphs associated with spreadsheet
 * ssview is called to display empty spreadsheet
 */

    void clear();

/**
 * Private member variables and functions of spreadsheet model
 */
private:

/**
 * Map<string cellName, celldata cellData> spreadsheet;
 * Mappings: "A1" => {Expression* exp, string displayValue, double numericValue}
 * spreadsheet model is represented as a map so cells are created on demand
 * Keys are strings representing valid cell names in uppercase
 * Key values are stored as celldata struct to cache calculated expression, expression value and display value to be displayed on spreadsheet
 * If a cell name is valid but is not present as a key in map, that represents empty cell
 */

    Map<string, celldata> spreadsheet;

/**
 * totalRows: total number of rows spreadsheet contains
 * actual usable number of rows is 1 less than totalRows because of one row used for labelling of spreadsheet
 */

    int totalRows;

/**
 * totalCols: total number of columns spreadsheet contains numbered starting from 'A'
 */

    int totalCols;

/**
 * SSView* view: pointer to ssview so as to update spreadsheet display whenever changes are made to spreadsheet
 */

    SSView* view;

/**
 * BasicGraph graph: directed graph to represent dependency between spreadsheet cells
 * The edge arrow reprsents dependent cell and edge tail represent the dependency(parent) cell
 * Vertices and edges are created on demand in graph as new spreadsheet cells are defined
 */

    BasicGraph graph;

/**
 * Map<string rangeFunctionName, rangeFnT rangeFunction> fnTable
 * Map represents mapping from string representing range function to actual range function definition
 * This works same as cmdFnT
 * The map is initialized by calling setUpRangeTable() method added in ssutil
 */

    Map<string, rangeFnT> fnTable;

/**
 * Map<string cellName, Set<string> cellNameAdjacentVertices> incomingNeighbors
 * This map needs to be defined to keep track of incoming dependencies of any cell
 * graph defined above keeps track of arcs in only one direction
 * This map keep track of arcs in opposite direction
 */

    Map<string, Set<string>> incomingNeighbors;

/**
 * Member function: evaluateExpression
 * Usage: evaluateExpression("A1", expression*);
 * ---------------------------------------------
 * Given cell name and expression for cell as input, evaluates expression value by calling eval(SSModel* model) of exp.cpp
 * Also gets the expression string by calling toString() method of exp.cpp
 * Caches the evaluated value and display value in spreadsheet map
 */
    void evaluateExpression(const string& cellname, Expression* exp);

/**
 * Member function: collectCellValues
 * Usage: collectCellValues(Vector<double>& cellValues, "A1", "D4");
 * ---------------------------------------------
 * Defines vector of double values and collect cell names by calling collectCellRef() between given range
 * Adds the numeric value of each cells in range of given input to the vector defined
 */

    void collectCellValues(Vector<double>& cellValues, const string startCellLocation, const string endCellLocation);

/**
 * Member function: addDataToGraph
 * Usage: addDataToGraph("A1", {"B1, C2, D3"});
 * ---------------------------------------------
 * Adds vertex and neighboring vertices and depedency arcs between them in member graph
 * Also adds incoming depedency arcs to incomingNeighbors map
 */

    void addDataToGraph(const string& cellname, Vector<string>& dependents);

/**
 * Member function: setLinesFromFile
 * Usage: setLinesFromFile(scanner);
 * ---------------------------------------------
 * Input: Scanner containing string representing each line in input file to be read
 * Passes the input string to setCellFromScanner() method to parse and evaluate each cell from input file
 * Throws error if malformed input line
 */

    void setLinesFromFile(TokenScanner& scanner);

/**
 * Member function: checkForCycle
 * Usage: if(checkForCycle("A1", {"B1", "C1", D1"}));
 * ---------------------------------------------
 * Checks if adding the new cell and its corresponding vertex will create a cycle in graph
 * If cycle is created either through direct or indirect dependency then formula is rejected
 */

    bool checkForCycle(const string& cellname, const Vector<string>& dependents);

/**
 * Member function: dfsRecursive
 * Usage: if(dfsRecursive("D1", "A1"));
 * ---------------------------------------------
 * Do Depth first search on dependency graph from input start vertex
 * If end cell(vertex) is visited during DFS, returns true else returns false
 */

    bool dfsRecursive(const string& start, const string& end);

/**
 * Member function: topologicalSort
 * Usage: topologicalSort(Vertex* A1, {});
 * ---------------------------------------------
 * Do DFS from start vertex and visits all neigboring vertices to do topological sort
 * After all vertices from input vertex is visited, vertex is added to stack
 * Topological sort helps in updating cells dependent on input cell only once
 * Returns Stack with vertices added in topologically sorted manner
 */

    void topologicalSort(Vertex* startNode, Stack<string>& topologicalOrder);

};

#endif
