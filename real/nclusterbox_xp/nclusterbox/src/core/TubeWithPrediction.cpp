// Copyright 2018-2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "TubeWithPrediction.h"

TubeWithPrediction::TubeWithPrediction(): tube()
{
}

TubeWithPrediction::TubeWithPrediction(const unsigned int size): tube(size)
{
}

TubeWithPrediction::TubeWithPrediction(vector<double>::const_iterator& membershipIt, const unsigned int size, const unsigned int unit): tube()
{
  tube.reserve(size);
  const vector<double>::const_iterator tubeEnd = membershipIt + size;
  do
    {
      tube.emplace_back(unit * *membershipIt);
    }
  while (++membershipIt != tubeEnd);
}

void TubeWithPrediction::setTuple(const vector<unsigned int>::const_iterator idIt, const int membership)
{
  tube[*idIt].setRealMembership(membership);
}

void TubeWithPrediction::membershipSumOnSlice(const vector<vector<unsigned int>>::const_iterator dimensionIt, int& sum) const
{
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      sum += tube[*idIt].getRealMembership();
    }
  while (++idIt != idEnd);
}

void TubeWithPrediction::addFirstPatternToModel(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int density)
{
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      tube[*idIt].setEstimatedMembership(density);
    }
  while (++idIt != idEnd);
}

void TubeWithPrediction::addPatternToModel(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int density)
{
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      tube[*idIt].addPrediction(density);
    }
  while (++idIt != idEnd);
}

void TubeWithPrediction::deltaOfRSSVariationAdding(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int minDensityOfSelectedAndUpdated, long long& delta) const
{
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      const TupleWithPrediction& tupleWithPrediction = tube[*idIt];
      if (tupleWithPrediction.isGreaterThanDensest(minDensityOfSelectedAndUpdated))
	{
	  delta += tupleWithPrediction.squaredResidualVariationFromDensest(minDensityOfSelectedAndUpdated);
	}
    }
  while (++idIt != idEnd);
}

void TubeWithPrediction::deltaOfRSSVariationRemovingIfSparserSelected(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int updatedDensity, const int selectedDensity, long long& delta) const
{
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      const TupleWithPrediction& tupleWithPrediction = tube[*idIt];
      if (tupleWithPrediction.isDensest(updatedDensity) && tupleWithPrediction.isGreaterThanSecondDensest(selectedDensity))
	{
	  delta += tupleWithPrediction.squaredResidualVariationFromSecondDensest(selectedDensity);
	}
    }
  while (++idIt != idEnd);
}

void TubeWithPrediction::deltaOfRSSVariationRemovingIfDenserSelected(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int updatedDensity, long long& delta) const
{
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      const TupleWithPrediction& tupleWithPrediction = tube[*idIt];
      if (tupleWithPrediction.isDensest(updatedDensity))
	{
	  delta += tupleWithPrediction.squaredResidualVariationFromSecondDensest(updatedDensity);
	}
    }
  while (++idIt != idEnd);
}

void TubeWithPrediction::reset(const vector<vector<unsigned int>>::const_iterator dimensionIt)
{
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      tube[*idIt].reset();
    }
  while (++idIt != idEnd);
}
