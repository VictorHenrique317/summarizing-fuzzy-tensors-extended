// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef PRESENT_VISITED_PATTERN_TRIE_H_
#define PRESENT_VISITED_PATTERN_TRIE_H_

#include "VisitedPatternTrie.h"

#if REMEMBER == 2
class PresentVisitedPatternTrie final : public VisitedPatternTrie
{
public:
  PresentVisitedPatternTrie(vector<pair<unsigned int, VisitedPatterns*>>& suffixes);
  PresentVisitedPatternTrie(const vector<unsigned int>::const_iterator prefixBegin, const vector<unsigned int>::const_iterator prefixEnd, const vector<unsigned int>::const_iterator suffixEnd); // one PresentVisitedPatternTrie before the VisitedPatternLeaf
  PresentVisitedPatternTrie(const vector<unsigned int>::const_iterator prefixBegin, const vector<unsigned int>::const_iterator prefixEnd, const vector<unsigned int>::const_iterator firstSuffixEnd, const vector<unsigned int>::const_iterator secondSuffixBegin, const vector<unsigned int>::const_iterator secondSuffixEnd); // only VisitedPatternTrie before two VisitedPatternLeaf, in the given order (i.e., *prefixEnd < *secondSuffixBegin)

  ~PresentVisitedPatternTrie();

  bool isPresent() const;
};
#endif

#endif /*PRESENT_VISITED_PATTERN_TRIE_H_*/
