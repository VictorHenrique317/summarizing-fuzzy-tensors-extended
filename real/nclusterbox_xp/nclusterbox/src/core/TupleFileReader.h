// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef TUPLE_FILE_READER_H_
#define TUPLE_FILE_READER_H_

#include <unordered_map>
#include <fstream>
#include <boost/tokenizer.hpp>

#include "FuzzyTuple.h"

using namespace boost;

class TupleFileReader
{
 public:
  TupleFileReader(const char* tensorFileName, const char* inputDimensionSeparator, const char* inputElementSeparator);

  vector<FuzzyTuple> read(); /* returns the unique fuzzy tuples, ordered by FuzzyTuple::operator< */

  vector<vector<string>>& getIds2Labels();

 private:
  const string tensorFileName;
  ifstream tensorFile;
  const char_separator<char> inputDimensionSeparator;
  const char_separator<char> inputElementSeparator;
  unsigned int lineNb;
  vector<vector<string>> ids2Labels;
  vector<unordered_map<string, unsigned int>> labels2Ids;
  vector<vector<unsigned int>> nSet;
  vector<vector<unsigned int>::const_iterator> tupleIts;

  void init();			/* membership remains 0 if and only if the tensor is empty */
  void parseLine(const tokenizer<char_separator<char>>::const_iterator dimensionBegin, const tokenizer<char_separator<char>>::const_iterator dimensionEnd);
};

#endif /*TUPLE_FILE_READER_H_*/
