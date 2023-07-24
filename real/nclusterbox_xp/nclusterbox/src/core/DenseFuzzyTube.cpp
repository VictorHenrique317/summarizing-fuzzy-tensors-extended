// Copyright 2018-2022 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "DenseFuzzyTube.h"

unsigned int DenseFuzzyTube::size;

DenseFuzzyTube::DenseFuzzyTube(vector<double>::const_iterator& membershipIt, const int unit): tube()
{
  tube.reserve(size);
  const vector<double>::const_iterator tubeEnd = membershipIt + size;
  do
    {
      tube.push_back(unit * *membershipIt);
    }
  while (++membershipIt != tubeEnd);
}

DenseFuzzyTube::DenseFuzzyTube(const vector<pair<unsigned int, int>>& sparseTube, const int defaultMembership): tube(size, defaultMembership)
{
  for (const pair<unsigned int, int>& entry : sparseTube)
    {
      tube[entry.first] = entry.second;
    }
}

void DenseFuzzyTube::setTuple(const vector<unsigned int>::const_iterator idIt, const int membership)
{
  tube[*idIt] = membership;
}

void DenseFuzzyTube::sumOnPattern(const vector<vector<unsigned int>>::const_iterator dimensionIt, int& sum) const
{
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      sum += tube[*idIt];
    }
  while (++idIt != idEnd);
}

void DenseFuzzyTube::minusSumOnPattern(const vector<vector<unsigned int>>::const_iterator dimensionIt, int& sum) const
{
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      sum -= tube[*idIt];
    }
  while (++idIt != idEnd);
}

void DenseFuzzyTube::setSize(const unsigned int sizeParam)
{
  size = sizeParam;
}

int DenseFuzzyTube::sumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  int sumOnPattern = 0;
  vector<int>::iterator sumIt = sumsIt->begin();
  vector<int>::const_iterator tubeIt;
  {
    // Hyperplanes until the last present one
    const vector<int>::const_iterator tubeBegin = tube.begin();
    tubeIt = tubeBegin;
    const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end();
    vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
    do
      {
	for (const vector<int>::const_iterator end = tubeBegin + *presentElementIdIt; tubeIt != end; ++tubeIt)
	  {
	    *sumIt++ += *tubeIt;
	  }
	*sumIt++ += *tubeIt;
	sumOnPattern += *tubeIt++;
      }
    while (++presentElementIdIt != presentElementIdEnd);
  }
  // Hyperplanes after the last present one
  for (const vector<int>::const_iterator tubeEnd = tube.end(); tubeIt != tubeEnd; ++tubeIt)
    {
      *sumIt++ += *tubeIt;
    }
  return sumOnPattern;
}

int DenseFuzzyTube::minusSumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  int sumOnPattern = 0;
  vector<int>::iterator sumIt = sumsIt->begin();
  vector<int>::const_iterator tubeIt;
  {
    // Hyperplanes until the last present one
    const vector<int>::const_iterator tubeBegin = tube.begin();
    tubeIt = tubeBegin;
    const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end();
    vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
    do
      {
	for (const vector<int>::const_iterator end = tubeBegin + *presentElementIdIt; tubeIt != end; ++tubeIt)
	  {
	    *sumIt++ -= *tubeIt;
	  }
	*sumIt++ -= *tubeIt;
	sumOnPattern += *tubeIt++;
      }
    while (++presentElementIdIt != presentElementIdEnd);
  }
  // Hyperplanes after the last present one
  for (const vector<int>::const_iterator tubeEnd = tube.end(); tubeIt != tubeEnd; ++tubeIt)
    {
      *sumIt++ -= *tubeIt;
    }
  return sumOnPattern;
}

void DenseFuzzyTube::increaseSumsOnHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  // n == 2 and first dimension increased
  vector<int>::iterator sumIt = sumsIt->begin();
  for (const int membership : tube)
    {
      *sumIt++ += membership;
    }
}

void DenseFuzzyTube::decreaseSumsOnHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  // n == 2 and first dimension decreased
  vector<int>::iterator sumIt = sumsIt->begin();
  for (const int membership : tube)
    {
      *sumIt++ -= membership;
    }
}

int DenseFuzzyTube::increaseSumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  // necessarily the increased dimension
  return tube[dimensionIt->front()];
}
