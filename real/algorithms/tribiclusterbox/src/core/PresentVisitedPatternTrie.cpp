// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "PresentVisitedPatternTrie.h"

#if REMEMBER == 2
PresentVisitedPatternTrie::PresentVisitedPatternTrie(vector<pair<unsigned int, VisitedPatterns*>>& suffixesParam) : VisitedPatternTrie(suffixesParam)
{
}

PresentVisitedPatternTrie::PresentVisitedPatternTrie(const vector<unsigned int>::const_iterator prefixBegin, const vector<unsigned int>::const_iterator prefixEnd, const vector<unsigned int>::const_iterator suffixEnd) : VisitedPatternTrie(prefixBegin, prefixEnd, suffixEnd)
{
}

PresentVisitedPatternTrie::PresentVisitedPatternTrie(const vector<unsigned int>::const_iterator prefixBegin, const vector<unsigned int>::const_iterator prefixEnd, const vector<unsigned int>::const_iterator firstSuffixEnd, const vector<unsigned int>::const_iterator secondSuffixBegin, const vector<unsigned int>::const_iterator secondSuffixEnd) : VisitedPatternTrie(prefixBegin, prefixEnd, firstSuffixEnd, secondSuffixBegin, secondSuffixEnd)
{
}

PresentVisitedPatternTrie::~PresentVisitedPatternTrie()
{
}

bool PresentVisitedPatternTrie::isPresent() const
{
  return true;
}
#endif
