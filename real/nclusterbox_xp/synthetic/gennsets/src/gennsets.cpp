// Copyright 2009,2010,2011,2023 Loïc Cerf (magicbanana@gmail.com)

// This file is part of gennsets.

// gennsets is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// gennsets is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with gennsets; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <random>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include "sysexits.h"

using namespace std;
using namespace boost;

template<typename T> vector<T> getVectorFromString(const string& str)
{
  vector<T> tokens;
  T token;
  stringstream ss(str);
  while (ss >> token)
    {
      tokens.push_back(token);
    }
  return tokens;
}

ofstream outputFile;

void printSample(vector<unsigned int>& ids, const unsigned int size, mt19937& g)
{
  if (size > ids.size())
    {
      cerr << "Size " << size << " is greater than that of the tensor: dimension not printed!\n";
      return;
    }
  vector<unsigned int>::iterator it = ids.begin();
  shuffle(it, ids.end(), g);
  const vector<unsigned int>::iterator end = it + size;
  sort(it, end);
  outputFile << *it;
  while (++it != end)
    {
      outputFile << ',' << *it;
    }
}

int main(int argc, char* argv[])
{
  if (argc < 1 || argc > 4)
    {
      cerr << "Usage: gennsets dataset-sizes [n-set-size-file] [output-file]\n";
      return EX_USAGE;
    }
  ifstream nSetSizeFile;
  if (argc > 2 && string(argv[2]) != "-")
    {
      nSetSizeFile.open(argv[2]);
    }
  else
    {
      nSetSizeFile.open("/dev/stdin");
    }
  if (!nSetSizeFile)
    {
      cerr << "Input file cannot be read!\n";
      return EX_IOERR;
    }
  if (argc == 4 && string(argv[3]) != "-")
    {
      outputFile.open(argv[3]);
    }
  else
    {
      outputFile.open("/dev/stdout");
    }
  if (!outputFile)
    {
      cerr << "Output file cannot be written!\n";
      return EX_IOERR;
    }
  vector<vector<unsigned int>> ids;
  unsigned int n;
  {
    const vector<unsigned int> sizes(getVectorFromString<unsigned int>(argv[1]));
    const vector<unsigned int>::const_iterator sizeEnd = sizes.end();
    vector<unsigned int>::const_iterator sizeIt = sizes.begin();
    n = sizeEnd - sizeIt;
    if (!n)
      {
	cerr << "No size given!\n";
	return EX_USAGE;
      }
    ids.reserve(n);
    for (; sizeIt != sizeEnd; ++sizeIt)
      {
	if (!*sizeIt)
	  {
	    cerr << "No size can be 0!\n";
	    return EX_USAGE;
	  }
	vector<unsigned int> idsInDimension;
	idsInDimension.reserve(*sizeIt);
	for (unsigned int id = 0; id != *sizeIt; ++id)
	  {
	    idsInDimension.push_back(id);
	  }
	ids.emplace_back(std::move(idsInDimension));
      }
  }
  random_device rd;
  mt19937 g(rd());
  const vector<vector<unsigned int>>::iterator idsEnd = ids.end();
  const vector<vector<unsigned int>>::iterator idsBegin = ids.begin();
  while (nSetSizeFile.good())
    {
      string sizeString;
      getline(nSetSizeFile, sizeString);
      tokenizer<char_separator<char>> nSetSizes(sizeString);
      tokenizer<char_separator<char>>::const_iterator nSetSizeIt = nSetSizes.begin();
      if (nSetSizeIt != nSetSizes.end())
	{
	  if (std::distance(nSetSizeIt, nSetSizes.end()) != n)
	    {
	      cerr << sizeString << " does not have " << lexical_cast<string>(n) << " dimensions!\n";
	      return EX_USAGE;
	    }
	  printSample(*idsBegin, lexical_cast<unsigned int>(*nSetSizeIt), g);
	  for (vector<vector<unsigned int>>::iterator idsIt = idsBegin; ++idsIt != idsEnd; )
	    {
	      outputFile << ' ';
	      printSample(*idsIt, lexical_cast<unsigned int>(*++nSetSizeIt), g);
	    }
	  outputFile << '\n';
	}
    }
  return EX_OK;
}
