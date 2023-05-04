// Copyright 2018-2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "TrieWithPrediction.h"

#include "TubeWithPrediction.h"

TrieWithPrediction::TrieWithPrediction(): hyperplanes()
{
}

TrieWithPrediction::TrieWithPrediction(TrieWithPrediction&& otherTrieWithPrediction): hyperplanes(std::move(otherTrieWithPrediction.hyperplanes))
{
}

TrieWithPrediction::TrieWithPrediction(const vector<unsigned int>::const_iterator cardinalityIt, const vector<unsigned int>::const_iterator cardinalityEnd): hyperplanes()
{
  unsigned int hyperplaneId = 0;
  const unsigned int cardinality = *cardinalityIt;
  hyperplanes.reserve(cardinality);
  const vector<unsigned int>::const_iterator nextCardinalityIt = cardinalityIt + 1;
  if (nextCardinalityIt + 1 == cardinalityEnd)
    {
      do
	{
	  hyperplanes.push_back(new TubeWithPrediction(*nextCardinalityIt));
	}
      while (++hyperplaneId != cardinality);
      return;
    }
  do
    {
      hyperplanes.push_back(new TrieWithPrediction(nextCardinalityIt, cardinalityEnd));
    }
  while (++hyperplaneId != cardinality);
}

TrieWithPrediction::TrieWithPrediction(vector<double>::const_iterator& membershipIt, const unsigned int unit, const vector<unsigned int>::const_iterator cardinalityIt, const vector<unsigned int>::const_iterator cardinalityEnd): hyperplanes()
{
  unsigned int hyperplaneId = 0;
  const unsigned int cardinality = *cardinalityIt;
  hyperplanes.reserve(cardinality);
  const vector<unsigned int>::const_iterator nextCardinalityIt = cardinalityIt + 1;
  if (nextCardinalityIt + 1 == cardinalityEnd)
    {
      do
	{
	  hyperplanes.push_back(new TubeWithPrediction(membershipIt, *nextCardinalityIt, unit));
	}
      while (++hyperplaneId != cardinality);
      return;
    }
  do
    {
      hyperplanes.push_back(new TrieWithPrediction(membershipIt, unit, nextCardinalityIt, cardinalityEnd));
    }
  while (++hyperplaneId != cardinality);
}

TrieWithPrediction::~TrieWithPrediction()
{
  for (AbstractDataWithPrediction* hyperplane : hyperplanes)
    {
      delete hyperplane;
    }
}

TrieWithPrediction& TrieWithPrediction::operator=(TrieWithPrediction&& otherTrieWithPrediction)
{
  hyperplanes = std::move(otherTrieWithPrediction.hyperplanes);
  return *this;
}

void TrieWithPrediction::setTuple(const vector<unsigned int>::const_iterator idIt, const int membership)
{
  hyperplanes[*idIt]->setTuple(idIt + 1, membership);
}

void TrieWithPrediction::membershipSumOnSlice(const vector<vector<unsigned int>>::const_iterator dimensionIt, int& sum) const
{
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      hyperplanes[*idIt]->membershipSumOnSlice(nextDimensionIt, sum);
    }
  while (++idIt != idEnd);
}

int TrieWithPrediction::density(const vector<vector<unsigned int>>& nSet) const
{
  long long sum = 0;
  vector<vector<unsigned int>>::const_iterator dimensionIt = ++nSet.begin();
  const vector<unsigned int>::const_iterator idEnd = nSet.front().end();
  vector<unsigned int>::const_iterator idIt = nSet.front().begin();
  do
    {
      int sumOnSlice = 0;
      hyperplanes[*idIt]->membershipSumOnSlice(dimensionIt, sumOnSlice);
      sum += sumOnSlice;
    }
  while (++idIt != idEnd);
  sum /= (dimensionIt - 1)->size();
  const vector<vector<unsigned int>>::const_iterator dimensionEnd = nSet.end();
  do
    {
      sum /= dimensionIt->size();
    }
  while (++dimensionIt != dimensionEnd);
  return sum;
}

void TrieWithPrediction::addFirstPatternToModel(const vector<vector<unsigned int>>& tuples, const int density)
{
  addFirstPatternToModel(tuples.begin(), density);
}

void TrieWithPrediction::addFirstPatternToModel(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int density)
{
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      hyperplanes[*idIt]->addFirstPatternToModel(nextDimensionIt, density);
    }
  while (++idIt != idEnd);
}

void TrieWithPrediction::addPatternToModel(const vector<vector<unsigned int>>& tuples, const int density)
{
  addPatternToModel(tuples.begin(), density);
}

void TrieWithPrediction::addPatternToModel(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int density)
{
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      hyperplanes[*idIt]->addPatternToModel(nextDimensionIt, density);
    }
  while (++idIt != idEnd);
}

long long TrieWithPrediction::deltaOfRSSVariationAdding(const vector<vector<unsigned int>>& tuples, const int minDensityOfSelectedAndUpdated) const
{
  long long delta = 0;
  deltaOfRSSVariationAdding(tuples.begin(), minDensityOfSelectedAndUpdated, delta);
  return delta;
}

void TrieWithPrediction::deltaOfRSSVariationAdding(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int minDensityOfSelectedAndUpdated, long long& delta) const
{
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      hyperplanes[*idIt]->deltaOfRSSVariationAdding(nextDimensionIt, minDensityOfSelectedAndUpdated, delta);
    }
  while (++idIt != idEnd);
}

long long TrieWithPrediction::deltaOfRSSVariationRemovingIfSparserSelected(const vector<vector<unsigned int>> tuples, const int updatedDensity, const int selectedDensity) const
{
  long long delta = 0;
  deltaOfRSSVariationRemovingIfSparserSelected(tuples.begin(), updatedDensity, selectedDensity, delta);
  return delta;
}

void TrieWithPrediction::deltaOfRSSVariationRemovingIfSparserSelected(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int updatedDensity, const int selectedDensity, long long& delta) const
{
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      hyperplanes[*idIt]->deltaOfRSSVariationRemovingIfSparserSelected(nextDimensionIt, updatedDensity, selectedDensity, delta);
    }
  while (++idIt != idEnd);
}

long long TrieWithPrediction::deltaOfRSSVariationRemovingIfDenserSelected(const vector<vector<unsigned int>> tuples, const int updatedDensity) const
{
  long long delta = 0;
  deltaOfRSSVariationRemovingIfDenserSelected(tuples.begin(), updatedDensity, delta);
  return delta;
}

void TrieWithPrediction::deltaOfRSSVariationRemovingIfDenserSelected(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int updatedDensity, long long& delta) const
{
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      hyperplanes[*idIt]->deltaOfRSSVariationRemovingIfDenserSelected(nextDimensionIt, updatedDensity, delta);
    }
  while (++idIt != idEnd);
}

void TrieWithPrediction::reset(const vector<vector<unsigned int>>& tuples)
{
  reset(tuples.begin());
}

void TrieWithPrediction::reset(const vector<vector<unsigned int>>::const_iterator dimensionIt)
{
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      hyperplanes[*idIt]->reset(nextDimensionIt);
    }
  while (++idIt != idEnd);
}
