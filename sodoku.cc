#include <algorithm>
#include <cassert>
#include <cstdio>
#include <functional>
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

namespace Data {

  /*
  constexpr int leetcode_board[] = {
    5, 3, _, _, 7, _, _, _, _,
    6, _, _, 1, 9, 5, _, _, _,
    _, 9, 8, _, _, _, _, 6, _,
    8, _, _, _, 6, _, _, _, 3,
    4, _, _, 8, _, 3, _, _, 1,
    7, _, _, _, 2, _, _, _, 6,
    _, 6, _, _, _, _, 2, 8, _,
    _, _, _, 4, 1, 9, _, _, 5,
    _, _, _, _, 8, _, _, 7, 9
  };
  */

  constexpr char leetcode1[] =
    "53..7...."
    "6..195..."
    ".98....6."
    "8...6...3"
    "4..8.3..1"
    "7...2...6"
    ".6....28."
    "...419..5"
    "....8..79";

  constexpr char leetcode2[] =
    "..9748..."
    "7........"
    ".2.1.9..."
    "..7...24."
    ".64.1.59."
    ".98...3.."
    "...8.3.2."
    "........6"
    "...2759..";

  /*
----------------------------------------------------------------------------------------------------------------------------
|       5            3            4      |       6            7            8      |       9            1            2      |
|       6            7            2      |       1            9            5      |       3            4            8      |
|       1            9            8      |       3            4            2      |       5            6            7      |
----------------------------------------------------------------------------------------------------------------------------
|       8            5            9      |       7            6            1      |       4            2            3      |
|       4            2            6      |       8            5            3      |       7            9            1      |
|       7            1            3      |       9            2            4      |       8            5            6      |
----------------------------------------------------------------------------------------------------------------------------
|       9            6            1      |       5            3            7      |       2            8            4      |
|       2            8            7      |       4            1            9      |       6            3            5      |
|       3            4            5      |       2            8            6      |       1            7            9      |
----------------------------------------------------------------------------------------------------------------------------
  */


  constexpr char puzzle8_1[] =
    ".5.....86"
    "....3..7."
    ".84.293.."
    "2..85...."
    "...6.6..."
    "....92..5"
    "..391.65."
    ".9..4...."
    "41.....9.";

  // Output:
  // [5","3","4","6","7","8","9","1","2"],
  // ["6","7","2","1","9","5","3","4","8"],
  // ["1","9","8","3","4","2","5","6","7"],
  // ["8","5","9","7","6","1","4","2","3"],
  // ["4","2","6","8","5","3","7","9","1"],
  // ["7","1","3","9","2","4","8","5","6"],
  // ["9","6","1","5","3","7","2","8","4"],
  // ["2","8","7","4","1","9","6","3","5"],
  // ["3","4","5","2","8","6","1","7","9"]
};

enum class Status {
  Simplified,
  Assigned,
  NoProgress
};

template<class UIntType = unsigned int>
class SmallIntSet {
public:
  void insert(int value) {
    set_ |= shift(value);
  }

  void insert(SmallIntSet that) {
    set_ |= that.set_;
  }

  int erase(int value) {
    UIntType shifted = shift(value);
    int ret = (shifted & set_) != 0;
    set_ &= ~shifted;
    return ret;
  }

  bool contains(int value) const {
    return (set_ & shift(value)) != 0;
  }

  bool contains(SmallIntSet other) const {
    return (set_ & other.set_) == other.set_;
  }

  void erase(SmallIntSet other) {
    set_ &= ~other.set_;
  }

  SmallIntSet intersect(SmallIntSet other) {
    SmallIntSet out;
    out.set_ = other.set_ & set_;
    return out;
  }

  bool empty() const { return set_ == 0; }
  size_t size() const { return std::popcount(set_); }
  void clear() { set_ = 0; }

  void foreach(std::function<bool(int)> fn) const {
    for (int index = 0, sz = size(), found = 0; found < sz; ++index) {
      if (contains(index)) {
        ++found;
        if (!fn(index)) {
          return;
        }
      }
    }
  }

  int front() {
    const int sz = size();
    int index = 0;
    assert(sz > 0);
    while (!contains(index)) {
      ++index;
    }
    return index;
  }

  void print() const { foreach([](int index) -> bool { fprintf(stdout, "%d", index); return true; }); }

private:
  static UIntType shift(int value) {
    return 1 << value;
  }

  UIntType set_{0};
};

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

  // Run a heuristics and algorithms on this cell, returning:
  //  Assigned if this cell has now been assigned, and thus may be removed from unassgned set
  //  Simplified if the cells set of possibilties has been reduced, but it is not yet assigned
  //  NoProgress
  Status reduce();

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
    possible_values_.foreach([] (int possible_value) -> bool {
      fprintf(stdout, "%d", possible_value);
      return true;
    });
    fprintf(stdout, ")");
    for (int i = 0; i < suffix_size; ++i) {
      fprintf(stdout, " ");
    }
  }

  void printTerse() const {
    fprintf(stdout, "[%d,%d]:", row_, col_);
    if (value_ == kUnset) {
      fprintf(stdout, "(");
      possible_values_.print();
      fprintf(stdout, ") ");
    } else {
      fprintf(stdout, "%d ", value_);
    }
  }

  void removeAvailable(int value) {
    if (possible_values_.contains(value)) {
      printTerse();
      fprintf(stdout, " removing %d --> ", value);
      possible_values_.erase(value);
      printTerse();
      fprintf(stdout, "\n");
    }
  }

private:
  int value_{kUnset};
  int row_{-1};
  int col_{-1};
  SmallIntSet<> possible_values_;
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
    for (Cell* cell : cells_) {
      cell->removeAvailable(value);
    }
    return available_.erase(value) == 1;
  }

  const char* name() const { return name_.c_str(); }
  void setName(const char* name) { name_ = name; }

  //SmallIntSet<> values_;
  SmallIntSet<> available_;
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
  bool makeProgress();
  void computeAvailable();

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
  std::set<Cell*> unassigned_cells_;
};

void Cell::computePossibleValues() {
  if (value_ != kUnset) {
    return;
  }
  assert(containers_.size() == 3);
  possible_values_ = containers_[0]->available_.intersect(containers_[1]->available_).
    intersect(containers_[2]->available_);
}

bool Cell::setValue(int value) {
  fprintf(stdout, "Assign: ");
  value_ = value;
  printTerse();
  fprintf(stdout, "\n");
  bool ret = true;
  if (value != kUnset) {
    for (Container* container : containers_) {
      if (!container->claimValue(value)) {
        fprintf(stderr, "Cell [%d,%d] setting value %d not valid in container %s\n",
                row_, col_, value, container->name());
        ret = false;
      }
    }
    possible_values_.clear();
  }
  // Consider also checking value is in possible_values_.
  return ret;
}

Status Cell::reduce() {
  if (possible_values_.size() == 1) {
    assert(setValue(possible_values_.front()));
    assert(possible_values_.empty());
    return Status::Assigned;
  }

  // Sodoku cross-hatching technique, where all of the other cells in any container
  // exclude a value this has.
  Status ret = Status::NoProgress;
  possible_values_.foreach([this, &ret](int value) -> bool {
    bool no_other_cells_in_containers_allow_value = true;
    for (Container* container : containers_) {
      for (Cell* other_cell : container->cells_) {
        if (other_cell != this &&
            (other_cell->value_ == value ||
             other_cell->possible_values_.contains(value))) {
          no_other_cells_in_containers_allow_value = false;
          return true;
        }
      }
    }
    if (no_other_cells_in_containers_allow_value) {
      possible_values_.clear();
      possible_values_.insert(value);
      ret = Status::Simplified;
      return false;           // Stop the iteration
    }
    return true;                // Continue the iteration
  });

  if (ret == Status::Assigned) {
    return ret;
  }
  assert(value_ == kUnset);

  // Check this cell's possible list against others in the same row/column/box for this case:
  //     This cell:   ABCDE
  //     Other cell1: AB
  //     Other cell2: AB
  //     --> change this cell's possible list to "CDE".
  // A similare case is:
  //     This cell: ABCDEF
  //     Other cell1: ABC
  //     Other cell2: ABC
  //     Other cell3: ABC
  //     --> change this cell's possible list to "DEF".
  // A slightly more complex one is:
  //     This cell: ABCDEF
  //     Other cell1: AB
  //     Other cell2: BC
  //     Other cell3: AC
  //     --> change this cell's possible list to "DEF".

  // Generalizing, the way to do this is to find N cells where the
  // union of their possible-sets is a subset of size N of this cell's
  // possible-set. To do this we first look for cells that with
  // possible-set that are subsets of this one.
  std::vector<Cell*> subsets;
  SmallIntSet<> union_of_possibilities;
  if (possible_values_.size() == 5) {
    fprintf(stdout, "stop here\n");
  }
  for (Container* container : containers_) {
    for (Cell* cell : container->cells_) {
      if (cell != this && cell->value_ == kUnset && possible_values_.contains(cell->possible_values_) &&
          possible_values_.size() > cell->possible_values_.size()) {
        subsets.push_back(cell);
        union_of_possibilities.insert(cell->possible_values_);
      }
    }
  }
  if (!subsets.empty() && subsets.size() == union_of_possibilities.size() &&
      union_of_possibilities.size() < possible_values_.size()) {
    fprintf(stdout, "Pruning %ld subsets", subsets.size());
    printTerse();
    fprintf(stdout, "of ");
    union_of_possibilities.print();
    fprintf(stdout, " from ");
    for (Cell* cell : subsets) {
      cell->printTerse();
    }
    possible_values_.erase(union_of_possibilities);
    fprintf(stdout, " --> ");
    possible_values_.print();
    fprintf(stdout, "\n");
    return Status::Simplified;
  }
  return Status::NoProgress;
}

void Board::computeAvailable() {
  for (int i = 0; i < kNumCells; ++i) {
    Cell& cell = cells_[i];
    if (cell.value() == kUnset) {
      cell.computePossibleValues();
      unassigned_cells_.insert(&cell);
    }
  }
}

#if 0
void Board::sortCellsByAvailableCount() {
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
  fprintf(stdout, "[%d,%d]", first->row(), first->col());
  first->printPossibleValues();
  fprintf(stdout, "\n");
}
#endif

bool Board::makeProgress() {
  bool progress = false;
  for (auto iter = unassigned_cells_.begin(); iter != unassigned_cells_.end(); ) {
    Status status = (*iter)->reduce();
    if (status == Status::Assigned) {
      progress = true;
      unassigned_cells_.erase(iter++);
    } else {
      progress |= status == Status::Simplified;
      ++iter;
    }
  }
  return progress;
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

static void run(const char* label, const char* data) {
  fprintf(stderr, "\n\nBoard %s\n", label);
  Board board;
  if (board.populate(data)) {
    board.computeAvailable();
    board.print();
    while (board.makeProgress()) {
      board.print();
    }
  }
}

int main() {
  run("Leetcode1", Data::leetcode1);
  run("Leetcode2", Data::leetcode2);
  //run("puzzle8_1", Data::puzzle8_1);
  return 0;
}

