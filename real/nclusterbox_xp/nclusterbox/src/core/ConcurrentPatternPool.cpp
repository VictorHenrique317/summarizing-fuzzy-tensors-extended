// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "ConcurrentPatternPool.h"

#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>
#include <iomanip>
#include <thread>

#include "AbstractRoughTensor.h"

vector<vector<vector<unsigned int>>> ConcurrentPatternPool::patterns;
mutex ConcurrentPatternPool::patternsLock;
condition_variable ConcurrentPatternPool::cv;
bool ConcurrentPatternPool::isDefaultInitialPatterns;
bool ConcurrentPatternPool::isUnboundedNumberOfPatterns;
bool ConcurrentPatternPool::isAllPatternsAdded;
unsigned long long ConcurrentPatternPool::nbOfFreeSlots;
vector<pair<vector<unsigned int>, double>> ConcurrentPatternPool::tuplesWithHighestMembershipDegrees;
vector<vector<unsigned int>> ConcurrentPatternPool::additionalTuplesWithLowestAmongHighestMembershipDegrees;
vector<unsigned int> ConcurrentPatternPool::old2NewDimensionOrder;
vector<vector<unsigned int>> ConcurrentPatternPool::oldIds2NewIds;

void ConcurrentPatternPool::setReadFromFile()
{
  isDefaultInitialPatterns = false;
  isAllPatternsAdded = false;
}

void ConcurrentPatternPool::setDefaultPatterns(const unsigned long long maxNbOfPatterns)
{
  isDefaultInitialPatterns = true;
  isAllPatternsAdded = false;
  if (maxNbOfPatterns)
    {
      isUnboundedNumberOfPatterns = false;
      nbOfFreeSlots = maxNbOfPatterns;
      tuplesWithHighestMembershipDegrees.reserve(maxNbOfPatterns);
      return;
    }
  isUnboundedNumberOfPatterns = true;
}

bool ConcurrentPatternPool::readFromFile()
{
  if (isDefaultInitialPatterns)
    {
      if (old2NewDimensionOrder.empty())
	{
	  if (isUnboundedNumberOfPatterns)
	    {
	      for_each(additionalTuplesWithLowestAmongHighestMembershipDegrees.cbegin(), additionalTuplesWithLowestAmongHighestMembershipDegrees.cend(), addDefaultPattern);
	      allPatternsAdded();
	      additionalTuplesWithLowestAmongHighestMembershipDegrees.clear();
	      additionalTuplesWithLowestAmongHighestMembershipDegrees.shrink_to_fit();
	      return false;
	    }
	  if (additionalTuplesWithLowestAmongHighestMembershipDegrees.empty())
	    {
	      for_each(tuplesWithHighestMembershipDegrees.cbegin(), tuplesWithHighestMembershipDegrees.cend(), [](const pair<vector<unsigned int>, double>& fuzzyTuple) { addDefaultPattern(fuzzyTuple.first); });
	      allPatternsAdded();
	    }
	  else
	    {
	      const unsigned int nbOfAdditionalTuples = additionalTuplesWithLowestAmongHighestMembershipDegrees.size();
	      {
		// insert in additionalTuplesWithLowestAmongHighestMembershipDegrees the tuples in tuplesWithHighestMembershipDegrees with the min memberhip degree and insert the remaining ones in patterns
		const vector<pair<vector<unsigned int>, double>>::const_iterator lastFuzzyTupleIt = --tuplesWithHighestMembershipDegrees.end();
		for (vector<pair<vector<unsigned int>, double>>::const_iterator fuzzyTupleIt = tuplesWithHighestMembershipDegrees.begin(); fuzzyTupleIt != lastFuzzyTupleIt; ++fuzzyTupleIt)
		  {
		    if (fuzzyTupleIt->second == tuplesWithHighestMembershipDegrees.back().second)
		      {
			additionalTuplesWithLowestAmongHighestMembershipDegrees.emplace_back(std::move(fuzzyTupleIt->first));
			continue;
		      }
		    addDefaultPattern(fuzzyTupleIt->first);
		  }
		additionalTuplesWithLowestAmongHighestMembershipDegrees.emplace_back(std::move(lastFuzzyTupleIt->first));
	      }
	      // insert in patterns random tuples from additionalTuplesWithLowestAmongHighestMembershipDegrees
	      vector<vector<unsigned int>>::iterator additionalTupleIt = additionalTuplesWithLowestAmongHighestMembershipDegrees.begin();
	      shuffle(additionalTupleIt, additionalTuplesWithLowestAmongHighestMembershipDegrees.end(), mt19937(random_device()()));
	      const vector<vector<unsigned int>>::iterator additionalTupleEnd = additionalTupleIt + (additionalTuplesWithLowestAmongHighestMembershipDegrees.size() - nbOfAdditionalTuples);
	      do
		{
		  addDefaultPattern(*additionalTupleIt);
		}
	      while (++additionalTupleIt != additionalTupleEnd);
	      allPatternsAdded();
	      additionalTuplesWithLowestAmongHighestMembershipDegrees.clear();
	      additionalTuplesWithLowestAmongHighestMembershipDegrees.shrink_to_fit();
	    }
	  tuplesWithHighestMembershipDegrees.clear();
	  tuplesWithHighestMembershipDegrees.shrink_to_fit();
	  return false;
	}
      if (isUnboundedNumberOfPatterns)
	{
	  for_each(additionalTuplesWithLowestAmongHighestMembershipDegrees.cbegin(), additionalTuplesWithLowestAmongHighestMembershipDegrees.cend(), addRemappedDefaultPattern);
	  allPatternsAdded();
	  additionalTuplesWithLowestAmongHighestMembershipDegrees.clear();
	  additionalTuplesWithLowestAmongHighestMembershipDegrees.shrink_to_fit();
	}
      else
	{
	  if (additionalTuplesWithLowestAmongHighestMembershipDegrees.empty())
	    {
	      for_each(tuplesWithHighestMembershipDegrees.cbegin(), tuplesWithHighestMembershipDegrees.cend(), [](const pair<vector<unsigned int>, double>& fuzzyTuple) { addRemappedDefaultPattern(fuzzyTuple.first); });
	      allPatternsAdded();
	    }
	  else
	    {
	      const unsigned int nbOfAdditionalTuples = additionalTuplesWithLowestAmongHighestMembershipDegrees.size();
	      {
		// insert in additionalTuplesWithLowestAmongHighestMembershipDegrees the tuples in tuplesWithHighestMembershipDegrees with the min memberhip degree and insert the remaining ones in patterns
		const vector<pair<vector<unsigned int>, double>>::const_iterator lastFuzzyTupleIt = --tuplesWithHighestMembershipDegrees.end();
		for (vector<pair<vector<unsigned int>, double>>::const_iterator fuzzyTupleIt = tuplesWithHighestMembershipDegrees.begin(); fuzzyTupleIt != lastFuzzyTupleIt; ++fuzzyTupleIt)
		  {
		    if (fuzzyTupleIt->second == tuplesWithHighestMembershipDegrees.back().second)
		      {
			additionalTuplesWithLowestAmongHighestMembershipDegrees.emplace_back(std::move(fuzzyTupleIt->first));
			continue;
		      }
		    addRemappedDefaultPattern(fuzzyTupleIt->first);
		  }
		additionalTuplesWithLowestAmongHighestMembershipDegrees.emplace_back(std::move(lastFuzzyTupleIt->first));
	      }
	      // insert in patterns random tuples from additionalTuplesWithLowestAmongHighestMembershipDegrees
	      vector<vector<unsigned int>>::iterator additionalTupleIt = additionalTuplesWithLowestAmongHighestMembershipDegrees.begin();
	      shuffle(additionalTupleIt, additionalTuplesWithLowestAmongHighestMembershipDegrees.end(), mt19937(random_device()()));
	      const vector<vector<unsigned int>>::iterator additionalTupleEnd = additionalTupleIt + (additionalTuplesWithLowestAmongHighestMembershipDegrees.size() - nbOfAdditionalTuples);
	      do
		{
		  addRemappedDefaultPattern(*additionalTupleIt);
		}
	      while (++additionalTupleIt != additionalTupleEnd);
	      allPatternsAdded();
	      additionalTuplesWithLowestAmongHighestMembershipDegrees.clear();
	      additionalTuplesWithLowestAmongHighestMembershipDegrees.shrink_to_fit();
	    }
	  tuplesWithHighestMembershipDegrees.clear();
	  tuplesWithHighestMembershipDegrees.shrink_to_fit();
	}
      old2NewDimensionOrder.clear();
      old2NewDimensionOrder.shrink_to_fit();
      oldIds2NewIds.clear();
      oldIds2NewIds.shrink_to_fit();
      return false;
    }
  return true;
}

void ConcurrentPatternPool::addDefaultPattern(const vector<unsigned int>& tuple)
{
  vector<vector<unsigned int>> pattern;
  const vector<unsigned int>::const_iterator idEnd = tuple.end();
  vector<unsigned int>::const_iterator idIt = tuple.begin();
  pattern.reserve(idEnd - idIt);
  pattern.emplace_back(1, *idIt);
  ++idIt;
  do
    {
      pattern.emplace_back(1, *idIt);
    }
  while (++idIt != idEnd);
  addPattern(pattern);
}


void ConcurrentPatternPool::addRemappedDefaultPattern(const vector<unsigned int>& tuple)
{
  const unsigned int n = tuple.size();
  vector<vector<unsigned int>> pattern(n);
  pattern[old2NewDimensionOrder.front()].emplace_back(oldIds2NewIds.front()[tuple.front()]);
  unsigned int oldDimensionId = 1;
  do
    {
      pattern[old2NewDimensionOrder[oldDimensionId]].emplace_back(oldIds2NewIds[oldDimensionId][tuple[oldDimensionId]]);
    }
  while (++oldDimensionId != n);
  addPattern(pattern);
}

void ConcurrentPatternPool::addPattern(vector<vector<unsigned int>>& pattern)
{
  patternsLock.lock();
  patterns.emplace_back(std::move(pattern));
  patternsLock.unlock();
  cv.notify_one();
}

void ConcurrentPatternPool::addFuzzyTuple(const vector<unsigned int>& tuple, const double shiftedMembership)
{
  if (isDefaultInitialPatterns && shiftedMembership > 0)
    {
      if (isUnboundedNumberOfPatterns)
	{
	  additionalTuplesWithLowestAmongHighestMembershipDegrees.emplace_back(tuple);
	  return;
	}
      if (nbOfFreeSlots)
	{
	  tuplesWithHighestMembershipDegrees.emplace_back(make_pair(tuple, shiftedMembership));
	  if (!--nbOfFreeSlots)
	    {
	      make_heap(tuplesWithHighestMembershipDegrees.begin(), tuplesWithHighestMembershipDegrees.end(), [](const pair<vector<unsigned int>, double>& fuzzyTuple1, const pair<vector<unsigned int>, double>& fuzzyTuple2) { return fuzzyTuple1.second > fuzzyTuple2.second; });
	      pop_heap(tuplesWithHighestMembershipDegrees.begin(), tuplesWithHighestMembershipDegrees.end(), [](const pair<vector<unsigned int>, double>& fuzzyTuple1, const pair<vector<unsigned int>, double>& fuzzyTuple2) { return fuzzyTuple1.second > fuzzyTuple2.second; });
	    }
	  return;
	}
      const double lowestAmongHighestMembershipDegrees = tuplesWithHighestMembershipDegrees.back().second;
      if (lowestAmongHighestMembershipDegrees < shiftedMembership)
	{
	  tuplesWithHighestMembershipDegrees.pop_back();
	  tuplesWithHighestMembershipDegrees.emplace_back(make_pair(tuple, shiftedMembership));
	  pop_heap(tuplesWithHighestMembershipDegrees.begin(), tuplesWithHighestMembershipDegrees.end(), [](const pair<vector<unsigned int>, double>& fuzzyTuple1, const pair<vector<unsigned int>, double>& fuzzyTuple2) { return fuzzyTuple1.second > fuzzyTuple2.second; });
	  if (tuplesWithHighestMembershipDegrees.back().second != lowestAmongHighestMembershipDegrees)
	    {
	      additionalTuplesWithLowestAmongHighestMembershipDegrees.clear();
	    }
	  return;
	}
      if (lowestAmongHighestMembershipDegrees == shiftedMembership)
	{
	  additionalTuplesWithLowestAmongHighestMembershipDegrees.emplace_back(tuple);
	}
    }
}

void ConcurrentPatternPool::allPatternsAdded()
{
  isAllPatternsAdded = true;
  cv.notify_one();
}

void ConcurrentPatternPool::setNewDimensionOrderAndNewIds(const vector<unsigned int>& old2NewDimensionOrderParam, const vector<vector<pair<double, unsigned int>>>& elementPositiveMemberships)
{
  old2NewDimensionOrder = old2NewDimensionOrderParam;
  oldIds2NewIds.reserve(old2NewDimensionOrder.size());
  vector<vector<pair<double, unsigned int>>>::const_iterator elementPositiveMembershipsInDimensionIt = elementPositiveMemberships.begin();
  oldIds2NewIds.emplace_back(oldIds2NewIdsInDimension(*elementPositiveMembershipsInDimensionIt));
  ++elementPositiveMembershipsInDimensionIt;
  const vector<vector<pair<double, unsigned int>>>::const_iterator elementPositiveMembershipsInDimensionEnd = elementPositiveMemberships.end();
  do
    {
      oldIds2NewIds.emplace_back(oldIds2NewIdsInDimension(*elementPositiveMembershipsInDimensionIt));
    }
  while (++elementPositiveMembershipsInDimensionIt != elementPositiveMembershipsInDimensionEnd);
}

vector<vector<unsigned int>> ConcurrentPatternPool::next()
{
  if (isAllPatternsAdded)
    {
      const lock_guard<mutex> lock(patternsLock);
      if (patterns.empty())
	{
	  return {};
	}
      const vector<vector<unsigned int>> pattern = std::move(patterns.back());
      patterns.pop_back();
      return pattern;
    }
  unique_lock<mutex> lock(patternsLock);
  if (patterns.empty())
    {
      cv.wait(lock, []{ return !patterns.empty() || isAllPatternsAdded; });
      if (patterns.empty())
	{
	  lock.unlock();
	  cv.notify_one();
	  return {};
	}
    }
  const vector<vector<unsigned int>> pattern = std::move(patterns.back());
  patterns.pop_back();
  lock.unlock();
  cv.notify_one();
  return pattern;
}

void ConcurrentPatternPool::printProgressionOnSTDIN(const float stepInSeconds)
{
  cout << "\rGetting initial patterns: done.           \n";
  unsigned int nbOfDigitsInNbOfPatterns = patterns.size();
  if (nbOfDigitsInNbOfPatterns)
    {
      nbOfDigitsInNbOfPatterns = log10(nbOfDigitsInNbOfPatterns);
      ++nbOfDigitsInNbOfPatterns;
      const chrono::duration<float> duration(stepInSeconds);
      // I believe locking patternsLock is unnecessary
      while (!patterns.empty())
	{
	  cout << "\rStill " << right << setw(nbOfDigitsInNbOfPatterns) << patterns.size() << " patterns to start modifying" << flush;
	  this_thread::sleep_for(duration);
	}
    }
}

vector<unsigned int> ConcurrentPatternPool::oldIds2NewIdsInDimension(const vector<pair<double, unsigned int>>& elements)
{
  const unsigned int nbOfElements = elements.size();
  vector<unsigned int> oldIds2NewIdsInDimension(nbOfElements);
  for (unsigned int newElementId = 0; newElementId != nbOfElements; ++newElementId)
    {
      oldIds2NewIdsInDimension[elements[newElementId].second] = newElementId;
    }
  return oldIds2NewIdsInDimension;
}
