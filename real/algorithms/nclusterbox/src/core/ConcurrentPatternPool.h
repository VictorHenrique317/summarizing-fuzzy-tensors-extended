// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef CONCURRENT_PATTERN_POOL_H_
#define CONCURRENT_PATTERN_POOL_H_

#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <boost/container_hash/hash.hpp>

using namespace std;

class ConcurrentPatternPool
{
 public:
  static void setReadFromFile();
  static void setDefaultPatterns();
  static void setNbOfDimensions(const unsigned int nbOfDimensions);
  static bool readFromFile(const unsigned int maxNbOfInitialPatterns);

  static void addPattern(vector<vector<unsigned int>>& pattern);
  static void addFuzzyTuple(const vector<unsigned int>& tuple, const double shiftedMembership);
  static void setNewDimensionOrderAndNewIds(const vector<unsigned int>& old2NewDimensionOrder, const vector<vector<pair<double, unsigned int>>>& elementPositiveMemberships);
  static void allPatternsAdded();
  static vector<vector<unsigned int>> next();

  static void printProgressionOnSTDIN(const float stepInSeconds);

 private:
  static vector<vector<vector<unsigned int>>> patterns;
  static mutex patternsLock;
  static condition_variable cv;
  static bool isDefaultInitialPatterns;
  static bool isAllPatternsAdded;
  static vector<unordered_map<vector<unsigned int>, vector<pair<unsigned int, double>>, boost::hash<vector<unsigned int>>>> tuples;
  static vector<unsigned int> old2NewDimensionOrder;
  static vector<vector<unsigned int>> oldIds2NewIds;

  static vector<unsigned int> oldIds2NewIdsInDimension(const vector<pair<double, unsigned int>>& elements);
  static pair<vector<vector<unsigned int>>, double> subFiberMaximizingG(const vector<unsigned int>& constrainedDimensions, vector<pair<unsigned int, double>>& freeDimension, const unsigned int freeDimensionId);
  static pair<vector<vector<unsigned int>>, double> remappedSubFiberMaximizingG(const vector<unsigned int>& constrainedDimensions, vector<pair<unsigned int, double>>& freeDimension, const unsigned int freeDimensionId);
};

#endif /*CONCURRENT_PATTERN_POOL_H_*/
