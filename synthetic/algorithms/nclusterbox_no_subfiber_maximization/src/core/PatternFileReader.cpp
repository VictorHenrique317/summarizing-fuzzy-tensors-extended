// Copyright 2018-2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "PatternFileReader.h"

#include <iostream>
#include <boost/lexical_cast.hpp>

#include "../utilities/NoInputException.h"
#include "../utilities/DataFormatException.h"
#include "ConcurrentPatternPool.h"
#include "AbstractRoughTensor.h"

string PatternFileReader::noisyNSetFileName;
ifstream PatternFileReader::noisyNSetFile;
vector<unordered_map<string, unsigned int>> PatternFileReader::labels2Ids;
char_separator<char> PatternFileReader::inputElementSeparator;
unsigned int PatternFileReader::lineNb;

unordered_map<string, unsigned int> labels2IdsInDimension(const vector<string>& labelsInDimension)
{
  vector<string>::const_iterator labelIt = labelsInDimension.begin();
  const vector<string>::const_iterator labelEnd = labelsInDimension.end();
  unordered_map<string, unsigned int> labels2Ids;
  labels2Ids.reserve(labelEnd - labelIt);
  unsigned int id = 0;
  do
    {
      labels2Ids[*labelIt] = id++;
    }
  while (++labelIt != labelEnd);
  return labels2Ids;
}

void PatternFileReader::openFile(const char* noisyNSetFileNameParam)
{
  noisyNSetFileName = noisyNSetFileNameParam;
  noisyNSetFile.open(noisyNSetFileNameParam);
  if (!noisyNSetFile)
    {
      throw NoInputException(noisyNSetFileNameParam);
    }
  ConcurrentPatternPool::setReadFromFile();
}

void PatternFileReader::read(const char* inputDimensionSeparatorParam, const char* inputElementSeparatorParam, const vector<vector<string>>& ids2Labels, const unsigned int maxNbOfInitialPatterns)
{
  inputElementSeparator = char_separator<char>(inputElementSeparatorParam);
  lineNb = 0;
  vector<vector<string>>::const_iterator labelsInDimensionIt = ids2Labels.begin();
  const vector<vector<string>>::const_iterator labelsInDimensionEnd = ids2Labels.end();
  labels2Ids.reserve(labelsInDimensionEnd - labelsInDimensionIt);
  labels2Ids.emplace_back(labels2IdsInDimension(*labelsInDimensionIt));
  ++labelsInDimensionIt;
  do
    {
      labels2Ids.emplace_back(labels2IdsInDimension(*labelsInDimensionIt));
    }
  while (++labelsInDimensionIt != labelsInDimensionEnd);
  unsigned int nbOfInitialPatterns = 0;
  const char_separator<char> inputDimensionSeparator(inputDimensionSeparatorParam);
  while (!noisyNSetFile.eof())
    {
      ++lineNb;
      string noisyNSetString;
      getline(noisyNSetFile, noisyNSetString);
      tokenizer<char_separator<char>> dimensions(noisyNSetString, inputDimensionSeparator);
      if (dimensions.begin() != dimensions.end())
	{
#ifdef VERBOSE_PARSER
	  cout << noisyNSetFileName << ':' << lineNb << ": " << noisyNSetString << '\n';
#endif
	  const tokenizer<char_separator<char>>::const_iterator dimensionEnd = dimensions.end();
	  tokenizer<char_separator<char>>::const_iterator dimensionIt = dimensions.begin();
	  vector<vector<unsigned int>> nSet(labels2Ids.size());
	  vector<unordered_map<string, unsigned int>>::const_iterator labels2IdsIt = labels2Ids.begin();
	  try
	    {
	      vector<unsigned int>::const_iterator internalDimensionIdIt = AbstractRoughTensor::getExternal2InternalDimensionOrder().begin();
	      nSet[*internalDimensionIdIt] = getDimension(dimensionIt, dimensionEnd, labels2IdsIt);
	      ++internalDimensionIdIt;
	      const vector<unsigned int>::const_iterator internalDimensionIdEnd = AbstractRoughTensor::getExternal2InternalDimensionOrder().end();
	      do
		{
		  nSet[*internalDimensionIdIt] = getDimension(++dimensionIt, dimensionEnd, ++labels2IdsIt);
		}
	      while (++internalDimensionIdIt != internalDimensionIdEnd);
	    }
	  catch (DataFormatException& e)
	    {
	      cerr << e.what() << " -> pattern ignored!\n";
	      continue;
	    }
	  ConcurrentPatternPool::addPattern(nSet);
	  if (++nbOfInitialPatterns == maxNbOfInitialPatterns)
	    {
	      break;
	    }
	}
    }
  ConcurrentPatternPool::allPatternsAdded();
  noisyNSetFile.close();
  noisyNSetFileName.clear();
  noisyNSetFileName.shrink_to_fit();
  labels2Ids.clear();
  labels2Ids.shrink_to_fit();
}

vector<unsigned int> PatternFileReader::getDimension(const tokenizer<char_separator<char>>::const_iterator dimensionIt, const tokenizer<char_separator<char>>::const_iterator dimensionEnd, const vector<unordered_map<string, unsigned int>>::const_iterator labels2IdsIt)
{
  if (dimensionIt == dimensionEnd)
    {
      throw DataFormatException(noisyNSetFileName.c_str(), lineNb, ("less than the expected " + lexical_cast<string>(labels2Ids.size()) + " dimensions").c_str());
    }
  vector<unsigned int> nSetDimension;
  const unordered_map<string, unsigned int>::const_iterator label2IdEnd = labels2IdsIt->end();
  tokenizer<char_separator<char>> elements(*dimensionIt, inputElementSeparator);
  for (const string& element : elements)
    {
      const unordered_map<string, unsigned int>::const_iterator label2IdIt = labels2IdsIt->find(element);
      if (label2IdIt == label2IdEnd)
	{
	  throw DataFormatException(noisyNSetFileName.c_str(), lineNb, (element + " is not in dimension " + lexical_cast<string>(labels2IdsIt - labels2Ids.begin()) + " of fuzzy tensor").c_str());
	}
      nSetDimension.push_back(label2IdIt->second);
    }
  if (nSetDimension.empty())
    {
      throw DataFormatException(noisyNSetFileName.c_str(), lineNb, ("no element in dimension " + lexical_cast<string>(labels2IdsIt - labels2Ids.begin())).c_str());
    }
  sort(nSetDimension.begin(), nSetDimension.end());
  if (adjacent_find(nSetDimension.begin(), nSetDimension.end()) != nSetDimension.end())
    {
      throw DataFormatException(noisyNSetFileName.c_str(), lineNb, ("repeated elements in dimension " + lexical_cast<string>(labels2IdsIt - labels2Ids.begin())).c_str());
    }
  return nSetDimension;
}
