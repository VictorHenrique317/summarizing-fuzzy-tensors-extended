// Copyright 2022,2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef RANK_PATTERNS_H_
#define RANK_PATTERNS_H_

#include "SelectionCriterion.h"
#include "AbstractRoughTensor.h"
#include "CandidateVariable.h"

class RankPatterns
{
 public:
  static void rank(const unsigned int nbOfCandidateVariablesHavingAllElements, AbstractRoughTensor* roughTensor, const float verboseStep, const int maxSelectionSize, const SelectionCriterion selectionCriterion, const bool isRSSPrinted);

 private:
  static TrieWithPrediction tensor;
  static vector<CandidateVariable> candidates;
  static vector<CandidateVariable>::iterator selectedIt;
  static vector<CandidateVariable>::iterator selectionEnd;
  static vector<CandidateVariable>::iterator candidateBegin;
  static vector<CandidateVariable>::iterator candidateEnd;
  static long long rssVariation;
  static long long maxRSSVariation;

#ifdef DEBUG_SELECT
  static AbstractRoughTensor* roughTensorForDebug;
#endif
#ifdef DETAILED_TIME
  static steady_clock::time_point startingPoint;
#endif

  static void printProgressionOnSTDIN(const float stepInSeconds);

  static void updateCandidate(const CandidateVariable& lastSelectedPattern, CandidateVariable& candidate);
  static void selectForAddition(const CandidateVariable& lastSelectedPattern);
  static void updatePreviouslySelected(const CandidateVariable& lastSelectedPattern, CandidateVariable& previouslySelected);
  static void selectForRemoval(const CandidateVariable& lastSelectedPattern);
  static void reselectOne();
  static void reselect();
  static void output(const AbstractRoughTensor* roughTensor, const bool isRSSPrinted, const vector<double>& rssHistory, const float verboseStep);
};

#endif	/* RANK_PATTERNS_H_ */
