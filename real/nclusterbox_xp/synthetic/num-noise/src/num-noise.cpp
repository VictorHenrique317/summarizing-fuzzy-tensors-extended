// Copyright 2009,2010,2011,2023 Loïc Cerf (magicbanana@gmail.com)

// This file is part of num-noise.

// num-noise is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License version 3 as published by the Free Software Foundation

// num-noise is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with num-noise; if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

#include <vector>
#include <sstream>
#include <iostream>
#include <random>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/math/special_functions/beta.hpp>
#include "sysexits.h"

using namespace std;
using namespace boost;
using namespace boost::math;

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

int main(int argc, char* argv[])
{
  if (argc != 3 && argc != 4)
    {
      cerr << "Usage: num-noise dataset-sizes nb-of-correct-observations-per-tuple [nb-of-incorrect-observations-per-tuple]" << endl;
      return EX_USAGE;
    }
  const vector<unsigned int> sizes(getVectorFromString<unsigned int>(argv[1]));
  const vector<unsigned int>::const_iterator sizeBegin = sizes.begin();
  const vector<unsigned int>::const_iterator sizeEnd = sizes.end();
  const unsigned int n = sizeEnd - sizeBegin;
  if (!n)
    {
      cerr << "No size given!\n";
      return EX_USAGE;
    }
  vector<vector<vector<unsigned int>>> patterns;
  const char_separator<char> valueSeparator(",");
  const char_separator<char> dimensionSeparator(" ");
  while (cin.good())
    {
      string nSetString;
      getline(cin, nSetString);
      tokenizer<char_separator<char>> dimensions(nSetString, dimensionSeparator);
      const tokenizer<char_separator<char>>::const_iterator dimensionEnd = dimensions.end();
      tokenizer<char_separator<char>>::const_iterator dimensionIt = dimensions.begin();
      if (dimensionIt != dimensions.end())
	{
	  if (std::distance(dimensionIt, dimensions.end()) != n)
	    {
	      cerr << nSetString << " does not have " << lexical_cast<string>(n) << " dimensions!\n";
	      return EX_USAGE;
	    }
	  vector<vector<unsigned int>> nSet;
	  nSet.reserve(n);
	  do
	    {
	      vector<unsigned int> dimension;
	      tokenizer<char_separator<char>> values(*dimensionIt, valueSeparator);
	      const tokenizer<char_separator<char>>::const_iterator valueEnd = values.end();
	      for (tokenizer<char_separator<char>>::const_iterator valueIt = values.begin(); valueIt != valueEnd; ++valueIt)
		{
		  dimension.push_back(lexical_cast<unsigned int>(*valueIt));
		}
	      sort(dimension.begin(), dimension.end());
	      nSet.emplace_back(dimension);
	    }
	  while (++dimensionIt != dimensionEnd);
	  patterns.push_back(nSet);
	}
    }
  const double alpha = lexical_cast<double>(argv[2]) + 1;
  double beta;
  if (argc == 3)
    {
      beta = 1;
    }
  else
    {
      beta = lexical_cast<double>(argv[3]) + 1;
    }
  vector<unsigned int> tuple(n);
  const vector<unsigned int>::iterator idBegin = tuple.begin();
  const vector<unsigned int>::iterator idEnd = tuple.end();
  random_device rd;
  mt19937 g(rd());
  uniform_real_distribution<> dis(0., 1.);
  for (; ; )
    {
      for (vector<unsigned int>::const_iterator dimensionIt = tuple.begin(); dimensionIt != tuple.end(); ++dimensionIt)
	{
	  cout << *dimensionIt << ' ';
	}
      const vector<vector<vector<unsigned int>>>::const_iterator patternEnd = patterns.end();
      vector<vector<vector<unsigned int>>>::const_iterator patternIt = patterns.begin();
      for (; patternIt != patternEnd; ++patternIt)
	{
	  vector<unsigned int>::const_iterator idIt = idBegin;
	  vector<vector<unsigned int>>::const_iterator dimensionIt = patternIt->begin();
	  do
	    {
	      const vector<unsigned int>::const_iterator foundIt = lower_bound(dimensionIt->begin(), dimensionIt->end(), *idIt);
	      if (foundIt == dimensionIt->end() || *foundIt != *idIt)
		{
		  break;
		}
	      ++dimensionIt;
	    }
	  while (++idIt != idEnd);
	  if (idIt == idEnd)
	    {
	      break;
	    }
	}
      if (patternIt == patternEnd)
	{
	  cout << ibeta_inv(beta, alpha, dis(g)) << '\n';
	}
      else
	{
	  cout << ibeta_inv(alpha, beta, dis(g)) << '\n';
	}
      vector<unsigned int>::const_iterator sizeIt = sizeBegin;
      for (vector<unsigned int>::iterator idIt = idBegin; ; *idIt++ = 0)
	{
	  if (*idIt != *sizeIt - 1)
	    {
	      ++(*idIt);
	      break;
	    }
	  if (++sizeIt == sizeEnd)
	    {
	      return EX_OK;
	    }
	}
    }
}
