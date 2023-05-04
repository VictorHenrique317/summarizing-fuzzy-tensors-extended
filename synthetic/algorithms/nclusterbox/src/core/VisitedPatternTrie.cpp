// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#if REMEMBER == 2
#include "PresentVisitedPatternLeaf.h"
#include "PresentVisitedPatternTrie.h"

#include <algorithm>

VisitedPatternTrie::VisitedPatternTrie(vector<pair<unsigned int, VisitedPatterns*>>& suffixesParam) : suffixes(std::move(suffixesParam))
{
}

VisitedPatternTrie::VisitedPatternTrie(const vector<unsigned int>::const_iterator prefixBegin, const vector<unsigned int>::const_iterator prefixEnd, const vector<unsigned int>::const_iterator suffixEnd) : suffixes()
{
  if (prefixBegin != prefixEnd)
    {
      suffixes.emplace_back(*prefixBegin, new VisitedPatternTrie(prefixBegin + 1, prefixEnd, suffixEnd));
      return;
    }
  suffixes.emplace_back(*prefixEnd, new PresentVisitedPatternLeaf(prefixEnd + 1, suffixEnd));
}

VisitedPatternTrie::VisitedPatternTrie(const vector<unsigned int>::const_iterator prefixBegin, const vector<unsigned int>::const_iterator prefixEnd, const vector<unsigned int>::const_iterator firstSuffixEnd, const vector<unsigned int>::const_iterator secondSuffixBegin, const vector<unsigned int>::const_iterator secondSuffixEnd) : suffixes()
{
  if (prefixBegin != prefixEnd)
    {
      suffixes.emplace_back(*prefixBegin, new VisitedPatternTrie(prefixBegin + 1, prefixEnd, firstSuffixEnd, secondSuffixBegin, secondSuffixEnd));
      return;
    }
  suffixes.reserve(2);
  if (prefixEnd + 1 == firstSuffixEnd)
    {
      suffixes.emplace_back(*prefixEnd, &presentWithNoExtension);
    }
  else
    {
      suffixes.emplace_back(*prefixEnd, new VisitedPatternLeaf(prefixEnd + 1, firstSuffixEnd));
    }
  if (secondSuffixBegin + 1 == secondSuffixEnd)
    {
      suffixes.emplace_back(*secondSuffixBegin, &presentWithNoExtension);
      return;
    }
  suffixes.emplace_back(*secondSuffixBegin, new VisitedPatternLeaf(secondSuffixBegin + 1, secondSuffixEnd));
}

VisitedPatternTrie::~VisitedPatternTrie()
{
}

bool VisitedPatternTrie::isPresent() const
{
  return false;
}

VisitedPatternTrie* VisitedPatternTrie::makePresent()
{
  return new PresentVisitedPatternTrie(suffixes);
}

pair<VisitedPatterns*, bool> VisitedPatternTrie::visited(const vector<unsigned int>::const_iterator flatNSetIt, const vector<unsigned int>::const_iterator flatNSetEnd)
{
  const vector<pair<unsigned int, VisitedPatterns*>>::iterator nextIt = lower_bound(suffixes.begin(), suffixes.end(), *flatNSetIt, [](const pair<unsigned int, VisitedPatterns*>& entry, const unsigned int id) {return entry.first < id;});
  if (nextIt == suffixes.end())
    {
      // Emplaced n-set has not been visited and is to be inserted at the end of suffixes
      if (flatNSetIt + 1 == flatNSetEnd)
	{
	  suffixes.emplace_back(*flatNSetIt, &presentWithNoExtension);
	  return {this, false};
	}
      suffixes.emplace_back(*flatNSetIt, new VisitedPatternLeaf(flatNSetIt + 1, flatNSetEnd));
      return {this, false};
    }
  if (nextIt->first == *flatNSetIt)
    {
      // Next element matches
      if (flatNSetIt + 1 == flatNSetEnd)
	{
	  // End of flat n-set
	  if (nextIt->second->isPresent())
	    {
	      // Emplaced n-set was visited
	      return {this, true};
	    }
	  // Emplaced n-set was not visited
	  VisitedPatterns* presentNode = nextIt->second->makePresent();
	  delete nextIt->second;
	  nextIt->second = presentNode;
	  return {this, false};
	}
      // Keep searching
      const pair<VisitedPatterns*, bool> replacementAndIsPresent = nextIt->second->visited(flatNSetIt + 1, flatNSetEnd);
      if (replacementAndIsPresent.first != nextIt->second)
	{
	  if (nextIt->second != &presentWithNoExtension)
	    {
	      delete nextIt->second;
	    }
	  nextIt->second = replacementAndIsPresent.first;
	}
      return {this, replacementAndIsPresent.second};
    }
  // Next element mismatches
  if (flatNSetIt + 1 == flatNSetEnd)
    {
      suffixes.insert(nextIt, pair<unsigned int, VisitedPatterns*>(*flatNSetIt, &presentWithNoExtension));
      return {this, false};
    }
  suffixes.insert(nextIt, pair<unsigned int, VisitedPatternLeaf*>(*flatNSetIt, new VisitedPatternLeaf(flatNSetIt + 1, flatNSetEnd)));
  return {this, false};
}

void VisitedPatternTrie::deepDelete()
{
  for (pair<unsigned int, VisitedPatterns*>& s : suffixes)
    {
      if (s.second != &presentWithNoExtension)
	{
	  s.second->deepDelete();
	  delete s.second;
	}
    }
}
#endif
