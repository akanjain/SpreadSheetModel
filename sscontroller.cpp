/**
 * Modified by Akanksha Jain
 * File: sscontroller.cpp
 * ----------------------
 * This file implements the command-line controller for the Stanford 1-2-3 program.
 *
 * File modified for GUI extension
 * Choosers and buttons are added through SSView
 * Controller catches event type and calls required functions in ssmodel
 *
 * Fall 2007 Julie Zelenski (all initial work)
 * Autumn 2009 Jerry Cain (upgrade to use new libraries)
 * Autumn 2012 Jerry Cain (upgrade to use ever newer libraries)
 */

#include <iostream>
#include <fstream>
#include <iomanip>

#include "simpio.h"
#include "map.h"
#include "tokenscanner.h"
#include "error.h"

#include "ssutil.h"
#include "ssmodel.h"
#include "ssview.h"
#include "gevents.h"
#include "filelib.h"
using namespace std;

static const string baseDirectory = "spreadsheets"; /*base directory where spreadsheet is saved*/

/**
 * General implementation notes
 * ----------------------------
 * The controller module implements the command-line interface to the spreadsheet
 * program.  The interface can easily be extended with additional
 * commands by writing a handler function for the command and then adding 
 * an entry to the table.
 */

/**
 * Typedef: cmdFnT
 * ---------------
 * This typedef makes a shorthand name for the function prototype
 * so we can refer to it as cmdFnT instead of the long form 
 */

typedef void (*cmdFnT)(TokenScanner& scanner, SSModel& model);

/**
 * Function: helpAction, loadAction, saveAction, etc.
 * ---------------------------------------------------------
 * Each of these handles one specific command.  In most cases, it is
 * simply a matter of using the scanner to read the following arguments and
 * then acting on the command by sending the appropriate message to the model.
 */

static const int kLeftColumnWidth = 22;
static void helpAction(TokenScanner& scanner, SSModel& model) {
	cout << left << setw(kLeftColumnWidth) 
         << "help" << "Print this menu of commands" << endl;
	cout << left << setw(kLeftColumnWidth) 
         << "load <filename>" << "Read named file into spreadsheet" << endl;
	cout << left << setw(kLeftColumnWidth) 
         << "save <filename>" << "Save current spreadsheet to named file" << endl;
	cout << left << setw(kLeftColumnWidth) 
         << "set <cell> = <value>" 
         << "Set cell to value. Value can be \"string\" or formula" << endl;
	cout << left << setw(kLeftColumnWidth) 
         << "get <cell>" << "Print contents of cell" << endl;
    cout << left << setw(kLeftColumnWidth)
         << "quit" << "Quit the program" << endl;
    cout << left << setw(kLeftColumnWidth)
         << "clear" << "Clear the spreadsheet" << endl;
	cout << endl;
}

/**
 * Clears the spreadsheet by calling model.clear() method on spreadsheet model
 */
static void clearAction(TokenScanner& scanner, SSModel& model) {
    model.clear();
    cout << "Cleared spreadsheet." << endl;
}

static void loadAction(TokenScanner& scanner, SSModel& model) {
	if (!scanner.hasMoreTokens()) 
        error("The load command requires a file name.");
	
    string filename;
	while (scanner.hasMoreTokens())
		filename += scanner.nextToken();
	ifstream infile(filename.c_str());
	if (infile.fail()) 
        error("Cannot open the file named \"" + filename + "\".");
	model.readFromStream(infile);
	cout << "Loaded file \"" << filename << "\"." << endl;
}

static void saveAction(TokenScanner& scanner, SSModel& model) {
	if (!scanner.hasMoreTokens()) 
        error("The save command requires a file name.");
    
	string filename;
	while (scanner.hasMoreTokens())
        filename += scanner.nextToken();
	ofstream out(filename.c_str());
	if (out.fail()) 
        error("Cannot open the file named \"" + filename + "\".");
	model.writeToStream(out);
    cout << "Saved file \"" << filename << "\"." << endl;
}

static void setAction(TokenScanner& scanner, SSModel& model) {
	if (!scanner.hasMoreTokens()) 
        error("The set command requires a cell name and a value.");
	string cellname = scanner.nextToken();
	if (!model.nameIsValid(cellname)) 
        error("Invalid cell name " + cellname);
	if (scanner.nextToken() != "=") 
        error("= expected.");
	model.setCellFromScanner(cellname, scanner);
}

static void getAction(TokenScanner& scanner, SSModel& model) {
	if (!scanner.hasMoreTokens()) 
        error("The get command requires a cell name.");
	string cellname = scanner.nextToken();
	if (!model.nameIsValid(cellname)) error("Invalid cell name " + cellname);
	model.printCellInformation(cellname);
}

static const double kHaltTimeBeforeWindowClose = 3000;
static void quitAction(TokenScanner& scanner, SSModel& model) {
    cout << "Thanks for using Stanford 1-2-3.  " 
         << "Closing window in " << kHaltTimeBeforeWindowClose/1000 << " "
         << "seconds...." << endl;
    pause(kHaltTimeBeforeWindowClose);
	exitGraphics();
}

/**
 * Function: setUpCommandTable
 * ---------------------------
 * Adds all commands into the table.
 */

static void setUpCommandTable(Map<string, cmdFnT>& table) {
    // store map entries using lowercase, always use lowercase to lookup
    table["help"] = helpAction;
    table["load"] = loadAction;
    table["save"] = saveAction;
    table["set"] = setAction;
    table["get"] = getAction;
    table["quit"] = quitAction;
    table["clear"] = clearAction;
}

/**
 * @brief executeCommand: executes input command
 * @param cmdName: command to be executed
 * @param cmdTable: cmdTable map to call correct function for input command
 * @param scanner: token scanner
 * @param model: ssmodel
 * uses try and catch to catch any error generated by malformed input and catched by model or parser
 */
static void executeCommand(string cmdName, Map<string, cmdFnT>& cmdTable, TokenScanner& scanner, SSModel& model) {
    if (!cmdTable.containsKey(cmdName)) {
        cout << "Unrecognized command \"" + cmdName
        << "\". Type \"help\" for list of commands." << endl;
    } else {
        try {
            cmdTable[cmdName](scanner, model);
        } catch (ErrorException ex) {
            cout << "Error in " << cmdName << " command: "
            << ex.getMessage() << endl;
        }
    }
    cout << endl;
}

/**
 * @brief processActionEvent: Helper method for processing action event through chooser and button
 * @param actionEvent: action event generated
 * @param cmdTable: cmdTable map to call correct function for input command
 * @param view: spreadsheet graphical view
 * @param model: spreadsheet model
 * @param scanner: token scanner
 * Executes commands selected through chooser on spreadsheet
 * If chooser = console, all commands should be entered through console(for default model with no extension)
 *            = command, single line command through console
 *            = load, loads selects spreadsheet
 *            = save, save spreadsheet with input name entered and also adds it to sheet chooser
 *            = get, prints cells information for selected cell through chooser
 *            = set, sets selected cell through chooser to input expression entered through console
 *            = help, prints help menu
 *            = clear, clears spreadsheet
 *            = quit, quits console and program
 */
static void processActionEvent(GActionEvent& actionEvent, Map<string, cmdFnT>& cmdTable, SSView& view, SSModel& model, TokenScanner& scanner) {
    string cmd = actionEvent.getActionCommand();
    if (cmd == "Execute") {
        string chooser = view.getCommandName();
        string cmdName = chooser;
        string cellName = view.getCellName();
        string sheetName = view.getSheetName();
        if (chooser == "command") {
            string command = getLine("Enter command: ");
            scanner.setInput(command);
            cmdName = toLowerCase(trim(scanner.nextToken()));
            executeCommand(cmdName, cmdTable, scanner, model);
        } else if (chooser == "console") {
            while (true) {
                string command = getLine("Enter command(enter \"exit\" to leave console mode): ");
                if (command == "exit") {
                    cout << endl;
                    break;
                }
                scanner.setInput(command);
                cmdName = toLowerCase(trim(scanner.nextToken()));
                executeCommand(cmdName, cmdTable, scanner, model);
            }
        } else if (chooser == "load") {
            scanner.setInput(sheetName);
            executeCommand(cmdName, cmdTable, scanner, model);
        } else if (chooser == "get") {
            scanner.setInput(cellName);
            executeCommand(cmdName, cmdTable, scanner, model);
        } else if (chooser == "set") {
            string rhs = getLine("Enter expression to be set(RHS of expression): ");
            string command = cellName + " = " + rhs;
            scanner.setInput(command);
            executeCommand(cmdName, cmdTable, scanner, model);
        } else if (chooser == "save") {
            string fileName = getLine("Enter fileName to be saved: ");
            scanner.setInput(fileName);
            executeCommand(cmdName, cmdTable, scanner, model);
            view.addSheetToChooser(fileName);
        } else {
            executeCommand(cmdName, cmdTable, scanner, model);
        }
    }
}

/**
 * Function: interpretCommands
 * ---------------------------
 * The loop in this function gets a command entered by the user through console or
 * through chooser or through spreadsheet and uses a
 * command dispatch table to determine how to act on the request.
 * Gets events and event types to act accordingly.
 * EventClass = TABLE_EVENT
 *                  = TABLE_UPDATED, sets cell to entered expression
 *                  = TABLE_SELECTED, gets cell information for selected cell
 *            = ACTION_EVENT, processes button and chooser event through processActionEvent();
 *            = WINDOW_EVENT, quits program on WINDOW_CLOSED
 */

static void interpretCommands(Map<string, cmdFnT>& cmdTable) {
	SSView view;
	SSModel model(kNumRowsDisplayed, kNumColsDisplayed, &view);
	TokenScanner scanner;
	scanner.ignoreWhitespace();
	scanner.scanNumbers();
	scanner.scanStrings();
	
    cout << "Welcome to Stanford 1-2-3.  Select \"help\" from chooser and execute for list of commands."
         << endl << endl;
    cout << "\"Either use table directly\" or \"select one option from chooser and click execute\" or \"close window to exit\"." << endl;
    cout << "To use console for entering commands, select \"console\" from chooser and execute." << endl;
    cout << "To enter single command on console, select \"command\" from chooser and execute." << endl;
    cout << "If a cell is selected in spreadsheet, it prints selected cell information." << endl;
    cout << "To edit a cell in spreadsheet, enter the right side(RHS) of expression(formula) directly into cell." << endl;
    cout << "Eg: A1 = sum(B1:C1) + 10; select cell A1 and type sum(B1:C1) + 10" << endl;
    cout << "For entering string value in spreadsheet cell, enter string in inverted comma." << endl;
    cout << "Eg: A1 = \"test\"; select A1 and type \"test\" in cell." << endl;
    cout << "To list menu of commands for spreadsheet, select \"help\" from chooser and execute." << endl;
    cout << endl;
    while (true) {
        GEvent event = waitForEvent(ACTION_EVENT | TABLE_EVENT | WINDOW_EVENT);
        if (event.getEventClass() == TABLE_EVENT) {
            GTableEvent tableEvent(event);
            int row = tableEvent.getRow();
            int col = tableEvent.getColumn();
            char colString = col - 1 + 'A';
            string cellref = colString + integerToString(row);
            if (event.getEventType() == TABLE_SELECTED) {
                scanner.setInput(cellref);
                executeCommand("get", cmdTable, scanner, model);
            } else if (event.getEventType() == TABLE_UPDATED) {
                string cellValue = tableEvent.getValue();
                string command = cellref + " = " + cellValue;
                scanner.setInput(command);
                executeCommand("set", cmdTable, scanner, model);
            }
        } else if (event.getEventClass() == ACTION_EVENT) {
            GActionEvent actionEvent(event);
            processActionEvent(actionEvent, cmdTable, view, model, scanner);
        } else if (event.getEventClass() == WINDOW_EVENT) {
            if (event.getEventType() == WINDOW_CLOSED) {
                string cmdName = "quit";
                executeCommand(cmdName, cmdTable, scanner, model);
            }
        }
	}
}

int main() {
    Map<string, cmdFnT> cmdTable;
    setUpCommandTable(cmdTable);
	interpretCommands(cmdTable);
	return 0;
}






