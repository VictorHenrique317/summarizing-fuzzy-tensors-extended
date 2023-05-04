// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef VISITED_PATTERNS_H_
#define VISITED_PATTERNS_H_

#include "../../Parameters.h"

#if REMEMBER != 0
#include <vector>
#include <mutex>

#if REMEMBER == 1
#include <unordered_set>
#include <boost/container_hash/hash.hpp>
#endif

using namespace std;

class VisitedPatterns
{
public:
  virtual ~VisitedPatterns();

  static void init(const vector<unsigned int>& cardinalities);
  static bool visited(const vector<vector<unsigned int>>& nSet);
  static void clear();

#if REMEMBER == 2
  virtual bool isPresent() const;
  virtual VisitedPatterns* makePresent();
  virtual pair<VisitedPatterns*, bool> visited(const vector<unsigned int>::const_iterator flatNSetBegin, const vector<unsigned int>::const_iterator flatNSetEnd);
  virtual void deepDelete();

protected:
  static VisitedPatterns presentWithNoExtension; // only object of this class
#endif

private:
  static vector<unsigned int> tupleOffsets;
  static vector<unsigned int> elementOffsets;
#if REMEMBER == 1
  static vector<pair<mutex, unordered_set<vector<unsigned int>, boost::hash<vector<unsigned int>>>>> firstTuples;
#else
  static vector<pair<mutex, VisitedPatterns*>> firstTuples;
#endif
};
#endif

#endif /*VISITED_PATTERNS_H_*/
