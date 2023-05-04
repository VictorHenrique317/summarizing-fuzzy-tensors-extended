// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef VISITED_PATTERN_TRIE_H_
#define VISITED_PATTERN_TRIE_H_

#if REMEMBER == 2
#include "VisitedPatterns.h"

class VisitedPatternTrie : public VisitedPatterns /* the pattern ending here is absent */
{
public:
  VisitedPatternTrie(vector<pair<unsigned int, VisitedPatterns*>>& suffixes);
  VisitedPatternTrie(const vector<unsigned int>::const_iterator prefixBegin, const vector<unsigned int>::const_iterator prefixEnd, const vector<unsigned int>::const_iterator suffixEnd); // one PresentVisitedPatternTrie before the VisitedPatternLeaf
  VisitedPatternTrie(const vector<unsigned int>::const_iterator prefixBegin, const vector<unsigned int>::const_iterator prefixEnd, const vector<unsigned int>::const_iterator firstSuffixEnd, const vector<unsigned int>::const_iterator secondSuffixBegin, const vector<unsigned int>::const_iterator secondSuffixEnd); // only VisitedPatternTrie before two VisitedPatternLeaf, in the given order (i.e., *prefixEnd < *secondSuffixBegin)

  virtual ~VisitedPatternTrie();

  bool isPresent() const;

  VisitedPatternTrie* makePresent();
  pair<VisitedPatterns*, bool> visited(const vector<unsigned int>::const_iterator flatNSetIt, const vector<unsigned int>::const_iterator flatNSetEnd);
  void deepDelete();

private:
  vector<pair<unsigned int, VisitedPatterns*>> suffixes; // ordered by next element
};
#endif

#endif /*VISITED_PATTERN_TRIE_H_*/
