// Copyright 2018-2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "DenseRoughTensor.h"

#include <algorithm>

#include "ConstantShift.h"
#include "ExpectationShift.h"
#include "ConcurrentPatternPool.h"
#include "SparseCrispTube.h"
#include "DenseCrispTube.h"
#include "DenseFuzzyTube.h"

vector<pair<double, unsigned int>> zeroForEveryElementAndPushBackLastElement(const unsigned int nbOfElements, vector<unsigned int>& lastElements)
{
  lastElements.push_back(nbOfElements - 1);
  vector<pair<double, unsigned int>> elementPositiveMembershipsInDimension;
  elementPositiveMembershipsInDimension.reserve(nbOfElements);
  unsigned int id = 0;
  do
    {
      elementPositiveMembershipsInDimension.emplace_back(0, id);
    }
  while (++id != nbOfElements);
  return elementPositiveMembershipsInDimension;
}

void updateElementMemberships(const vector<unsigned int>& tuple, const double shiftedMembership, vector<vector<pair<double, unsigned int>>>& elementMemberships)
{
  vector<vector<pair<double, unsigned int>>>::iterator elementMembershipsInDimensionIt = elementMemberships.begin();
  const vector<unsigned int>::const_iterator idEnd = tuple.end();
  vector<unsigned int>::const_iterator idIt = tuple.begin();
  (*elementMembershipsInDimensionIt)[*idIt].first += shiftedMembership;
  ++idIt;
  do
    {
      (*++elementMembershipsInDimensionIt)[*idIt].first += shiftedMembership;
    }
  while (++idIt != idEnd);
}

DenseRoughTensor::DenseRoughTensor(const char* tensorFileName, const char* inputDimensionSeparator, const char* inputElementSeparator, const bool isInput01, const bool isVerbose): shift(), memberships()
{
  vector<FuzzyTuple> fuzzyTuples = getFuzzyTuples(tensorFileName, inputDimensionSeparator, inputElementSeparator, isInput01, isVerbose);
  Trie::is01 = false;
  shift = new ExpectationShift(fuzzyTuples, ids2Labels);
  init(fuzzyTuples);
}

DenseRoughTensor::DenseRoughTensor(vector<FuzzyTuple>& fuzzyTuples, const double constantShift): shift(new ConstantShift(constantShift)), memberships()
{
  init(fuzzyTuples);
  if (Trie::is01)
    {
      SparseCrispTube::setDefaultMembership(unit * -constantShift);
    }
}

DenseRoughTensor::~DenseRoughTensor()
{
  delete shift;
}

void DenseRoughTensor::init(vector<FuzzyTuple>& fuzzyTuples)
{
  nullModelRSS = 0;
  // Inform ConcurrentPatternPool, responsible of computing the initial patterns, of the number of dimensions
  ConcurrentPatternPool::setNbOfDimensions(ids2Labels.size());
  // Initialize tuple and positive/negative memberships of the elements
  vector<double> shiftedMemberships;
  unsigned long long nbOfTuples;
  vector<unsigned int> tuple;
  tuple.reserve(ids2Labels.size());
  vector<vector<pair<double, unsigned int>>> elementPositiveMemberships;
  elementPositiveMemberships.reserve(ids2Labels.size());
  double minElementNegativeMembership;
  if (Trie::is01)
    {
      // Consider a slice with every membership null to define minElementNegativeMembership and, consequently, unit, so that SparseCrispTube::getDefaultMembership() * area (with area the area of a slice) fits in an int (e.g., in Trie::increaseSumsOnHyperplanes)
      vector<vector<string>>::const_iterator labelsInDimensionIt = ids2Labels.begin();
      nbOfTuples = labelsInDimensionIt->size();
      unsigned int minCardinality = nbOfTuples;
      elementPositiveMemberships.emplace_back(zeroForEveryElementAndPushBackLastElement(minCardinality, tuple));
      ++labelsInDimensionIt;
      const vector<vector<string>>::const_iterator labelsInDimensionEnd = ids2Labels.end();
      do
	{
	  const unsigned int nbOfElements = labelsInDimensionIt->size();
	  if (nbOfElements < minCardinality)
	    {
	      minCardinality = nbOfElements;
	    }
	  nbOfTuples *= nbOfElements;
	  elementPositiveMemberships.emplace_back(zeroForEveryElementAndPushBackLastElement(nbOfElements, tuple));
	}
      while (++labelsInDimensionIt != labelsInDimensionEnd);
      shiftedMemberships.resize(nbOfTuples);
      vector<double>::reverse_iterator shiftedMembershipIt = shiftedMemberships.rbegin(); // filled backwards, so that in lexicographic order of the tuples
      const double constantShift = -shift->getShift(tuple); // necessary constant (otherwise !is01)
      minElementNegativeMembership = constantShift * (nbOfTuples / minCardinality);
      const vector<FuzzyTuple>::const_iterator end = --fuzzyTuples.end();
      vector<FuzzyTuple>::const_iterator fuzzyTupleIt = fuzzyTuples.begin();
      do
	{
	  // Comparing in the reverse order because the last ids are more likely to be different
	  while (!equal(tuple.rbegin(), tuple.rend(), fuzzyTupleIt->getTuple().rbegin()))
	    {
	      *shiftedMembershipIt++ = updateNullModelRSSAndElementMembershipsAndAdvance(tuple, constantShift, elementPositiveMemberships);
	    }
	  ConcurrentPatternPool::addFuzzyTuple(tuple, fuzzyTupleIt->getMembership() + constantShift);
	  *shiftedMembershipIt++ = updateNullModelRSSAndElementMembershipsAndAdvance(tuple, fuzzyTupleIt->getMembership() + constantShift, elementPositiveMemberships);
	}
      while (++fuzzyTupleIt != end);
      // fuzzyTupleIt->getTuple() is necessarily a vector of zero (first fuzzy tuple that was read): no more tuple
      // Comparing in the reverse order because the last ids are more likely to be different
      while (!equal(tuple.rbegin(), tuple.rend(), fuzzyTupleIt->getTuple().rbegin()))
	{
	  *shiftedMembershipIt++ = updateNullModelRSSAndElementMembershipsAndAdvance(tuple, constantShift, elementPositiveMemberships);
	}
      *shiftedMembershipIt = fuzzyTupleIt->getMembership() + constantShift;
      updateNullModelRSSAndElementMemberships(tuple, *shiftedMembershipIt, elementPositiveMemberships);
    }
  else
    {
      vector<vector<string>>::const_iterator labelsInDimensionIt = ids2Labels.begin();
      nbOfTuples = labelsInDimensionIt->size();
      elementPositiveMemberships.emplace_back(zeroForEveryElementAndPushBackLastElement(nbOfTuples, tuple));
      vector<vector<double>> elementNegativeMemberships;
      elementNegativeMemberships.reserve(ids2Labels.size());
      elementNegativeMemberships.emplace_back(nbOfTuples);
      ++labelsInDimensionIt;
      const vector<vector<string>>::const_iterator labelsInDimensionEnd = ids2Labels.end();
      do
	{
	  const unsigned int nbOfElements = labelsInDimensionIt->size();
	  nbOfTuples *= nbOfElements;
	  elementPositiveMemberships.emplace_back(zeroForEveryElementAndPushBackLastElement(nbOfElements, tuple));
	  elementNegativeMemberships.emplace_back(nbOfElements);
	}
      while (++labelsInDimensionIt != labelsInDimensionEnd);
      shiftedMemberships.resize(nbOfTuples);
      vector<double>::reverse_iterator shiftedMembershipIt = shiftedMemberships.rbegin(); // filled backwards, so that in lexicographic order of the tuples
      const vector<FuzzyTuple>::const_iterator end = --fuzzyTuples.end();
      vector<FuzzyTuple>::const_iterator fuzzyTupleIt = fuzzyTuples.begin();
      do
	{
	  // Comparing in the reverse order because the last ids are more likely to be different
	  while (!equal(tuple.rbegin(), tuple.rend(), fuzzyTupleIt->getTuple().rbegin()))
	    {
	      *shiftedMembershipIt++ = updateNullModelRSSAndElementMembershipsAndAdvance(tuple, -shift->getShift(tuple), elementPositiveMemberships, elementNegativeMemberships);
	    }
	  const double shiftForThisTuple = shift->getShift(tuple);
	  ConcurrentPatternPool::addFuzzyTuple(tuple, fuzzyTupleIt->getMembership() - shiftForThisTuple);
	  *shiftedMembershipIt++ = updateNullModelRSSAndElementMembershipsAndAdvance(tuple, fuzzyTupleIt->getMembership() - shiftForThisTuple, elementPositiveMemberships, elementNegativeMemberships);
	}
      while (++fuzzyTupleIt != end);
      // fuzzyTupleIt->getTuple() is necessarily a vector of zero (first fuzzy tuple that was read): no more tuple
      // Comparing in the reverse order because the last ids are more likely to be different
      while (!equal(tuple.rbegin(), tuple.rend(), fuzzyTupleIt->getTuple().rbegin()))
	{
	  *shiftedMembershipIt++ = updateNullModelRSSAndElementMembershipsAndAdvance(tuple, -shift->getShift(tuple), elementPositiveMemberships, elementNegativeMemberships);
	}
      *shiftedMembershipIt = fuzzyTupleIt->getMembership() - shift->getShift(tuple);
      updateNullModelRSSAndElementMemberships(tuple, *shiftedMembershipIt, elementPositiveMemberships, elementNegativeMemberships);
      vector<vector<double>>::const_iterator elementNegativeMembershipsIt = elementNegativeMemberships.begin();
      minElementNegativeMembership = *min_element(elementNegativeMembershipsIt->begin(), elementNegativeMembershipsIt->end());
      ++elementNegativeMembershipsIt;
      const vector<vector<double>>::const_iterator elementNegativeMembershipsEnd = elementNegativeMemberships.end();
      do
	{
	  const double minElementNegativeMembershipInDimension = *min_element(elementNegativeMembershipsIt->begin(), elementNegativeMembershipsIt->end());
	  if (minElementNegativeMembershipInDimension < minElementNegativeMembership)
	    {
	      minElementNegativeMembership = minElementNegativeMembershipInDimension;
	    }
	}
      while (++elementNegativeMembershipsIt != elementNegativeMembershipsEnd);
    }
  fuzzyTuples.clear();
  fuzzyTuples.shrink_to_fit();
  // Compute new ids, in increasing order of element membership, set unit, cardinalities and external2InternalDimensionOrder
  setMetadata(elementPositiveMemberships, -minElementNegativeMembership);
  // Inform the shift of the new dimension order
  shift->setNewDimensionOrderAndNewIds(external2InternalDimensionOrder, elementPositiveMemberships);
  // Inform ConcurrentPatternPool, responsible of computing the initial patterns, of the new dimension order
  ConcurrentPatternPool::setNewDimensionOrderAndNewIds(external2InternalDimensionOrder, elementPositiveMemberships);
  // Compute the offsets to access shiftedMemberships and init tuple to the last one
  vector<unsigned int> offsets(ids2Labels.size(), 1);
  {
    const vector<unsigned int>::reverse_iterator rend = offsets.rend();
    vector<unsigned int>::reverse_iterator offsetIt = ++offsets.rbegin();
    vector<unsigned int>::iterator tupleIt = tuple.begin();
    vector<unsigned int>::const_iterator cardinalityIt = cardinalities.begin();
    *tupleIt = *cardinalityIt - 1;
    vector<vector<string>>::const_reverse_iterator labelsInDimensionIt = ids2Labels.rbegin();
    unsigned int offset = labelsInDimensionIt->size();
    do
      {
	*offsetIt = offset;
	offset *= (++labelsInDimensionIt)->size();
	*++tupleIt = *++cardinalityIt - 1;
      }
    while (++offsetIt != rend);
  }
  // Compute memberships, reodering of shiftedMemberships according to external2InternalDimensionOrder
  memberships.resize(nbOfTuples);
  for (vector<double>::reverse_iterator membershipIt = memberships.rbegin(); ; ++membershipIt) // filled backwards, so that in lexicographic order of the tuples
    {
      vector<vector<pair<double, unsigned int>>>::const_iterator elementPositiveMembershipsInDimensionIt = elementPositiveMemberships.begin();
      vector<unsigned int>::const_iterator offsetIt = offsets.begin();
      vector<unsigned int>::const_iterator internalDimensionIdIt = external2InternalDimensionOrder.begin();
      unsigned long long membershipIndex = (*elementPositiveMembershipsInDimensionIt)[tuple[*internalDimensionIdIt]].second * *offsetIt;
      ++internalDimensionIdIt;
      const vector<unsigned int>::const_iterator internalDimensionIdEnd = external2InternalDimensionOrder.end();
      do
	{
	  membershipIndex += (*++elementPositiveMembershipsInDimensionIt)[tuple[*internalDimensionIdIt]].second * *++offsetIt;
	}
      while (++internalDimensionIdIt != internalDimensionIdEnd);
      *membershipIt = shiftedMemberships[membershipIndex];
      if (!--nbOfTuples)
	{
	  break;
	}
      vector<unsigned int>::reverse_iterator elementIt = tuple.rbegin();
      for (vector<unsigned int>::const_reverse_iterator cardinalityIt = cardinalities.rbegin(); !*elementIt; ++cardinalityIt)
	{
	  *elementIt++ = *cardinalityIt - 1;
	}
      --*elementIt;
    }
}

double DenseRoughTensor::updateNullModelRSSAndElementMembershipsAndAdvance(vector<unsigned int>& tuple, const double shiftedMembership, vector<vector<pair<double, unsigned int>>>& elementPositiveMemberships)
{
  updateNullModelRSSAndElementMemberships(tuple, shiftedMembership, elementPositiveMemberships);
  // Advance tuple, big-endian-like
  vector<unsigned int>::reverse_iterator elementIt = tuple.rbegin();
  for (vector<vector<string>>::const_reverse_iterator labelsInDimensionReverseIt = ids2Labels.rbegin(); !*elementIt; ++labelsInDimensionReverseIt)
    {
      *elementIt++ = labelsInDimensionReverseIt->size() - 1;
    }
  --*elementIt;
  return shiftedMembership;
}

double DenseRoughTensor::updateNullModelRSSAndElementMembershipsAndAdvance(vector<unsigned int>& tuple, const double shiftedMembership, vector<vector<pair<double, unsigned int>>>& elementPositiveMemberships, vector<vector<double>>& elementNegativeMemberships)
{
  updateNullModelRSSAndElementMemberships(tuple, shiftedMembership, elementPositiveMemberships, elementNegativeMemberships);
  // Advance tuple, big-endian-like
  vector<unsigned int>::reverse_iterator elementIt = tuple.rbegin();
  for (vector<vector<string>>::const_reverse_iterator labelsInDimensionReverseIt = ids2Labels.rbegin(); !*elementIt; ++labelsInDimensionReverseIt)
    {
      *elementIt++ = labelsInDimensionReverseIt->size() - 1;
    }
  --*elementIt;
  return shiftedMembership;
}

void DenseRoughTensor::updateNullModelRSSAndElementMemberships(const vector<unsigned int>& tuple, const double shiftedMembership, vector<vector<pair<double, unsigned int>>>& elementPositiveMemberships)
{
  nullModelRSS += shiftedMembership * shiftedMembership; // the RSS of the null model on the tuples read so far
  if (shiftedMembership > 0)
    {
      updateElementMemberships(tuple, shiftedMembership, elementPositiveMemberships);
    }
}

void DenseRoughTensor::updateNullModelRSSAndElementMemberships(const vector<unsigned int>& tuple, const double shiftedMembership, vector<vector<pair<double, unsigned int>>>& elementPositiveMemberships, vector<vector<double>>& elementNegativeMemberships)
{
  nullModelRSS += shiftedMembership * shiftedMembership; // the RSS of the null model on the tuples read so far
  if (shiftedMembership > 0)
    {
      updateElementMemberships(tuple, shiftedMembership, elementPositiveMemberships);
      return;
    }
  vector<vector<double>>::iterator elementNegativeMembershipsInDimensionIt = elementNegativeMemberships.begin();
  const vector<unsigned int>::const_iterator idEnd = tuple.end();
  vector<unsigned int>::const_iterator idIt = tuple.begin();
  (*elementNegativeMembershipsInDimensionIt)[*idIt] += shiftedMembership;
  ++idIt;
  do
    {
      (*++elementNegativeMembershipsInDimensionIt)[*idIt] += shiftedMembership;
    }
  while (++idIt != idEnd);
}

Trie DenseRoughTensor::getTensor() const
{
  vector<double>::const_iterator membershipIt = memberships.begin();
  if (Trie::is01)
    {
      DenseCrispTube::setSize(cardinalities.back());
      return Trie(membershipIt, cardinalities.begin(), cardinalities.end());
    }
  DenseFuzzyTube::setSize(cardinalities.back());
  return Trie(membershipIt, unit, cardinalities.begin(), cardinalities.end());
}

void DenseRoughTensor::setNoSelection()
{
  memberships.clear();
  memberships.shrink_to_fit();
}

TrieWithPrediction DenseRoughTensor::projectTensor(const unsigned int nbOfPatternsHavingAllElements)
{
  // Compute the offsets to access memberships from the non-updated cardinalities
  vector<unsigned int>::const_reverse_iterator cardinalityRIt = cardinalities.rbegin();
  unsigned int offset = *cardinalityRIt;
  vector<unsigned int> offsets(cardinalities.size() - 1, offset);
  const vector<unsigned int>::reverse_iterator offsetREnd = offsets.rend();
  for (vector<unsigned int>::reverse_iterator offsetRIt = offsets.rbegin(); ++offsetRIt != offsetREnd; )
    {
      offset *= *++cardinalityRIt;
      *offsetRIt = offset;
    }
  // Update cardinalities, ids2Labels and candidateVariables
  const vector<vector<unsigned int>> newIds2OldIds = projectMetadata(nbOfPatternsHavingAllElements, false);
  // Inform the shift of the new ids
  shift->setNewIds(newIds2OldIds);
  // Compute last tuple and nb of tuples, according to new cardinalities
  vector<unsigned int>::const_iterator cardinalityIt = cardinalities.begin();
  const vector<unsigned int>::const_iterator cardinalityEnd = cardinalities.end();
  vector<unsigned int> tuple;
  tuple.reserve(cardinalityEnd - cardinalityIt);
  tuple.push_back(*cardinalityIt - 1);
  unsigned long long nbOfSelectedTuples = *cardinalityIt++;
  do
    {
      nbOfSelectedTuples *= *cardinalityIt;
      tuple.push_back(*cardinalityIt - 1);
    }
  while (++cardinalityIt != cardinalityEnd);
  // Select memberships in reduced tensor, compute negative/positive memberships of elements in first dimension and the RSS of the null model
  double rss = 0;
  vector<double> elementNegativeMemberships(cardinalities.front());
  vector<double> elementPositiveMemberships(cardinalities.front());
  vector<double> selectedMemberships(nbOfSelectedTuples);
  for (vector<double>::reverse_iterator selectedMembershipIt = selectedMemberships.rbegin(); ; ++selectedMembershipIt) // filled backwards, so that in lexicographic order of the tuples
    {
      vector<vector<unsigned int>>::const_iterator newIds2OldIdsIt = newIds2OldIds.begin();
      vector<unsigned int>::const_iterator idIt = tuple.begin();
      vector<unsigned int>::const_iterator offsetIt = offsets.begin();
      unsigned long long membershipIndex = *offsetIt * (*newIds2OldIdsIt)[*idIt];
      for (const vector<unsigned int>::const_iterator offsetEnd = offsets.end(); ++offsetIt != offsetEnd; )
	{
	  membershipIndex += *offsetIt * (*++newIds2OldIdsIt)[*++idIt];
	}
      const double membership = memberships[membershipIndex + (*++newIds2OldIdsIt)[*++idIt]];
      *selectedMembershipIt = membership;
      if (membership < 0)
	{
	  elementNegativeMemberships[tuple.front()] -= membership;
	}
      else
	{
	  elementPositiveMemberships[tuple.front()] += membership;
	}
      rss += membership * membership;
      if (--nbOfSelectedTuples == 0)
	{
	  break;
	}
      vector<unsigned int>::reverse_iterator elementIt = tuple.rbegin();
      for (vector<unsigned int>::const_reverse_iterator cardinalityIt = cardinalities.rbegin(); !*elementIt; ++cardinalityIt)
	{
	  *elementIt++ = *cardinalityIt - 1;
	}
      --*elementIt;
    }
  setNoSelection();
  // Compute unit
  setUnitForProjectedTensor(rss, elementNegativeMemberships, elementPositiveMemberships);
  // Construct TrieWithPrediction
  vector<double>::const_iterator selectedMembershipIt = selectedMemberships.begin();
  return TrieWithPrediction(selectedMembershipIt, unit, cardinalities.begin(), cardinalities.end());
}

double DenseRoughTensor::getAverageShift(const vector<vector<unsigned int>>& nSet) const
{
  return shift->getAverageShift(nSet);
}
