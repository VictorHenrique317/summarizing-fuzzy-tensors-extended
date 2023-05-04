// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "PresentVisitedPatternLeaf.h"
#include "VisitedPatternTrie.h"

#if REMEMBER == 2
VisitedPatternLeaf::VisitedPatternLeaf(vector<unsigned int>& suffixParam) : suffix(std::move(suffixParam))
{
}

VisitedPatternLeaf::VisitedPatternLeaf(const vector<unsigned int>::const_iterator suffixBegin, const vector<unsigned int>::const_iterator suffixEnd) : suffix(suffixBegin, suffixEnd)
{
}

VisitedPatternLeaf::~VisitedPatternLeaf()
{
}

bool VisitedPatternLeaf::isPresent() const
{
  return false;
}

VisitedPatternLeaf* VisitedPatternLeaf::makePresent()
{
  return new PresentVisitedPatternLeaf(suffix);
}

pair<VisitedPatterns*, bool> VisitedPatternLeaf::visited(const vector<unsigned int>::const_iterator flatNSetIt, const vector<unsigned int>::const_iterator flatNSetEnd)
{
  const pair<vector<unsigned int>::const_iterator, vector<unsigned int>::const_iterator> firstMismatch = mismatch(flatNSetIt, flatNSetEnd, suffix.begin(), suffix.end());
  if (firstMismatch.first == flatNSetEnd)
    {
      if (firstMismatch.second == suffix.end())
	{
	  // Already visited
	  return {this, true};
	}
      // Emplaced n-set is a prefix of this
      return {new VisitedPatternTrie(suffix.begin(), firstMismatch.second, suffix.end()), false};
    }
  // Emplaced n-set is different and not a prefix
  if (firstMismatch.second == suffix.end())
    {
      // this is a prefix of the emplaced n-set
      return {new VisitedPatternTrie(flatNSetIt, firstMismatch.first, flatNSetEnd), false};
    }
  if (*firstMismatch.second < *firstMismatch.first)
    {
      return {new VisitedPatternTrie(suffix.begin(), firstMismatch.second, suffix.end(), firstMismatch.first, flatNSetEnd), false};
    }
  return {new VisitedPatternTrie(flatNSetIt, firstMismatch.first, flatNSetEnd, firstMismatch.second, suffix.end()), false};
}
#endif
