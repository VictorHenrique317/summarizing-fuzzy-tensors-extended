// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef PRESENT_VISITED_PATTERN_LEAF_H_
#define PRESENT_VISITED_PATTERN_LEAF_H_

#include "VisitedPatternLeaf.h"

#if REMEMBER == 2
class PresentVisitedPatternLeaf final : public VisitedPatternLeaf
{
public:
  PresentVisitedPatternLeaf(vector<unsigned int>& suffix);
  PresentVisitedPatternLeaf(const vector<unsigned int>::const_iterator suffixBegin, const vector<unsigned int>::const_iterator suffixEnd);

  ~PresentVisitedPatternLeaf();

  bool isPresent() const;

  pair<VisitedPatterns*, bool> emplace(const vector<unsigned int>::const_iterator flatNSetIt, const vector<unsigned int>::const_iterator flatNSetEnd);
};
#endif

#endif /*PRESENT_VISITED_PATTERN_LEAF_H_*/
