// Copyright 2022,2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef MODIFIED_PATTERN_H_
#define MODIFIED_PATTERN_H_

#include "AbstractRoughTensor.h"

enum NextStep { insert, erase, stop };

class ModifiedPattern
{
 public:
  ModifiedPattern();

  void modify();
  void grow();

  void insertCandidateVariables();
  void output() const;
  unsigned int outputAndGetSizeOfOutput() const;

  static void setContext(const AbstractRoughTensor* roughTensor, const bool isIntermediaryPatternsOutput);
  static void clearAndFree();

 private:
  vector<vector<unsigned int>> nSet;
  unsigned long long area;
  long long membershipSum;
  vector<vector<int>> sumsOnHyperplanes;

  NextStep nextStep;
  double bestG;
  vector<vector<unsigned int>>::iterator bestDimensionIt;
  vector<int>::const_iterator bestSumIt;

  // patternsToOutput is used if AbstractRoughTensor::isDirectOutput(), candidateVariables otherwise
  vector<vector<vector<unsigned int>>> candidateVariables;
  vector<pair<vector<vector<unsigned int>>, float>> patternsToOutput;

  static const AbstractRoughTensor* roughTensor;
  static bool isIntermediaryPatternsOutput;
  static Trie tensor;

  void init();
  void initStep();
  void considerDimensionForNextModificationStep(const vector<vector<unsigned int>>::iterator dimensionIt, const vector<int>& sumsInDimension);
  void considerDimensionForNextGrowingStep(const vector<vector<unsigned int>>::iterator dimensionIt, const vector<int>& sumsInDimension, const vector<unsigned int>& firstNonInitialAndSubsequentInitial);
  bool doStep(); // returns whether to go on

#ifdef ASSERT
  void assertAreaAndSums();
#endif
};

#endif	/* MODIFIED_PATTERN_H_ */
