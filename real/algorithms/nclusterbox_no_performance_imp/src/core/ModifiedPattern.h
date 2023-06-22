// Copyright 2022,2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef MODIFIED_PATTERN_H_
#define MODIFIED_PATTERN_H_

#include <unordered_set>
#include <mutex>
#include <boost/container_hash/hash.hpp>

#include "AbstractRoughTensor.h"

enum NextStep { insert, erase, stop };

class ModifiedPattern
{
 public:
  ModifiedPattern();

  void modify();
  void grow();

  static void setContext(const AbstractRoughTensor* roughTensor, const bool isEveryVisitedPatternStored);
  static unsigned int getNbOfOutputPatterns();
  static void insertCandidateVariables();

 private:
  vector<vector<unsigned int>> nSet;
  unsigned long long area;
  long long membershipSum;
  vector<vector<int>> sumsOnHyperplanes;

  NextStep nextStep;
  double bestG;
  vector<vector<unsigned int>>::iterator bestDimensionIt;
  vector<int>::const_iterator bestSumIt;

  // if isEveryVisitedPatternStored && AbstractRoughTensor::isDirectOutput(), no container below is used, otherwise one single is used
  vector<vector<vector<unsigned int>>> candidateVariables; // if isEveryVisitedPatternStored && !AbstractRoughTensor::isDirectOutput()
  static unordered_set<vector<vector<unsigned int>>, boost::hash<vector<vector<unsigned int>>>> distinctCandidateVariables; // if !isEveryVisitedPatternStored

  static unsigned int nbOfOutputPatterns;
  static mutex candidateVariablesLock;

  static const AbstractRoughTensor* roughTensor;
  static bool isEveryVisitedPatternStored;
  static Trie tensor;

  void init();
  void considerDimensionForNextModificationStep(const vector<vector<unsigned int>>::iterator dimensionIt, const vector<int>& sumsInDimension);
  void considerDimensionForNextGrowingStep(const vector<vector<unsigned int>>::iterator dimensionIt, const vector<int>& sumsInDimension, const vector<unsigned int>& firstNonInitialAndSubsequentInitial);
  bool doStep(); // returns whether to go on
  void terminate();

#ifdef ASSERT
  void assertAreaAndSums();
#endif
};

#endif	/* MODIFIED_PATTERN_H_ */
