


#include "CSV.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <sstream>
#include <vector>


std::vector<std::string> CSV::singleColumn(std::string column) {
  if (!hasColumn(column)) {
    std::cout << " Error : could not find column " << column
              << " to merge from file " << file_name_ << std::endl;
    exit(1);
  }

  auto const column_pos =
      std::find(std::begin(columns_), std::end(columns_), column) -
      std::begin(columns_);

  std::vector<std::string> values;
  std::transform(std::begin(rows_), std::end(rows_), std::back_inserter(values),
                 [column_pos](auto const &row) { return row[column_pos]; });

  return values;
}

std::string CSV::lookUp(std::string lookup_column, std::string value,
                        std::string return_column) const {

  if (!hasColumn(lookup_column)) {
    std::cout << " Error : could not find column " << lookup_column
              << " in file " << file_name_ << std::endl;
    exit(1);
  }

  if (!hasColumn(return_column)) {
    std::cout << " Error : could not find column " << return_column
              << " in file " << file_name_ << std::endl;
    exit(1);
  }

  // find position of lookup column
  auto const column_pos =
      std::find(std::begin(columns_), std::end(columns_), lookup_column) -
      std::begin(columns_);

  // find number of values in lookup column
  auto const value_count = std::count_if(
      std::begin(rows_), std::end(rows_),
      [value, column_pos](auto &row) { return row[column_pos] == value; });

  if (!value_count) {
    std::cout << "Error : could not find requested lookup value" << value
              << " from column " << lookup_column << " from file " << file_name_
              << std::endl;
    exit(1);
  }

  if (value_count > 1) {
    std::cout << "Error : multiple entries found for requested lookup value"
              << value << " from column " << lookup_column << " from file "
              << file_name_ << std::endl;
    exit(1);
  }

  // find row where lookup column has value
  auto const value_pos = std::find_if(std::begin(rows_), std::end(rows_),
                                      [value, column_pos](auto &row) {
                                        return row[column_pos] == value;
                                      }) -
                         std::begin(rows_);

  // find position of return column
  auto const return_pos =
      std::find(std::begin(columns_), std::end(columns_), return_column) -
      std::begin(columns_);

  return rows_[value_pos][return_pos];
}

CSV::CSV(std::string fn, char s, char se) : file_name_(fn), reader_(s, se) {

  std::ifstream file(file_name_);
  if (!file.is_open()) {
    std::cout << " Error: cannot open csv file "
              << file_name_ << std::endl;
    exit(1);
  }

  std::string raw_line;

  // read header line
  getline(file, raw_line);
  columns_ = reader_.parseLine(raw_line);

  // ensure column names are unique
  std::set<std::string> uniq_headers(std::begin(columns_), std::end(columns_));
  if (uniq_headers.size() != columns_.size()) {
    std::cout << "Error: CSV file " << file_name_
              << " does not have unique Header names" << std::endl;
    exit(1);
  }

  // read remaining rows
  while (getline(file, raw_line)) {
    auto data_line = reader_.parseLine(raw_line);
    // ensure all rows have correct number of columns
    if (columns_.size() != data_line.size()) {
      std::cout << " Error: incorrect number of columns in CSV file "
                << file_name_ << std::endl;
      exit(1);
    }
    rows_.push_back(data_line);
  }
}

void CSV::merge(CSV merge_csv, std::string column) {

  if (!hasColumn(column)) {
    std::cout << " Error : could not find column " << column << " in file "
              << file_name_ << std::endl;
    exit(1);
  }

  if (!merge_csv.hasColumn(column)) {
    std::cout << " Error : could not find column " << column
              << " to merge from file " << merge_csv.fileName() << std::endl;
    exit(1);
  }

  // ensure lookup column has distinct values
  auto lookop_column = singleColumn(column);
  std::set<std::string> lookup_values(std::begin(lookop_column),
                                      std::end(lookop_column));
  if (lookup_values.size() != rows_.size()) {
    std::cout << "Error: CSV file " << file_name_
              << " does not have unique values in column " << column
              << std::endl;
    exit(1);
  }

  // ensure column in second file has matching values for all values in this
  // file
  auto merge_column = merge_csv.singleColumn(column);
  std::set<std::string> merge_values(std::begin(merge_column),
                                     std::end(merge_column));
  if (!std::includes(std::begin(merge_values), std::end(merge_values),
                     std::begin(lookup_values), std::end(lookup_values))) {
    std::cout << "Error: CSV file " << merge_csv.fileName()
              << " does not have some matching values for column " << column
              << std::endl;
    exit(1);
  }

  // find position of lookup column
  auto const column_pos =
      std::find(std::begin(columns_), std::end(columns_), column) -
      std::begin(columns_);

  // merge columns from second file
  for (auto const &merge_column : merge_csv.columns()) {
    // only add additional columns
    if (std::find(std::begin(columns_), std::end(columns_), merge_column) ==
        std::end(columns_)) {
      columns_.push_back(merge_column);
      // for each column add value to every row
      for (auto &row : rows_)
        row.push_back(merge_csv.lookUp(column, row[column_pos], merge_column));
    }
  }
}


