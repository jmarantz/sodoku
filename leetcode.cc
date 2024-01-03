#include <algorithm>
#include <cassert>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

constexpr int kUnset = 0;

constexpr int kBoxColumns = 3;
constexpr int kBoxRows = 3;
constexpr int kNumValues = kBoxColumns * kBoxRows;

constexpr int kNumCellsPerBox = kBoxRows * kBoxColumns;
//constexpr int kNumBoxesHorizontal = 3;
constexpr int kNumBoxesVertical = 3;
//constexpr int kNumCellsPerBoxRow = kNumCellsPerBox * kNumBoxesHorizontal;

constexpr int kNumRows = kNumBoxesVertical * kBoxRows;
constexpr int kNumCols = kNumBoxesVertical * kBoxRows;
constexpr int kNumCells = kNumRows * kNumCols;

constexpr int kFirstDigit = 1;
constexpr int kLastDigit = kBoxColumns * kBoxRows;
constexpr int kNumBoxes = kNumCells / kNumCellsPerBox;

constexpr int kPrintWidth = kNumValues + 4; // strlen(" (12345678) ")

class Container;

class Cell {
public:
  void addContainer(Container* container) { containers_.push_back(container); }
  void computePossibleValues();
  void setRowCol(int row,
                 int col) {
    row_ = row;
    col_ = col;
  }
  size_t numPossibleValues() const { return possible_values_.size(); }
  int row() const { return row_; }
  int col() const { return col_; }
  bool setValue(int value);
  int value() const { return value_; }
  void print() const {
    if (value_ == kUnset) {
      printPossibleValues();
    } else {
      fprintf(stdout, "      %d      ", value_);
    }
  }
  bool assignIfDetermined() {
    if (possible_values_.size() == 1) {
      assert(setValue(possible_values_[0]));
      possible_values_.clear();
      return true;
    }
    return false;
  }

  void printPossibleValues() const {
    //fprintf(stdout, "[%d,%d]=%d: ", row_, col_, value_);
    if (possible_values_.size() == kNumValues) {
      fprintf(stdout, "     (*)     "); 
    }
    const int chars = possible_values_.size() + 2;
    const int prefix_size = (kPrintWidth - chars) / 2;
    const int suffix_size = kPrintWidth - chars - prefix_size;
    assert(prefix_size + suffix_size + chars == kPrintWidth);
    for (int i = 0; i < prefix_size; ++i) {
      fprintf(stdout, " ");
    }
    fprintf(stdout, "(");
    for (int possible_value : possible_values_) {
      fprintf(stdout, "%d", possible_value);
    }
    fprintf(stdout, ")");
    for (int i = 0; i < suffix_size; ++i) {
      fprintf(stdout, " ");
    }
  }

private:
  int value_{kUnset};
  int row_{-1};
  int col_{-1};
  std::vector<int> possible_values_;
  std::vector<Container*> containers_;
};

class Container {
public:
  Container() {
    for (int i = kFirstDigit; i <= kLastDigit; ++i) {
      available_.insert(i);
    }
  }

  void addCell(Cell& cell) {
    cells_.insert(&cell);
    cell.addContainer(this);
  }

  bool claimValue(int value) {
    return available_.erase(value) == 1;
  }

  const char* name() const { return name_.c_str(); }
  void setName(const char* name) { name_ = name; }

  //std::set<int> values_;
  std::set<int> available_;
  std::set<Cell*> cells_;
  std::string name_;
};

class Board {
public:
  Board() {
    // Name the containers for better error messages.
    populateContainerNames("Row", kNumRows, rows_);
    populateContainerNames("Column", kNumCols, columns_);
    populateContainerNames("Box", kNumBoxes, boxes_);

    int cell_index = 0;

    // Assign the rows and coluns.
    for (int r = 0; r < kNumRows; ++r) {
      Container& row = rows_[r];
      const int box_row = r / kBoxRows;

      for (int c = 0; c < kNumCols; ++c) {
        Container& col = columns_[c];
        const int box_col = c / kBoxColumns;
        const int box_index = box_row * kBoxColumns + box_col;
        Container& box = boxes_[box_index];

        Cell& cell = cells_[cell_index++];
        cell.setRowCol(r, c);
        row.addCell(cell);
        col.addCell(cell);
        box.addCell(cell);
      }
    }
  }

  bool populate(const char* values);
  void sortCellsByAvailableCount();
  void print();
  bool assignDeterminedCells();
  char value(int row, int col) {
    Cell& cell = cells_[row * kNumCols + col];
    if (cell.value() == kUnset) {
      return '.';
    }
    return cell.value() + '0';
  }
  
private:
  void populateContainerNames(const char* label, int num, Container* containers) {
    char buf[100];
    for (int i = 0; i < num; ++i) {
      snprintf(buf, sizeof(buf), "%s %d", label, i);
      containers[i].setName(buf);
    }
  }

  Cell cells_[kNumCells];
  Container rows_[kNumRows];
  Container columns_[kNumCols];
  Container boxes_[kNumBoxes];
  std::vector<Cell*> unassigned_cells_;
};

void Cell::computePossibleValues() {
  if (value_ != kUnset) {
    return;
  }
  assert(containers_.size() == 3);
  std::vector<int> temp;
  std::set_intersection(containers_[0]->available_.begin(), containers_[0]->available_.end(),
                        containers_[1]->available_.begin(), containers_[1]->available_.end(),
                        std::back_inserter(temp));
  possible_values_.clear();
  std::set_intersection(containers_[2]->available_.begin(), containers_[2]->available_.end(),
                        temp.begin(), temp.end(), std::back_inserter(possible_values_));
}

bool Cell::setValue(int value) {
  value_ = value;
  bool ret = true;
  if (value != kUnset) {
    for (Container* container : containers_) {
      if (!container->claimValue(value)) {
        fprintf(stderr, "Cell [%d,%d] setting value %d not valid in container %s\n",
                row_, col_, value, container->name());
        ret = false;
      }
    }
  }
  // Consider also checking value is in possible_values_.
  return ret;
}

void Board::sortCellsByAvailableCount() {
  unassigned_cells_.clear();
  for (int i = 0; i < kNumCells; ++i) {
    Cell& cell = cells_[i];
    if (cell.value() == kUnset) {
      cell.computePossibleValues();
      unassigned_cells_.push_back(&cell);
    }
  }
  if (unassigned_cells_.empty()) {
    return;
  }
  std::sort(unassigned_cells_.begin(), unassigned_cells_.end(), [](const Cell* a, const Cell* b) -> bool {
    return a->numPossibleValues() < b->numPossibleValues();
  });
  Cell* first = unassigned_cells_[0];
  if (first->numPossibleValues() == 0) {
    fprintf(stderr, "Cannot solve board: cell in row=%d, col=%d has no possible values.\n",
            first->row(), first->col());
  }
  //fprintf(stdout, "[%d,%d]", first->row(), first->col());
  //first->printPossibleValues();
  //fprintf(stdout, "\n");
}

bool Board::assignDeterminedCells() {
  bool assigned = false;
  for (Cell* cell : unassigned_cells_) {
    assigned |= cell->assignIfDetermined();
  }
  return assigned;
}

void Board::print() {
  int cell_index = 0;
  const char* row_divider = "----------------------------------------------------------------------------------------------------------------------------\n";
  for (int r = 0; r < kNumRows; ++r) {
    if ((r % kBoxRows) == 0) {
      fprintf(stdout, "%s", row_divider);
    }

    // Print the values or possible values
    for (int c = 0; c < kNumCols; ++c) {
      if ((c % kBoxColumns) == 0) {
        fprintf(stdout, "| ");
      }
      Cell& cell = cells_[cell_index++];
      cell.print();
    }
    fprintf(stdout, "|\n");
  }
  fprintf(stdout, "%s", row_divider);
}

bool Board::populate(const char* values) {
  bool ret = true;
  for (int i = 0; i < kNumCells; ++i, ++values) {
    if (*values != '.') {
      ret &= cells_[i].setValue(*values - '0');
    }
  }
  return ret;
}

class Solution {
public:
  void solveSudoku(std::vector<std::vector<char>>& array) {
    char data[81];
    int index = 0;
    for (std::vector<char>& row: array) {
      for (char c : row) {
        data[index++] = c;
      }
    }
    Board board;
    if (board.populate(data)) {
      while (true) {
        board.sortCellsByAvailableCount();
        if (!board.assignDeterminedCells()) {
          break;
        }
        //board.print();
      }
    }
    for (int r = 0; r < kNumRows; ++r) {
      std::vector<char>& row = array[r];
      for (int c = 0; c < kNumCols; ++c) {
        row[c] = board.value(r, c);
      }
    }
  }
};
