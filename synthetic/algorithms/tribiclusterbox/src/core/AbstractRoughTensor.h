// Copyright 2018-2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef ABSTRACT_ROUGH_TENSOR_H_
#define ABSTRACT_ROUGH_TENSOR_H_

#include <string>
#include <fstream>

#include "../../Parameters.h"
#include "FuzzyTuple.h"
#include "Trie.h"
#include "TrieWithPrediction.h"

#if defined TIME || defined DETAILED_TIME
#include <chrono>

using namespace std::chrono;
#endif

class AbstractRoughTensor
{
 public:
  static vector<vector<vector<unsigned int>>> candidateVariables;

  virtual ~AbstractRoughTensor();

  virtual void setNoSelection() = 0;
  virtual Trie getTensor() const = 0;
  virtual TrieWithPrediction projectTensor() = 0;
  virtual double getAverageShift(const vector<vector<unsigned int>>& nSet) const = 0;

  void printPattern(const vector<vector<unsigned int>>& nSet, const float density, ostream& out) const;
  void output(const vector<vector<unsigned int>>& nSet, const float density) const;
  void output(const vector<vector<unsigned int>>& nSet, const float density, const double rss) const;

  static AbstractRoughTensor* makeRoughTensor(const char* tensorFileName, const char* inputDimensionSeparator, const char* inputElementSeparator, const double densityThreshold, const bool isInput01, const bool isVerbose);
  static AbstractRoughTensor* makeRoughTensor(const char* tensorFileName, const char* inputDimensionSeparator, const char* inputElementSeparator, const double densityThreshold, const double shift, const bool isInput01, const bool isVerbose);

  static void setOutput(const char* outputFileName, const char* outputDimensionSeparator, const char* outputElementSeparator, const char* hierarchyPrefix, const char* hierarchySeparator, const char* sizePrefix, const char* sizeSeparator, const char* areaPrefix, const char* rssPrefix, const bool isPrintLambda, const bool isSizePrinted, const bool isAreaPrinted, const bool isNoSelection);
  static int getUnit();
  static bool isDirectOutput();
  static const vector<unsigned int>& getCardinalities();
  static const vector<vector<string>>& getIds2Labels();
  static const vector<unsigned int>& getExternal2InternalDimensionOrder();
  static unsigned long long getArea();
  static double getNullModelRSS();

#if defined DEBUG_MODIFY || defined ASSERT
  static void printElement(const unsigned int dimensionId, const unsigned int elementId, ostream& out);
#endif
#ifdef TIME
  static void printCurrentDuration();
#endif

 protected:
  static vector<vector<string>> ids2Labels;
  static double nullModelRSS;
  static vector<unsigned int> cardinalities;
  static int unit;
  static bool isNoSelection;
  static vector<unsigned int> external2InternalDimensionOrder;

  static double unitDenominatorGivenNullModelRSS();
  static void setUnit(const int unit);
  static void setUnitForProjectedTensor(const double rss, const vector<double>& elementNegativeMemberships, const vector<double>& elementPositiveMemberships);
  static vector<FuzzyTuple> getFuzzyTuples(const char* tensorFileName, const char* inputDimensionSeparator, const char* inputElementSeparator, const bool isInput01, const bool isVerbose);
  static void orderDimensionsAndSetExternal2InternalDimensionOrderAndCardinalities();
  static void setMetadata(vector<FuzzyTuple>& fuzzyTuples, const double shift);
  static void setMetadata(vector<vector<pair<double, unsigned int>>>& elementPositiveMemberships, const double maxNegativeMembership); /* the inner vectors of elementPositiveMemberships are reordered by increasing element membership, hence a mapping from new ids (the index) and old ids (the second component of the pairs) */
  static vector<vector<unsigned int>> projectMetadata(const bool isReturningOld2New);

 private:
  static ostream outputStream;
  static ofstream outputFile;
  static string outputDimensionSeparator;
  static string outputElementSeparator;
  static string hierarchyPrefix;
  static string hierarchySeparator;
  static string sizePrefix;
  static string sizeSeparator;
  static string areaPrefix;
  static string rssPrefix;
  static bool isPrintLambda;
  static bool isSizePrinted;
  static bool isAreaPrinted;

#if defined TIME || defined DETAILED_TIME
  static steady_clock::time_point overallBeginning;
#ifdef DETAILED_TIME
  static steady_clock::time_point shiftingBeginning;
#endif
#endif

  static AbstractRoughTensor* makeRoughTensor(vector<FuzzyTuple>& fuzzyTuples, const double densityThreshold, const double shift);
  static unsigned long long getAreaFromIds2Labels();
  static void printDimension(const vector<unsigned int>& dimension, const vector<string>& ids2LabelsInDimension, ostream& out);
  static void setMetadataForDimension(const unsigned int dimensionId, const unsigned long long area, const double shift, double& unitDenominator, vector<string>& ids2LabelsInDimension, vector<FuzzyTuple>& fuzzyTuples);
  static void setMetadataForDimension(vector<pair<double, unsigned int>>& elementPositiveMembershipsInDimension, double& unitDenominator, vector<string>& ids2LabelsInDimension);
  static void projectMetadataForDimension(const unsigned int internalDimensionId, const bool isReturningOld2New, vector<string>& ids2LabelsInDimension, vector<unsigned int>& newIds2OldIdsInDimension);
};

#endif /*ABSTRACT_ROUGH_TENSOR_H_*/
