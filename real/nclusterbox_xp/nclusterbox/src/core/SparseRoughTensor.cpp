// Copyright 2018-2022 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "SparseRoughTensor.h"

#include "TupleWithPrediction.h"

SparseRoughTensor::SparseRoughTensor(vector<FuzzyTuple>& fuzzyTuplesParam, const double shiftParam): fuzzyTuples(std::move(fuzzyTuplesParam)), shift(shiftParam)
{
}

Trie SparseRoughTensor::getTensor() const
{
  Trie tensor(cardinalities.begin(), cardinalities.end());
  if (Trie::is01)
    {
      {
	const vector<FuzzyTuple>::const_iterator fuzzyTupleEnd = fuzzyTuples.end();
	vector<FuzzyTuple>::const_iterator fuzzyTupleIt = fuzzyTuples.begin();
	do
	  {
	    tensor.setTuple(fuzzyTupleIt->getTuple().begin());
	  }
	while (++fuzzyTupleIt != fuzzyTupleEnd);
      }
      tensor.sortTubes();
      return tensor;
    }
  {
    const vector<FuzzyTuple>::const_iterator fuzzyTupleEnd = fuzzyTuples.end();
    vector<FuzzyTuple>::const_iterator fuzzyTupleIt = fuzzyTuples.begin();
    do
      {
	tensor.setTuple(fuzzyTupleIt->getTuple().begin(), unit * fuzzyTupleIt->getMembership());
      }
    while (++fuzzyTupleIt != fuzzyTupleEnd);
  }
  tensor.sortTubes();
  return tensor;
}

void SparseRoughTensor::setNoSelection()
{
  fuzzyTuples.clear();
  fuzzyTuples.shrink_to_fit();
}

TrieWithPrediction SparseRoughTensor::projectTensor(const unsigned int nbOfPatternsHavingAllElements)
{
  // Update cardinalities, ids2Labels, candidateVariables, and fuzzyTuples
  const vector<vector<unsigned int>> oldIds2NewIds = projectMetadata(nbOfPatternsHavingAllElements, true);
  vector<FuzzyTuple>::iterator fuzzyTupleEnd = fuzzyTuples.end();
  vector<FuzzyTuple>::iterator fuzzyTupleIt = fuzzyTuples.begin();
  do
    {
      if (fuzzyTupleIt->setNewIds(oldIds2NewIds))
	{
	  ++fuzzyTupleIt;
	}
      else
	{
	  *fuzzyTupleIt = std::move(*--fuzzyTupleEnd);
	}
    }
  while (fuzzyTupleIt != fuzzyTupleEnd);
  fuzzyTuples.erase(fuzzyTupleEnd, fuzzyTuples.end());
  fuzzyTupleIt = fuzzyTuples.begin();
  // Compute negative/positive memberships of elements in first dimension and the RSS of the null model
  vector<unsigned int>::const_iterator cardinalityIt = ++cardinalities.begin();
  double totalShiftOnElementInFirstDimension = shift * *cardinalityIt;
  for (const vector<unsigned int>::const_iterator cardinalityEnd = cardinalities.end(); ++cardinalityIt != cardinalityEnd; )
    {
      totalShiftOnElementInFirstDimension *= *cardinalityIt;
    }
  double rss = totalShiftOnElementInFirstDimension * cardinalities.front() * shift;
  const double squaredShift = shift * shift;
  vector<double> elementPositiveMemberships(cardinalities.front());
  vector<double> elementNegativeMemberships(cardinalities.front(), totalShiftOnElementInFirstDimension);
  do
    {
      const unsigned int elementId = fuzzyTupleIt->getElementId(0);
      const double membership = fuzzyTupleIt->getMembership();
      if (membership > 0)
	{
	  elementPositiveMemberships[elementId] += membership;
	  elementNegativeMemberships[elementId] -= shift;
	}
      else
	{
	  elementNegativeMemberships[elementId] -= membership + shift;
	}
      rss += membership * membership - squaredShift;
    }
  while (++fuzzyTupleIt != fuzzyTupleEnd);
  fuzzyTupleIt = fuzzyTuples.begin();
  // Compute unit
  setUnitForProjectedTensor(rss, elementNegativeMemberships, elementPositiveMemberships);
  // Construct TrieWithPrediction
  TupleWithPrediction::setDefaultMembership(unit * -shift);
  TrieWithPrediction tensor(cardinalities.begin(), cardinalities.end());
  do
    {
      tensor.setTuple(fuzzyTupleIt->getTuple().begin(), unit * fuzzyTupleIt->getMembership());
    }
  while (++fuzzyTupleIt != fuzzyTupleEnd);
  setNoSelection();
  return tensor;
}

double SparseRoughTensor::getAverageShift(const vector<vector<unsigned int>>& nSet) const
{
  return shift;
}
