// Copyright 2018-2022 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "DenseCrispTube.h"

unsigned int DenseCrispTube::size;

DenseCrispTube::DenseCrispTube(vector<double>::const_iterator& membershipIt): tube(size)
{
  unsigned int elementId = 0;
  do
    {
      if (*membershipIt > 0)
	{
	  tube.set(elementId);
	}
      ++membershipIt;
    }
  while (++elementId != size);
}

DenseCrispTube::DenseCrispTube(const vector<unsigned int>& sparseTube): tube(size)
{
  for (const unsigned int elementId : sparseTube)
    {
      tube.set(elementId);
    }
}

void DenseCrispTube::setTuple(const vector<unsigned int>::const_iterator idIt)
{
  tube.set(*idIt);
}

void DenseCrispTube::sumOnPattern(const vector<vector<unsigned int>>::const_iterator dimensionIt, int& nbOfPresentTuples) const
{
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      if (tube[*idIt])
	{
	  ++nbOfPresentTuples;
	}
    }
  while (++idIt != idEnd);
}

void DenseCrispTube::setSize(const unsigned int sizeParam)
{
  size = sizeParam;
}

int DenseCrispTube::sumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator numbersOfPresentTuplesIt) const
{
  int sumOnPattern = 0;
  dynamic_bitset<>::size_type elementId = tube.find_first();
  {
    // Hyperplanes until the last present one
    const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end();
    vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
    unsigned int presentElementId = *presentElementIdIt;
    for (; elementId != dynamic_bitset<>::npos; elementId = tube.find_next(elementId))
      {
	++((*numbersOfPresentTuplesIt)[elementId]);
	if (elementId >= presentElementId)
	  {
	    if (elementId == presentElementId)
	      {
		++sumOnPattern;
		if (++presentElementIdIt == presentElementIdEnd)
		  {
		    break;
		  }
	      }
	    else
	      {
		presentElementIdIt = lower_bound(++presentElementIdIt, presentElementIdEnd, elementId);
		if (presentElementIdIt == presentElementIdEnd)
		  {
		    break;
		  }
		if (elementId == *presentElementIdIt)
		  {
		    ++sumOnPattern;
		    if (++presentElementIdIt == presentElementIdEnd)
		      {
			break;
		      }
		  }
	      }
	    presentElementId = *presentElementIdIt;
	  }
      }
  }
  // Hyperplanes after the last present one
  for (elementId = tube.find_next(elementId); elementId != dynamic_bitset<>::npos; elementId = tube.find_next(elementId))
    {
      ++((*numbersOfPresentTuplesIt)[elementId]);
    }
  return sumOnPattern;
}

void DenseCrispTube::increaseSumsOnHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  // n == 2 and first dimension increased
  for (dynamic_bitset<>::size_type elementId = tube.find_first(); elementId != dynamic_bitset<>::npos; elementId = tube.find_next(elementId))
    {
      ++((*sumsIt)[elementId]);
    }
}

int DenseCrispTube::increaseSumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  // necessarily the increased dimension
  return tube[dimensionIt->front()];
}
