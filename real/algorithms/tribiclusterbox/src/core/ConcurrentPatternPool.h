// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef CONCURRENT_PATTERN_POOL_H_
#define CONCURRENT_PATTERN_POOL_H_

#include <vector>
#include <mutex>
#include <condition_variable>

using namespace std;

class ConcurrentPatternPool
{
 public:
  static void setReadFromFile();
  static void setDefaultPatterns(const unsigned long long maxNbOfPatterns);
  static bool readFromFile();

  static void addPattern(vector<vector<unsigned int>>& pattern);
  static void addFuzzyTuple(const vector<unsigned int>& tuple, const double shiftedMembership);
  static void setNewDimensionOrderAndNewIds(const vector<unsigned int>& old2NewDimensionOrder, const vector<vector<pair<double, unsigned int>>>& elementPositiveMemberships);
  static void allPatternsAdded();
  static vector<vector<unsigned int>> next();
  static void moveTo(vector<vector<vector<unsigned int>>>& candidateVariables);

  static void printProgressionOnSTDIN(const float stepInSeconds);

 private:
  static vector<vector<vector<unsigned int>>> patterns;
  static mutex patternsLock;
  static condition_variable cv;
  static bool isDefaultInitialPatterns;
  static bool isUnboundedNumberOfPatterns;
  static bool isAllPatternsAdded;
  static unsigned long long nbOfFreeSlots;
  static vector<pair<vector<unsigned int>, double>> tuplesWithHighestMembershipDegrees;
  static vector<vector<unsigned int>> additionalTuplesWithLowestAmongHighestMembershipDegrees;
  static vector<unsigned int> old2NewDimensionOrder;
  static vector<vector<unsigned int>> oldIds2NewIds;

  static vector<unsigned int> oldIds2NewIdsInDimension(const vector<pair<double, unsigned int>>& elements);
  static void addDefaultPattern(const vector<unsigned int>& tuple);
  static void addRemappedDefaultPattern(const vector<unsigned int>& tuple);
};

#endif /*CONCURRENT_PATTERN_POOL_H_*/
