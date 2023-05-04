// Copyright 2018-2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef PATTERN_FILE_READER_H_
#define PATTERN_FILE_READER_H_

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost;

class PatternFileReader
{
 public:
  static void openFile(const char* noisyNSetFileName);
  static void read(const char* inputDimensionSeparator, const char* inputElementSeparator, const vector<vector<string>>& ids2Labels, const unsigned int maxNbOfInitialPatterns);

 private:
  static string noisyNSetFileName;
  static ifstream noisyNSetFile;
  static vector<unordered_map<string, unsigned int>> labels2Ids;
  static char_separator<char> inputElementSeparator;
  static unsigned int lineNb;

  static vector<unsigned int> getDimension(const tokenizer<char_separator<char>>::const_iterator dimensionIt, const tokenizer<char_separator<char>>::const_iterator dimensionEnd, const vector<unordered_map<string, unsigned int>>::const_iterator labels2IdsIt);
};

#endif /*PATTERN_FILE_READER_H_*/
