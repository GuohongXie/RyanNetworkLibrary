#ifndef RYANLIB_EXAMPLES_SUDOKU_SUDOKU_H
#define RYANLIB_EXAMPLES_SUDOKU_SUDOKU_H

#include <string>
#include <string_view>

std::string solveSudoku(const std::string_view& puzzle);
const int kCells = 81;
extern const char kNoSolution[];

#endif  // RYANLIB_EXAMPLES_SUDOKU_SUDOKU_H
