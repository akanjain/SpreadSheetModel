/**
 * File: ssview.h
 * --------------
 * This file defines the interface for SSView class used in the
 * Stanford 1-2-3 spreadsheet program.
 * This file is changed for graphical interface extension for spreadsheet model.
 */

#ifndef _ssview_
#define _ssview_

#include <string>
#include "gtable.h"
#include "gwindow.h"
#include "ginteractors.h"

/**
 * Class constants: kNumRowsDisplayed, kNumColsDisplayed
 * Usage: int count = kNumRowsDisplayed;
 * --------------------------------------------
 * Constants that control the number of visible rows/cols
 * in the graphics window.
 */

static const int kNumRowsDisplayed = 35;
static const int kNumColsDisplayed = 20;

/**
 * Class: SSView
 * --------------
 * This class handles the display of a spreadsheet in the graphics
 * window. It exports two public member functions, one to display the
 * spreadsheet grid/labels/background, and another to display the
 * contents for a given cell. The member functions are intended to
 * be invoked by the model when cells are updated.
 */

class SSView : private GWindow {
public:

/**
 * Constructor: SSView
 * Usage: SSView view;
 * -------------------
 * The constructor initializes the graphics window, configures
 * the view object, and displays an empty spreadsheet.
 */

    SSView();

/**
 * Member function: getCommandName
 * Usage: view.getCommandName();
 * -----------------------------
 * Method returns command selected through cmdChooser on spreadsheet
 */

    std::string getCommandName();

/**
 * Member function: getCellName
 * Usage: view.getCellName();
 * -----------------------------
 * Method returns cell name e.g. "A1" selected through cellRowChooser and cellColChooser on spreadsheet
 */

    std::string getCellName();

/**
 * Member function: getSheetName
 * Usage: view.getSheetName();
 * ---------------------------
 * Method returns spreadsheet selected through sheetChooser on spreadsheet
 */

    std::string getSheetName();

/**
 * Member function: addSheetToChooser
 * Usage: view.addSheetToChooser(fileNameSaved);
 * ---------------------------
 * Adds the saved spreadsheet to chooser through controller
 */

    void addSheetToChooser(std::string fileName);

/**
 * Member function: displayEmptySpreadsheet
 * Usage: view.displayEmptySpreadsheet();
 * -------------------------------------
 * This member function draws an empty spreadsheet. It will erase any
 * previous contents by filling the background. It then redraws the
 * grid and row/col labels. All cells are drawn empty.
 */

    void displayEmptySpreadsheet();

/**
 * Member function: displayCell
 * Usage: view.displayCell(name, contents);
 * ----------------------------------------
 * This member function draws the contents for a given cell.
 * Cell name format is column letter followed by row number, e.g. "A7".
 * Columns are lettered starting from 'A'. Rows are numbered starting from 1.
 * Note that rows are not zero-based (typical users don't count from zero!)
 * An error is raised if cellname is invalid. The string is drawn
 * right-aligned within the cell. If the string is too long, it is
 * truncated to fit the cell.
 */

    void displayCell(const std::string& cellname, const std::string& txt);

private:
    GTable table;

/**
 * Button for executing commands from chooser on spreadsheet
 */
    GButton	mybutton;

/**
 * GChooser for command chooser from spreadsheet:
 * help - listing out commands available on spreadsheet
 * load - loads spreadsheet
 * save - saves spreadsheet
 * set - setting cell value
 * get - getting cell value
 * console - entering commands through console like in original version of assignment
 * command - entering single command through console
 * quit - quit the program and console
 * clear - clears the spreadsheet
 */
    GChooser cmdChooser;

/**
 * GChooser for selecting row of spreadsheet
 */
    GChooser cellRowChooser;

/**
 * GChooser for selecting column of spreadsheet
 */
    GChooser cellColChooser;

/**
 * GChooser for selecting spreadsheet to load
 * All available sheets in spreadsheet/ directory is added to chooser
 * Any new saved sheets are also added on the way to chooser by controller
 */
    GChooser sheetChooser;

    void labelAxes();

/**
 * Adds all commands to cmdChooser
 */
    void setUpCommandChooser();

/**
 * Adds all valid spreadsheet cells to cellRowChooser and cellColChooser
 */
    void setUpCellChooser();

/**
 * Adds all sheets in spreadsheet/ directory to sheetChooser
 */
    void setUpSheetChooser();
};

#endif
