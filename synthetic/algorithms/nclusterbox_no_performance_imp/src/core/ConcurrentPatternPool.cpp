// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "ConcurrentPatternPool.h"

#include <algorithm>
#include <iostream>
#include <iomanip>
#include <thread>

#include "AbstractRoughTensor.h"
#include "BestPatterns.h"

vector<vector<vector<unsigned int>>> ConcurrentPatternPool::patterns;
mutex ConcurrentPatternPool::patternsLock;
condition_variable ConcurrentPatternPool::cv;
bool ConcurrentPatternPool::isDefaultInitialPatterns;
bool ConcurrentPatternPool::isAllPatternsAdded;
vector<unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>> ConcurrentPatternPool::tuples;
vector<unsigned int> ConcurrentPatternPool::old2NewDimensionOrder;
vector<vector<unsigned int>> ConcurrentPatternPool::oldIds2NewIds;

void ConcurrentPatternPool::setReadFromFile()
{
  isDefaultInitialPatterns = false;
  isAllPatternsAdded = false;
}

void ConcurrentPatternPool::setDefaultPatterns()
{
  isDefaultInitialPatterns = true;
  isAllPatternsAdded = false;
}

void ConcurrentPatternPool::setNbOfDimensions(const unsigned int nbOfDimensions)
{
  tuples.resize(nbOfDimensions);
}

bool ConcurrentPatternPool::readFromFile(const unsigned int maxNbOfInitialPatterns)
{
  if (isDefaultInitialPatterns)
    {
      // Compute how many initial patterns with no max
      vector<unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>>::const_iterator tuplesIt = tuples.begin();
      long long nbOfInitialPatterns = tuplesIt->size();
      ++tuplesIt;
      const vector<unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>>::const_iterator tuplesEnd = tuples.end();
      do
	{
	  nbOfInitialPatterns += tuplesIt->size();
	}
      while (++tuplesIt != tuplesEnd);
      patterns.reserve(nbOfInitialPatterns);
      const unsigned int n = tuples.size();
      unsigned int freeDimensionId = 0;
      if (maxNbOfInitialPatterns && nbOfInitialPatterns > maxNbOfInitialPatterns)
	{
	  BestPatterns::setMaxNbOfPatterns(maxNbOfInitialPatterns);
	  if (old2NewDimensionOrder.empty())
	    {
	      do
		{
		  unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>& fibers = tuples[freeDimensionId];
		  const unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>::iterator fiberEnd = fibers.end();
		  unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>::iterator fiberIt = fibers.begin();
		  do
		    {
		      BestPatterns::push(subFiberMaximizingG(fiberIt->first, fiberIt->second, freeDimensionId));
		      fiberIt = fibers.erase(fiberIt);
		    }
		  while (fiberIt != fiberEnd);
		}
	      while (++freeDimensionId != n);
	    }
	  else
	    {
	      do
		{
		  unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>& fibers = tuples[freeDimensionId];
		  const unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>::iterator fiberEnd = fibers.end();
		  unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>::iterator fiberIt = fibers.begin();
		  do
		    {
		      BestPatterns::push(remappedSubFiberMaximizingG(fiberIt->first, fiberIt->second, freeDimensionId));
		      fiberIt = fibers.erase(fiberIt);
		    }
		  while (fiberIt != fiberEnd);
		}
	      while (++freeDimensionId != n);
	    }
	  vector<pair<vector<vector<unsigned int>>, double>>& patternsWithG = BestPatterns::get();
	  const vector<pair<vector<vector<unsigned int>>, double>>::iterator patternWithGEnd = patternsWithG.end();
	  vector<pair<vector<vector<unsigned int>>, double>>::iterator patternWithGIt = patternsWithG.begin();
	  do
	    {
	      addPattern(patternWithGIt->first);
	    }
	  while (++patternWithGIt != patternWithGEnd);
	  patternsWithG.clear();
	  patternsWithG.shrink_to_fit();
	}
      else
	{
	  if (old2NewDimensionOrder.empty())
	    {
	      do
		{
		  unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>& fibers = tuples[freeDimensionId];
		  const unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>::iterator fiberEnd = fibers.end();
		  unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>::iterator fiberIt = fibers.begin();
		  do
		    {
		      vector<vector<unsigned int>> pattern = subFiberMaximizingG(fiberIt->first, fiberIt->second, freeDimensionId).first;
		      addPattern(pattern);
		      fiberIt = fibers.erase(fiberIt);
		    }
		  while (fiberIt != fiberEnd);
		}
	      while (++freeDimensionId != n);
	    }
	  else
	    {
	      do
		{
		  unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>& fibers = tuples[freeDimensionId];
		  const unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>::iterator fiberEnd = fibers.end();
		  unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>::iterator fiberIt = fibers.begin();
		  do
		    {
		      vector<vector<unsigned int>> pattern = remappedSubFiberMaximizingG(fiberIt->first, fiberIt->second, freeDimensionId).first;
		      addPattern(pattern);
		      fiberIt = fibers.erase(fiberIt);
		    }
		  while (fiberIt != fiberEnd);
		}
	      while (++freeDimensionId != n);
	    }
	}
      allPatternsAdded();
      tuples.clear();
      tuples.shrink_to_fit();
      if (!old2NewDimensionOrder.empty())
	{
	  old2NewDimensionOrder.clear();
	  old2NewDimensionOrder.shrink_to_fit();
	  oldIds2NewIds.clear();
	  oldIds2NewIds.shrink_to_fit();
	}
      return false;
    }
  return true;
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
      vector<unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>>::iterator tuplesIt = tuples.begin();
      vector<unsigned int>::const_iterator elementInFreeDimensionIt = tuple.begin();
      vector<unsigned int> key(elementInFreeDimensionIt + 1, tuple.end());
      unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>::iterator insertIt = tuplesIt->find(key);
      if (insertIt == tuplesIt->end())
	{
	  (*tuplesIt)[key] = {pair<unsigned int, double>(*elementInFreeDimensionIt, shiftedMembership)};
	}
      else
	{
	  insertIt->second.push_back(pair<unsigned int, double>(*elementInFreeDimensionIt, shiftedMembership));
	}
      const vector<unsigned int>::iterator keyEnd = key.end();
      vector<unsigned int>::iterator keyIt = key.begin();
      do
	{
	  *keyIt++ = *elementInFreeDimensionIt;
	  insertIt = (++tuplesIt)->find(key);
	  if (insertIt == tuplesIt->end())
	    {
	      (*tuplesIt)[key] = {pair<unsigned int, double>(*++elementInFreeDimensionIt, shiftedMembership)};
	    }
	  else
	    {
	      insertIt->second.push_back(pair<unsigned int, double>(*++elementInFreeDimensionIt, shiftedMembership));
	    }
	}
      while (keyIt != keyEnd);
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

pair<vector<vector<unsigned int>>, double> ConcurrentPatternPool::subFiberMaximizingG(const vector<unsigned int>& constrainedDimensions, vector<pair<unsigned int, double>>& freeDimension, const unsigned int freeDimensionId)
{
  unsigned int n = tuples.size();
  vector<vector<unsigned int>> pattern;
  pattern.reserve(n);
  // Single elements before the dimension with possibly several elements
  for (unsigned int dimensionId = 0; dimensionId != freeDimensionId; ++dimensionId)
    {
      pattern.emplace_back(1, constrainedDimensions[dimensionId]);
    }
  // Elements in the dimension with possibly several elements
  // Compute the subset with the maximal product of the size and the square of the density
  const vector<pair<unsigned int, double>>::iterator fiberEnd = freeDimension.end();
  vector<pair<unsigned int, double>>::iterator fiberIt = freeDimension.begin();
  sort(fiberIt, fiberEnd, [](const pair<unsigned int, double>& elementAndMembershipDegree1, const pair<unsigned int, double>& elementAndMembershipDegree2) { return elementAndMembershipDegree1.second > elementAndMembershipDegree2.second; });
  pattern.emplace_back(1, fiberIt->first);
  unsigned int area = 1;
  double sum = fiberIt->second;
  while (++fiberIt != fiberEnd && (sum + fiberIt->second) * (sum + fiberIt->second) * area > sum * sum * (area + 1))
    {
      ++area;
      sum += fiberIt->second;
      pattern.back().push_back(fiberIt->first);
    }
  sort(pattern.back().begin(), pattern.back().end());
  // Single elements after the dimension with possibly several elements
  --n;
  for (unsigned int dimensionId = freeDimensionId; dimensionId != n; ++dimensionId)
    {
      pattern.emplace_back(1, constrainedDimensions[dimensionId]);
    }
  return {pattern, sum * sum / area};
}

pair<vector<vector<unsigned int>>, double> ConcurrentPatternPool::remappedSubFiberMaximizingG(const vector<unsigned int>& constrainedDimensions, vector<pair<unsigned int, double>>& freeDimension, const unsigned int freeDimensionId)
{
  const unsigned int n = tuples.size();
  vector<vector<unsigned int>> pattern(n);
  vector<vector<unsigned int>>::const_iterator oldIds2NewIdsInDimensionIt = oldIds2NewIds.begin();
  // Single elements before the dimension with possibly several elements
  for (unsigned int oldDimensionId = 0; oldDimensionId != freeDimensionId; ++oldDimensionId)
    {
      pattern[old2NewDimensionOrder[oldDimensionId]].push_back((*oldIds2NewIdsInDimensionIt)[constrainedDimensions[oldDimensionId]]);
      ++oldIds2NewIdsInDimensionIt;
    }
  // Elements in the dimension with possibly several elements
  // Compute the subset with the maximal product of the size and the square of the density
  const vector<pair<unsigned int, double>>::iterator fiberEnd = freeDimension.end();
  vector<pair<unsigned int, double>>::iterator fiberIt = freeDimension.begin();
  sort(fiberIt, fiberEnd, [](const pair<unsigned int, double>& elementAndMembershipDegree1, const pair<unsigned int, double>& elementAndMembershipDegree2) { return elementAndMembershipDegree1.second > elementAndMembershipDegree2.second; });
  vector<unsigned int>& dimension = pattern[old2NewDimensionOrder[freeDimensionId]];
  dimension.push_back((*oldIds2NewIdsInDimensionIt)[fiberIt->first]);
  unsigned int area = 1;
  double sum = fiberIt->second;
  while (++fiberIt != fiberEnd && (sum + fiberIt->second) * (sum + fiberIt->second) * area > sum * sum * (area + 1))
    {
      ++area;
      sum += fiberIt->second;
      dimension.push_back((*oldIds2NewIdsInDimensionIt)[fiberIt->first]);
    }
  sort(dimension.begin(), dimension.end());
  // Single elements after the dimension with possibly several elements
  ++oldIds2NewIdsInDimensionIt;
  for (unsigned int oldDimensionId = freeDimensionId + 1; oldDimensionId != n; ++oldDimensionId)
    {
      pattern[old2NewDimensionOrder[oldDimensionId]].push_back((*oldIds2NewIdsInDimensionIt)[constrainedDimensions[oldDimensionId - 1]]);
      ++oldIds2NewIdsInDimensionIt;
    }
  return {pattern, sum * sum / area};
}
