// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "TupleFileReader.h"

#include <boost/lexical_cast.hpp>

#include "../../Parameters.h"
#include "../utilities/UsageException.h"
#include "../utilities/NoInputException.h"
#include "../utilities/DataFormatException.h"

#ifdef VERBOSE_PARSER
#include <iostream>
#endif

TupleFileReader::TupleFileReader(const char* tensorFileNameParam, const char* inputDimensionSeparatorParam, const char* inputElementSeparatorParam): tensorFileName(tensorFileNameParam), tensorFile(tensorFileName), inputDimensionSeparator(inputDimensionSeparatorParam), inputElementSeparator(inputElementSeparatorParam), lineNb(0), ids2Labels(), labels2Ids(), nSet(), tupleIts()
{
  if (!tensorFile)
    {
      throw NoInputException(tensorFileNameParam);
    }
  init();
}

void TupleFileReader::init()
{
  if (tensorFile.eof())
    {
      throw UsageException(("No tuple in " + tensorFileName + '!').c_str());
    }
  ++lineNb;
  string nSetString;
  getline(tensorFile, nSetString);
  tokenizer<char_separator<char>> dimensions(nSetString, inputDimensionSeparator);
  tokenizer<char_separator<char>>::const_iterator dimensionIt = dimensions.begin();
  if (dimensionIt == dimensions.end())
    {
      init();
      return;
    }
  ids2Labels.resize(std::distance(dimensionIt, dimensions.end()));
  if (ids2Labels.size() < 2)
    {
      throw DataFormatException(tensorFileName.c_str(), lineNb, (lexical_cast<string>(ids2Labels.size()) + " dimension, but at least 2 are required!").c_str());
    }
  labels2Ids.resize(ids2Labels.size());
  nSet.resize(ids2Labels.size());
  tupleIts.resize(ids2Labels.size());
  parseLine(dimensions.begin(), dimensions.end());
#ifdef VERBOSE_PARSER
  cout << tensorFileName << ':' << lineNb << ": " << nSetString << '\n';
#endif
}

vector<FuzzyTuple> TupleFileReader::read()
{
  const vector<vector<unsigned int>::const_iterator>::iterator tupleItsEnd = tupleIts.end();
  for (vector<FuzzyTuple> fuzzyTuples; ; )
    {
      fuzzyTuples.emplace_back(tupleIts, 1.);
      // Advance tuple in nSet, little-endian-like
      vector<vector<unsigned int>::const_iterator>::iterator tupleItsIt = tupleIts.begin();
      for (vector<vector<unsigned int>>::const_iterator nSetIt = nSet.begin(); nSetIt != nSet.end() && ++*tupleItsIt == nSetIt->end(); ++nSetIt)
	{
	  *tupleItsIt++ = nSetIt->begin();
	}
      if (tupleItsIt == tupleItsEnd)
	{
	  // All tuples in nSet enumerated: find and parse the next line (if any)
	  for (; ; )
	    {
	      if (tensorFile.eof())
		{
		  tensorFile.close();
		  fuzzyTuples.shrink_to_fit();
		  stable_sort(fuzzyTuples.begin(), fuzzyTuples.end());
		  fuzzyTuples.erase(unique(fuzzyTuples.begin(), fuzzyTuples.end()), fuzzyTuples.end());
		  return fuzzyTuples;
		}
	      ++lineNb;
	      string nSetString;
	      getline(tensorFile, nSetString);
	      tokenizer<char_separator<char>> dimensions(nSetString, inputDimensionSeparator);
	      if (dimensions.begin() != dimensions.end())
		{
		  parseLine(dimensions.begin(), dimensions.end());
#ifdef VERBOSE_PARSER
		  cout << tensorFileName << ':' << lineNb << ": " << nSetString << '\n';
#endif
		  break;
		}
	    }
	}
    }
}

vector<vector<string>>& TupleFileReader::getIds2Labels()
{
  return ids2Labels;
}

void TupleFileReader::parseLine(const tokenizer<char_separator<char>>::const_iterator dimensionBegin, const tokenizer<char_separator<char>>::const_iterator dimensionEnd)
{
  vector<vector<string>>::iterator ids2LabelsIt = ids2Labels.begin();
  tokenizer<char_separator<char>>::const_iterator dimensionIt = dimensionBegin;
  vector<vector<unsigned int>>::iterator nSetIt = nSet.begin();
  for (unordered_map<string, unsigned int>& labels2IdsInDimension : labels2Ids)
    {
      if (dimensionIt == dimensionEnd)
	{
	  throw DataFormatException(tensorFileName.c_str(), lineNb, ("fewer than the expected " + lexical_cast<string>(nSet.size()) + " dimensions!").c_str());
	}
      nSetIt->clear();
      tokenizer<char_separator<char>> elements(*dimensionIt, inputElementSeparator);
      const tokenizer<char_separator<char>>::const_iterator elementEnd = elements.end();
      tokenizer<char_separator<char>>::const_iterator elementIt = elements.begin();
      if (elementIt == elementEnd)
	{
	  throw DataFormatException(tensorFileName.c_str(), lineNb, ("no element in dimension " + lexical_cast<string>(nSetIt - nSet.begin()) + '!').c_str());
	}
      do
	{
	  const pair<unordered_map<string, unsigned int>::const_iterator, bool> label2Id = labels2IdsInDimension.insert({*elementIt, ids2LabelsIt->size()});
	  if (label2Id.second)
	    {
	      ids2LabelsIt->push_back(*elementIt);
	    }
	  nSetIt->push_back(label2Id.first->second);
	}
      while (++elementIt != elementEnd);
      ++ids2LabelsIt;
      ++nSetIt;
      ++dimensionIt;
    }
  if (dimensionIt != dimensionEnd)
    {
      throw DataFormatException(tensorFileName.c_str(), lineNb, ("more than the expected " + lexical_cast<string>(nSet.size()) + " dimensions!").c_str());
    }
  vector<vector<unsigned int>::const_iterator>::iterator tupleItsIt = tupleIts.begin();
  for (const vector<unsigned int>& dimension : nSet)
    {
      *tupleItsIt++ = dimension.begin();
    }
}
