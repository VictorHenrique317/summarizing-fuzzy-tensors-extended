// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "PresentVisitedPatternLeaf.h"

#if REMEMBER != 0
vector<unsigned int> VisitedPatterns::tupleOffsets;
vector<unsigned int> VisitedPatterns::elementOffsets;

VisitedPatterns::~VisitedPatterns()
{
}

#if REMEMBER == 1
vector<pair<mutex, unordered_set<vector<unsigned int>, boost::hash<vector<unsigned int>>>>> VisitedPatterns::firstTuples;
#else
VisitedPatterns VisitedPatterns::presentWithNoExtension;

vector<pair<mutex, VisitedPatterns*>> VisitedPatterns::firstTuples;

bool VisitedPatterns::isPresent() const
{
  return true;
}

VisitedPatterns* VisitedPatterns::makePresent()
{
  // Never called
  return nullptr;
}

pair<VisitedPatterns*, bool> VisitedPatterns::visited(const vector<unsigned int>::const_iterator flatNSetBegin, const vector<unsigned int>::const_iterator flatNSetEnd)
{
  return {new PresentVisitedPatternLeaf(flatNSetBegin, flatNSetEnd), false};
}

void VisitedPatterns::deepDelete()
{
}
#endif

void VisitedPatterns::init(const vector<unsigned int>& cardinalities)
{
  const vector<unsigned int>::const_iterator lastCardinalityIt = --cardinalities.end();
  vector<unsigned int>::const_iterator cardinalityIt = cardinalities.begin();
  tupleOffsets.reserve(distance(cardinalityIt, lastCardinalityIt));
  elementOffsets.reserve(distance(cardinalityIt, lastCardinalityIt));
  tupleOffsets.push_back(*cardinalityIt);
  elementOffsets.push_back(*cardinalityIt);
  while (++cardinalityIt != lastCardinalityIt)
    {
      tupleOffsets.push_back(*cardinalityIt * tupleOffsets.back());
      elementOffsets.push_back(*cardinalityIt + elementOffsets.back());
    }
#if REMEMBER == 1
  firstTuples = vector<pair<mutex, unordered_set<vector<unsigned int>, boost::hash<vector<unsigned int>>>>>(tupleOffsets.back());
#else
  firstTuples = vector<pair<mutex, VisitedPatterns*>>(tupleOffsets.back());
#endif
}

bool VisitedPatterns::visited(const vector<vector<unsigned int>>& nSet)
{
  vector<vector<unsigned int>>::const_iterator nSetEnd = nSet.end();
  vector<vector<unsigned int>>::const_iterator nSetIt = nSet.begin();
  unsigned int nbOfElementsInNSet = nSetIt->size();
  ++nSetIt;
  do
    {
      nbOfElementsInNSet += nSetIt->size();
    }
  while (++nSetIt != nSetEnd);
  nSetIt = nSet.begin();
  vector<unsigned int> flatNSet;
  flatNSet.reserve(nbOfElementsInNSet - distance(nSetIt, --nSetEnd));
  vector<unsigned int>::const_iterator elementIt = nSetIt->begin();
  unsigned int mostSurprisingTupleId = *elementIt;
  flatNSet.insert(flatNSet.end(), ++elementIt, nSetIt->end());
  vector<unsigned int>::const_iterator elementOffsetIt = elementOffsets.begin();
  if (++nSetIt != nSetEnd)
    {
      vector<unsigned int>::const_iterator tupleOffsetIt = tupleOffsets.begin();
      for (; ; )
	{
	  elementIt = nSetIt->begin();
	  mostSurprisingTupleId += *elementIt * *tupleOffsetIt;
	  for (const vector<unsigned int>::const_iterator elementEnd = nSetIt->end(); ++elementIt != elementEnd; )
	    {
	      flatNSet.push_back(*elementIt + *elementOffsetIt);
	    }
	  if (++nSetIt == nSetEnd)
	    {
	      break;
	    }
	  ++elementOffsetIt;
	  ++tupleOffsetIt;
	}
    }
  elementIt = nSetIt->begin();
  const vector<unsigned int>::const_iterator elementEnd = nSetIt->end();
  do
    {
      flatNSet.push_back(*elementIt + *elementOffsetIt);
    }
  while (++elementIt != elementEnd);
#if REMEMBER == 1
  pair<mutex, unordered_set<vector<unsigned int>, boost::hash<vector<unsigned int>>>>& firstTuple = firstTuples[mostSurprisingTupleId];
  firstTuple.first.lock();
  if (firstTuple.second.emplace(flatNSet).second)
    {
      firstTuple.first.unlock();
      return false;
    }
  firstTuple.first.unlock();
  return true;
#else
  pair<mutex, VisitedPatterns*>& firstTuple = firstTuples[mostSurprisingTupleId];
  firstTuple.first.lock();
  if (firstTuple.second)
    {
      // Some already visited n-set covered firstTuple
      const pair<VisitedPatterns*, bool> replacementAndIsPresent = firstTuple.second->visited(flatNSet.begin(), flatNSet.end());
      if (replacementAndIsPresent.first != firstTuple.second)
	{
	  delete firstTuple.second;
	  firstTuple.second = replacementAndIsPresent.first;
	}
      firstTuple.first.unlock();
      return replacementAndIsPresent.second;
    }
  // No n-set so far covered firstTuple
  firstTuple.second = new VisitedPatternLeaf(flatNSet.begin(), flatNSet.end());
  firstTuple.first.unlock();
  return false;
#endif
}

void VisitedPatterns::clear()
{
#if REMEMBER == 2
  for (pair<mutex, VisitedPatterns*>& firstTuple : firstTuples)
    {
      if (firstTuple.second)
	{
	  firstTuple.second->deepDelete();
	  delete firstTuple.second;
	}
    }
#endif
  firstTuples.clear();
  firstTuples.shrink_to_fit();
}
#endif
