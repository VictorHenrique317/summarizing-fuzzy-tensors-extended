// Copyright 2018-2022 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "SparseCrispTube.h"

#include <algorithm>

long long SparseCrispTube::defaultMembership;
unsigned int SparseCrispTube::sizeLimit;

SparseCrispTube::SparseCrispTube(): tube()
{
}

bool SparseCrispTube::isFullSparseTube() const
{
  return tube.size() == sizeLimit;
}

void SparseCrispTube::setTuple(const vector<unsigned int>::const_iterator idIt)
{
  tube.push_back(*idIt);
}

DenseCrispTube* SparseCrispTube::getDenseRepresentation() const
{
  return new DenseCrispTube(tube);
}

void SparseCrispTube::sortTubes()
{
  tube.shrink_to_fit();
  sort(tube.begin(), tube.end());
}

void SparseCrispTube::sumOnPattern(const vector<vector<unsigned int>>::const_iterator dimensionIt, int& nbOfPresentTuples) const
{
  const vector<unsigned int>::const_iterator tubeEnd = tube.end();
  vector<unsigned int>::const_iterator tubeBegin = tube.begin();
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      tubeBegin = lower_bound(tubeBegin, tubeEnd, *idIt);
      if (tubeBegin == tubeEnd)
	{
	  return;
	}
      if (*tubeBegin == *idIt)
	{
	  ++nbOfPresentTuples;
	}
    }
  while (++idIt != idEnd);
}

int SparseCrispTube::sumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator numbersOfPresentTuplesIt) const
{
  if (tube.empty())
    {
      return 0;
    }
  int sumOnPattern = 0;
  if (dimensionIt->back() < tube.back())
    {
      const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end();
      vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
      vector<unsigned int>::const_iterator idIt = tube.begin();
      do
	{
	  for (; *idIt < *presentElementIdIt; ++idIt)
	    {
	      ++(*numbersOfPresentTuplesIt)[*idIt];
	    }
	  if (*idIt == *presentElementIdIt)
	    {
	      ++idIt;
	      ++(*numbersOfPresentTuplesIt)[*presentElementIdIt];
	      ++sumOnPattern;
	    }
	}
      while (++presentElementIdIt != presentElementIdEnd);
      for (const vector<unsigned int>::const_iterator idEnd = tube.end(); idIt != idEnd; ++idIt)
	{
	  ++(*numbersOfPresentTuplesIt)[*idIt];
	}
      return sumOnPattern;
    }
  const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
  for (const unsigned int id : tube)
    {
      ++(*numbersOfPresentTuplesIt)[id];
      presentElementIdIt = lower_bound(presentElementIdIt, presentElementIdEnd, id);
      if (*presentElementIdIt == id)
	{
	  ++sumOnPattern;
	  ++presentElementIdIt;
	}
    }
  return sumOnPattern;
}

void SparseCrispTube::increaseSumsOnHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  // n == 2 and first dimension increased
  for (const unsigned int elementId : tube)
    {
      ++(*sumsIt)[elementId];
    }
}

int SparseCrispTube::increaseSumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  // necessarily the increased dimension
  if (binary_search(tube.begin(), tube.end(), dimensionIt->front()))
    {
      return 1;
    }
  return 0;
}

long long SparseCrispTube::getDefaultMembership()
{
  return defaultMembership;
}

void SparseCrispTube::setDefaultMembership(const int defaultMembershipParam)
{
  defaultMembership = defaultMembershipParam;
}

void SparseCrispTube::setDefaultMembershipAndSizeLimit(const int defaultMembershipParam, const unsigned int sizeLimitParam)
{
  defaultMembership = defaultMembershipParam;
  sizeLimit = sizeLimitParam;
}
