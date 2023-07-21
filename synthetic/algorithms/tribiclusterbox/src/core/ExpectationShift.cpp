// Copyright 2018-2022 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "ExpectationShift.h"

vector<double> reorderElementAvgs(const vector<pair<double, unsigned int>>& newIds2OldIdsInDimension, const vector<double>& elementAvgsInDimension)
{
  vector<pair<double, unsigned int>>::const_iterator oldIdIt = newIds2OldIdsInDimension.begin();
  const vector<pair<double, unsigned int>>::const_iterator oldIdEnd = newIds2OldIdsInDimension.end();
  vector<double> reorderedElementsAvgsInDimension;
  reorderedElementsAvgsInDimension.reserve(oldIdEnd - oldIdIt);
  do
    {
      reorderedElementsAvgsInDimension.push_back(elementAvgsInDimension[oldIdIt->second]);
    }
  while (++oldIdIt != oldIdEnd);
  return reorderedElementsAvgsInDimension;
}

vector<double> reorderElementAvgs(const vector<unsigned int>& newIds2OldIdsInDimension, const vector<double>& elementAvgsInDimension)
{
  vector<unsigned int>::const_iterator oldIdIt = newIds2OldIdsInDimension.begin();
  const vector<unsigned int>::const_iterator oldIdEnd = newIds2OldIdsInDimension.end();
  vector<double> reorderedElementsAvgsInDimension;
  reorderedElementsAvgsInDimension.reserve(oldIdEnd - oldIdIt);
  do
    {
      reorderedElementsAvgsInDimension.push_back(elementAvgsInDimension[*oldIdIt]);
    }
  while (++oldIdIt != oldIdEnd);
  return reorderedElementsAvgsInDimension;
}

ExpectationShift::ExpectationShift(const vector<FuzzyTuple>& fuzzyTuples, const vector<vector<string>>& ids2Labels): elementAvgs()
{
  vector<vector<string>>::const_iterator ids2LabelsInDimensionIt = ids2Labels.begin();
  const vector<vector<string>>::const_iterator ids2LabelsInDimensionEnd = ids2Labels.end();
  elementAvgs.reserve(ids2LabelsInDimensionEnd - ids2LabelsInDimensionIt);
  elementAvgs.emplace_back(vector<double>(ids2LabelsInDimensionIt->size()));
  unsigned long long area = ids2LabelsInDimensionIt->size();
  ++ids2LabelsInDimensionIt;
  do
    {
      area *= ids2LabelsInDimensionIt->size();
      elementAvgs.emplace_back(vector<double>(ids2LabelsInDimensionIt->size()));
    }
  while (++ids2LabelsInDimensionIt != ids2LabelsInDimensionEnd);
  ids2LabelsInDimensionIt = ids2Labels.begin();
  const vector<FuzzyTuple>::const_iterator fuzzyTupleEnd = fuzzyTuples.end();
  vector<FuzzyTuple>::const_iterator fuzzyTupleIt = fuzzyTuples.begin();
  do
    {
      vector<vector<double>>::iterator elementAvgsInDimensionIt = elementAvgs.begin();
      vector<unsigned int>::const_iterator elementIt = fuzzyTupleIt->getTuple().begin();
      (*elementAvgsInDimensionIt)[*elementIt] += fuzzyTupleIt->getMembership();
      ++elementAvgsInDimensionIt;
      const vector<vector<double>>::iterator elementAvgsInDimensionEnd = elementAvgs.end();
      do
	{
	  (*elementAvgsInDimensionIt)[*++elementIt] += fuzzyTupleIt->getMembership();
	}
      while (++elementAvgsInDimensionIt != elementAvgsInDimensionEnd);
    }
  while (++fuzzyTupleIt != fuzzyTupleEnd);

  unsigned int areaOfElementInDimension = area / ids2LabelsInDimensionIt->size();
  ++ids2LabelsInDimensionIt;
  vector<vector<double>>::iterator elementAvgsInDimensionIt = elementAvgs.begin();
  vector<double>::iterator elementAvgEnd = elementAvgsInDimensionIt->end();
  vector<double>::iterator elementAvgIt = elementAvgsInDimensionIt->begin();
  do
    {
      *elementAvgIt /= areaOfElementInDimension;
    }
  while (++elementAvgIt != elementAvgEnd);
  do
    {
      areaOfElementInDimension = area / ids2LabelsInDimensionIt->size();
      elementAvgEnd = (++elementAvgsInDimensionIt)->end();
      elementAvgIt = elementAvgsInDimensionIt->begin();
      do
	{
	  *elementAvgIt /= areaOfElementInDimension;
	}
      while (++elementAvgIt != elementAvgEnd);
    }
  while (++ids2LabelsInDimensionIt != ids2LabelsInDimensionEnd);
}

double ExpectationShift::getShift(const vector<unsigned int>& tuple) const
{
  vector<vector<double>>::const_iterator elementAvgsInDimensionIt = elementAvgs.begin();
  vector<unsigned int>::const_iterator elementIt = tuple.begin();
  double max = (*elementAvgsInDimensionIt)[*elementIt];
  ++elementIt;
  const vector<unsigned int>::const_iterator end = tuple.end();
  do
    {
      const double elementAvg = (*++elementAvgsInDimensionIt)[*elementIt];
      if (elementAvg > max)
	{
	  max = elementAvg;
	}
    }
  while (++elementIt != end);
  return max;
}

double ExpectationShift::getAverageShift(const vector<vector<unsigned int>>& nSet) const
{
  vector<vector<unsigned int>::const_iterator> tupleIts;
  tupleIts.reserve(nSet.size());
  const vector<vector<unsigned int>>::const_iterator dimensionBegin = nSet.begin();
  vector<vector<unsigned int>>::const_iterator dimensionIt = dimensionBegin;
  tupleIts.push_back(dimensionIt->begin());
  long long area = dimensionIt->size();
  ++dimensionIt;
  const vector<vector<unsigned int>>::const_iterator dimensionEnd = nSet.end();
  do
    {
      area *= dimensionIt->size();
      tupleIts.push_back(dimensionIt->begin());
    }
  while (++dimensionIt != dimensionEnd);
  long long expectedSumOnNSet = 0;
  const vector<vector<unsigned int>::const_iterator>::iterator tupleItsEnd = tupleIts.end();
  vector<vector<unsigned int>::const_iterator>::iterator tupleItsIt = tupleIts.begin();
  do
    {
      // Search max average across the elements that tupleIts points
      tupleItsIt = tupleIts.begin();
      vector<vector<double>>::const_iterator elementAvgsInDimensionIt = elementAvgs.begin();
      double maxAvg = (*elementAvgsInDimensionIt)[**tupleItsIt];
      ++tupleItsIt;
      do
	{
	  if ((*++elementAvgsInDimensionIt)[**tupleItsIt] > maxAvg)
	    {
	      maxAvg = (*elementAvgsInDimensionIt)[**tupleItsIt];
	    }
	}
      while (++tupleItsIt != tupleItsEnd);
      expectedSumOnNSet += maxAvg;
      // Advance tupleIts, little-endian-like
      tupleItsIt = tupleIts.begin();
      for (dimensionIt = dimensionBegin; dimensionIt != dimensionEnd && ++*tupleItsIt == dimensionIt->end(); ++dimensionIt)
	{
	  *tupleItsIt++ = dimensionIt->begin();
	}
    }
  while (tupleItsIt != tupleItsEnd);
  return expectedSumOnNSet / area;
}

void ExpectationShift::setNewDimensionOrderAndNewIds(const vector<unsigned int>& old2NewDimensionOrder, const vector<vector<pair<double, unsigned int>>>& elementPositiveMemberships)
{
  vector<vector<double>> reorderedElementsAvg(elementAvgs.size());
  vector<vector<pair<double, unsigned int>>>::const_iterator elementPositiveMembershipsInDimensionIt = elementPositiveMemberships.begin();
  vector<vector<double>>::const_iterator elementAvgsInDimensionIt = elementAvgs.begin();
  vector<unsigned int>::const_iterator newDimensionIdIt = old2NewDimensionOrder.begin();
  reorderedElementsAvg[*newDimensionIdIt] = reorderElementAvgs(*elementPositiveMembershipsInDimensionIt, *elementAvgsInDimensionIt);
  ++newDimensionIdIt;
  const vector<unsigned int>::const_iterator newDimensionIdEnd = old2NewDimensionOrder.end();
  do
    {
      reorderedElementsAvg[*newDimensionIdIt] = reorderElementAvgs(*++elementPositiveMembershipsInDimensionIt, *++elementAvgsInDimensionIt);
    }
  while (++newDimensionIdIt != newDimensionIdEnd);
  elementAvgs = std::move(reorderedElementsAvg);
}

void ExpectationShift::setNewIds(const vector<vector<unsigned int>>& newIds2OldIds)
{
  vector<vector<double>>::iterator elementAvgsInDimensionIt = elementAvgs.begin();
  vector<vector<unsigned int>>::const_iterator newIds2OldIdsInDimensionIt = newIds2OldIds.begin();
  *elementAvgsInDimensionIt++ = reorderElementAvgs(*newIds2OldIdsInDimensionIt, *elementAvgsInDimensionIt);
  const vector<vector<double>>::iterator elementAvgsInDimensionEnd = elementAvgs.end();
  do
    {
      *elementAvgsInDimensionIt = reorderElementAvgs(*++newIds2OldIdsInDimensionIt, *elementAvgsInDimensionIt);
    }
  while (++elementAvgsInDimensionIt != elementAvgsInDimensionEnd);
}
