// Copyright 2018-2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include <cmath>

#include "DenseRoughTensor.h"

#include "../utilities/NoOutputException.h"
#include "TupleFileReader.h"
#include "FuzzyTupleFileReader.h"
#include "SparseRoughTensor.h"
#include "ConcurrentPatternPool.h"
#include "SparseCrispTube.h"
#include "SparseFuzzyTube.h"

vector<vector<string>> AbstractRoughTensor::ids2Labels;
double AbstractRoughTensor::nullModelRSS;
vector<unsigned int> AbstractRoughTensor::cardinalities;
int AbstractRoughTensor::unit;
bool AbstractRoughTensor::isNoSelection;
vector<unsigned int> AbstractRoughTensor::external2InternalDimensionOrder;
vector<vector<vector<unsigned int>>> AbstractRoughTensor::candidateVariables;
bool AbstractRoughTensor::isPrintLambda;

ofstream AbstractRoughTensor::outputFile;
string AbstractRoughTensor::outputDimensionSeparator;
string AbstractRoughTensor::outputElementSeparator;
string AbstractRoughTensor::hierarchyPrefix;
string AbstractRoughTensor::hierarchySeparator;
string AbstractRoughTensor::sizePrefix;
string AbstractRoughTensor::sizeSeparator;
string AbstractRoughTensor::areaPrefix;
string AbstractRoughTensor::rssPrefix;
bool AbstractRoughTensor::isSizePrinted;
bool AbstractRoughTensor::isAreaPrinted;

#if defined TIME || defined DETAILED_TIME
steady_clock::time_point AbstractRoughTensor::overallBeginning;
#endif
#ifdef DETAILED_TIME
steady_clock::time_point AbstractRoughTensor::shiftingBeginning;
#endif

AbstractRoughTensor::~AbstractRoughTensor()
{
  outputFile.close();
}

void AbstractRoughTensor::printDimension(const vector<unsigned int>& dimension, const vector<string>& ids2LabelsInDimension, ostream& out)
{
  vector<unsigned int>::const_iterator elementIt = dimension.begin();
  out << ids2LabelsInDimension[*elementIt];
  for (const vector<unsigned int>::const_iterator end = dimension.end(); ++elementIt != end; )
    {
      out << outputElementSeparator << ids2LabelsInDimension[*elementIt];
    }
  out << outputDimensionSeparator;
}

void AbstractRoughTensor::printPattern(const vector<vector<unsigned int>>& nSet, const float density, ostream& out) const
{
  vector<vector<string>>::const_iterator ids2LabelsIt = ids2Labels.begin();
  vector<unsigned int>::const_iterator internalDimensionIdIt = external2InternalDimensionOrder.begin();
  printDimension(nSet[*internalDimensionIdIt], *ids2LabelsIt, out);
  ++ids2LabelsIt;
  ++internalDimensionIdIt;
  const vector<unsigned int>::const_iterator internalDimensionIdEnd = external2InternalDimensionOrder.end();
  do
    {
      printDimension(nSet[*internalDimensionIdIt], *ids2LabelsIt, out);
      ++ids2LabelsIt;
    }
  while (++internalDimensionIdIt != internalDimensionIdEnd);
  if (isPrintLambda)
    {
      out << density / unit;
    }
  else
    {
      out << density / unit + getAverageShift(nSet);
    }
  if (isSizePrinted)
    {
      internalDimensionIdIt = external2InternalDimensionOrder.begin();
      out << sizePrefix << nSet[*internalDimensionIdIt].size();
      ++internalDimensionIdIt;
      do
	{
	  out << sizeSeparator << nSet[*internalDimensionIdIt].size();
	}
      while (++internalDimensionIdIt != internalDimensionIdEnd);
    }
  if (isAreaPrinted)
    {
      vector<vector<unsigned int>>::const_iterator dimensionIt = nSet.begin();
      unsigned long long area = dimensionIt->size();
      ++dimensionIt;
      const vector<vector<unsigned int>>::const_iterator dimensionEnd = nSet.end();
      do
	{
	  area *= dimensionIt->size();
	}
      while (++dimensionIt != dimensionEnd);
      out << areaPrefix << area;
    }
}

void AbstractRoughTensor::output(const vector<vector<unsigned int>>& nSet, const float density) const
{
  printPattern(nSet, density, outputFile);
  outputFile << '\n';
}

void AbstractRoughTensor::output(const vector<vector<unsigned int>>& nSet, const float density, const double rss) const
{
  printPattern(nSet, density, outputFile);
  outputFile << rssPrefix << rss / unit / unit << '\n';
}

AbstractRoughTensor* AbstractRoughTensor::makeRoughTensor(vector<FuzzyTuple>& fuzzyTuples, const double densityThreshold, const double shift)
{
  if (Trie::is01)
    {
      if ((8 * sizeof(double) + 1) * getAreaFromIds2Labels() < 8 * (3 * sizeof(unsigned int*) + sizeof(unsigned int) * (ids2Labels.size() + 1) + sizeof(double)) * fuzzyTuples.size())
      	{
	  // Dense storage (including the rough tensor) takes less space, assuming the sparse storage would only use sparse tubes
	  return new DenseRoughTensor(fuzzyTuples, shift);
      	}
      setMetadata(fuzzyTuples, shift);
      SparseCrispTube::setDefaultMembershipAndSizeLimit(unit * -shift, densityThreshold * cardinalities.back() / sizeof(unsigned int) / 8);
      DenseCrispTube::setSize(cardinalities.back());
      return new SparseRoughTensor(fuzzyTuples, shift);
    }
  if ((sizeof(unsigned int) + sizeof(double)) * getAreaFromIds2Labels() < (3 * sizeof(unsigned int*) + sizeof(unsigned int) * (ids2Labels.size() + 2) + sizeof(double)) * fuzzyTuples.size())
    {
      // Dense storage (including the rough tensor) takes less space, assuming the sparse storage would only use sparse tubes
      return new DenseRoughTensor(fuzzyTuples, shift);
    }
  setMetadata(fuzzyTuples, shift);
  SparseFuzzyTube::setDefaultMembershipAndSizeLimit(unit * -shift, densityThreshold * cardinalities.back() / 2);
  DenseFuzzyTube::setSize(cardinalities.back());
  return new SparseRoughTensor(fuzzyTuples, shift);
}

AbstractRoughTensor* AbstractRoughTensor::makeRoughTensor(const char* tensorFileName, const char* inputDimensionSeparator, const char* inputElementSeparator, const double densityThreshold, const bool isInput01, const bool isVerbose)
{
  vector<FuzzyTuple> fuzzyTuples = getFuzzyTuples(tensorFileName, inputDimensionSeparator, inputElementSeparator, isInput01, isVerbose);
  double shift;
  if (Trie::is01)
    {
      shift = fuzzyTuples.size();
    }
  else
    {
      vector<FuzzyTuple>::const_iterator fuzzyTupleIt = fuzzyTuples.begin();
      shift = fuzzyTupleIt->getMembership();
      for (const vector<FuzzyTuple>::const_iterator fuzzyTupleEnd = fuzzyTuples.end(); ++fuzzyTupleIt != fuzzyTupleEnd; )
	{
	  shift += fuzzyTupleIt->getMembership();
	}
    }
  vector<vector<string>>::const_iterator ids2LabelsInDimensionIt = ids2Labels.begin();
  shift /= ids2LabelsInDimensionIt->size();
  ++ids2LabelsInDimensionIt;
  const vector<vector<string>>::const_iterator ids2LabelsInDimensionEnd = ids2Labels.end();
  do
    {
      shift /= ids2LabelsInDimensionIt->size();
    }
  while (++ids2LabelsInDimensionIt != ids2LabelsInDimensionEnd);
  return makeRoughTensor(fuzzyTuples, densityThreshold, shift);
}

AbstractRoughTensor* AbstractRoughTensor::makeRoughTensor(const char* tensorFileName, const char* inputDimensionSeparator, const char* inputElementSeparator, const double densityThreshold, const double shift, const bool isInput01, const bool isVerbose)
{
  vector<FuzzyTuple> fuzzyTuples = getFuzzyTuples(tensorFileName, inputDimensionSeparator, inputElementSeparator, isInput01, isVerbose);
  return makeRoughTensor(fuzzyTuples, densityThreshold, shift);
}

void AbstractRoughTensor::setOutput(const char* outputFileName, const char* outputDimensionSeparatorParam, const char* outputElementSeparatorParam, const char* hierarchyPrefixParam, const char* hierarchySeparatorParam, const char* sizePrefixParam, const char* sizeSeparatorParam, const char* areaPrefixParam, const char* rssPrefixParam, const bool isPrintLambdaParam, const bool isSizePrintedParam, const bool isAreaPrintedParam, const bool isNoSelectionParam)
{
#ifdef DETAILED_TIME
#ifdef GNUPLOT
  cout << '\t' << duration_cast<duration<double>>(steady_clock::now() - shiftingBeginning).count();
#else
  cout << "Tensor shifting time: " << duration_cast<duration<double>>(steady_clock::now() - shiftingBeginning).count() << "s\n";
#endif
#endif
  outputDimensionSeparator = outputDimensionSeparatorParam;
  outputElementSeparator = outputElementSeparatorParam;
  hierarchyPrefix = hierarchyPrefixParam;
  hierarchySeparator = hierarchySeparatorParam;
  sizePrefix = sizePrefixParam;
  sizeSeparator = sizeSeparatorParam;
  areaPrefix = areaPrefixParam;
  rssPrefix = rssPrefixParam;
  isPrintLambda = isPrintLambdaParam;
  isSizePrinted = isSizePrintedParam;
  isAreaPrinted = isAreaPrintedParam;
  isNoSelection = isNoSelectionParam;
  outputFile.open(outputFileName);
  if (!outputFile)
    {
      throw NoOutputException(outputFileName);
    }
}

int AbstractRoughTensor::getUnit()
{
  return unit;
}

bool AbstractRoughTensor::isDirectOutput()
{
  return isNoSelection;
}

const vector<unsigned int>& AbstractRoughTensor::getCardinalities()
{
  return cardinalities;
}

const vector<vector<string>>& AbstractRoughTensor::getIds2Labels()
{
  return ids2Labels;
}

const vector<unsigned int>& AbstractRoughTensor::getExternal2InternalDimensionOrder()
{
  return external2InternalDimensionOrder;
}

unsigned int AbstractRoughTensor::getNbOfCandidateVariables()
{
  return candidateVariables.size();
}

unsigned long long AbstractRoughTensor::getArea()
{
  const vector<unsigned int>::const_iterator cardinalityEnd = cardinalities.end();
  vector<unsigned int>::const_iterator cardinalityIt = cardinalities.begin();
  unsigned long long area = *cardinalityIt++;
  do
    {
      area *= *cardinalityIt++;
    }
  while (cardinalityIt != cardinalityEnd);
  return area;
}

double AbstractRoughTensor::unitDenominatorGivenNullModelRSS()
{
  if (nullModelRSS > 1)
    {
      // sqrt(nullModelRSS) is a possible unit denominator (sqrt because the RSS is stored in a long long)
      return sqrt(nullModelRSS);
    }
  return 1;
}

void AbstractRoughTensor::setUnit(const int unitParam)
{
#if defined NUMERIC_PRECISION && !defined GNUPLOT
  cout << "Numeric precision: " << 1. / unitParam << '\n';
#endif
  unit = unitParam;
}

void AbstractRoughTensor::setUnitForProjectedTensor(const double rss, const vector<double>& elementNegativeMemberships, const vector<double>& elementPositiveMemberships)
{
  if (rss > 1)
    {
#if defined NUMERIC_PRECISION && defined GNUPLOT
      cout << '\t' << max(sqrt(rss), max(*max_element(elementNegativeMemberships.begin(), elementNegativeMemberships.end()), *max_element(elementPositiveMemberships.begin(), elementPositiveMemberships.end()))) / numeric_limits<int>::max();
#endif
      setUnit(static_cast<double>(numeric_limits<int>::max()) / max(sqrt(rss), max(*max_element(elementNegativeMemberships.begin(), elementNegativeMemberships.end()), *max_element(elementPositiveMemberships.begin(), elementPositiveMemberships.end()))));
      return;
    }
#if defined NUMERIC_PRECISION && defined GNUPLOT
  cout << '\t' << max(1., max(*max_element(elementNegativeMemberships.begin(), elementNegativeMemberships.end()), *max_element(elementPositiveMemberships.begin(), elementPositiveMemberships.end()))) / numeric_limits<int>::max();
#endif
  setUnit(static_cast<double>(numeric_limits<int>::max()) / max(1., max(*max_element(elementNegativeMemberships.begin(), elementNegativeMemberships.end()), *max_element(elementPositiveMemberships.begin(), elementPositiveMemberships.end()))));
}

vector<FuzzyTuple> AbstractRoughTensor::getFuzzyTuples(const char* tensorFileName, const char* inputDimensionSeparator, const char* inputElementSeparator, const bool isInput01, const bool isVerbose)
{
#if defined TIME || defined DETAILED_TIME
  overallBeginning = steady_clock::now();
#endif
  if (isInput01)
    {
      if (isVerbose)
	{
	  cout << "Parsing Boolean tensor ... " << flush;
	}
      Trie::is01 = true;
      TupleFileReader tupleFileReader(tensorFileName, inputDimensionSeparator, inputElementSeparator);
      vector<FuzzyTuple> tuples = tupleFileReader.read();
      ids2Labels = std::move(tupleFileReader.getIds2Labels());
      if (isVerbose)
	{
	  cout << "\rParsing Boolean tensor: " << tuples.size() << '/' << getAreaFromIds2Labels() << " tuples with nonzero membership degrees.\n" << flush;
	}
#ifdef DETAILED_TIME
      shiftingBeginning = steady_clock::now();
#ifdef GNUPLOT
      cout << duration_cast<duration<double>>(shiftingBeginning - overallBeginning).count();
#else
      cout << "Tensor parsing time: " << duration_cast<duration<double>>(shiftingBeginning - overallBeginning).count() << "s\n";
#endif
#endif
      if (isVerbose)
	{
	  cout << "Shifting tensor ... " << flush;
	}
      return tuples;
    }
  if (isVerbose)
    {
      cout << "Parsing fuzzy tensor ... " << flush;
    }
  FuzzyTupleFileReader fuzzyTupleFileReader(tensorFileName, inputDimensionSeparator, inputElementSeparator);
  pair<vector<FuzzyTuple>, bool> fuzzyTuplesAndIs01 = fuzzyTupleFileReader.read();
  Trie::is01 = fuzzyTuplesAndIs01.second;
  ids2Labels = std::move(fuzzyTupleFileReader.getIds2Labels());
  if (isVerbose)
    {
      cout << "\rParsing fuzzy tensor: " << fuzzyTuplesAndIs01.first.size() << '/' << getAreaFromIds2Labels() << " tuples with nonzero membership degrees.\n" << flush;
    }
#ifdef DETAILED_TIME
  shiftingBeginning = steady_clock::now();
#ifdef GNUPLOT
  cout << duration_cast<duration<double>>(shiftingBeginning - overallBeginning).count();
#else
  cout << "Tensor parsing time: " << duration_cast<duration<double>>(shiftingBeginning - overallBeginning).count() << "s\n";
#endif
#endif
  if (isVerbose)
    {
      cout << "Shifting tensor ... " << flush;
    }
  return fuzzyTuplesAndIs01.first;
}

unsigned long long AbstractRoughTensor::getAreaFromIds2Labels()
{
  vector<vector<string>>::const_iterator ids2LabelsInDimensionIt = ids2Labels.begin();
  unsigned long long area = ids2LabelsInDimensionIt->size();
  ++ids2LabelsInDimensionIt;
  const vector<vector<string>>::const_iterator ids2LabelsInDimensionEnd = ids2Labels.end();
  do
    {
      area *= ids2LabelsInDimensionIt->size();
    }
  while (++ids2LabelsInDimensionIt != ids2LabelsInDimensionEnd);
  return area;
}

void AbstractRoughTensor::orderDimensionsAndSetExternal2InternalDimensionOrderAndCardinalities()
{
  vector<vector<string>>::const_iterator ids2LabelsInDimensionIt = ids2Labels.begin();
  const unsigned int n = ids2Labels.end() - ids2LabelsInDimensionIt;
  vector<pair<unsigned int, unsigned int>> dimensions;
  dimensions.reserve(n);
  dimensions.emplace_back(ids2LabelsInDimensionIt->size(), 0);
  unsigned int dimensionId = 1;
  do
    {
      dimensions.emplace_back((++ids2LabelsInDimensionIt)->size(), dimensionId);
    }
  while (++dimensionId != n);
  // Order dimensions by increasing cardinality
  sort(dimensions.begin(), dimensions.end(), [](const pair<unsigned int, unsigned int>& dimension1, const pair<unsigned int, unsigned int>& dimension2) {return dimension1.first < dimension2.first;});
  // Compute external2InternalDimensionOrder and cardinalities
  cardinalities.reserve(n);
  external2InternalDimensionOrder.resize(n);
  vector<pair<unsigned int, unsigned int>>::const_iterator dimensionIt = dimensions.begin();
  external2InternalDimensionOrder[dimensionIt->second] = 0;
  cardinalities.push_back(dimensionIt->first);
  dimensionId = 1;
  do
    {
      external2InternalDimensionOrder[(++dimensionIt)->second] = dimensionId;
      cardinalities.push_back(dimensionIt->first);
    }
  while (++dimensionId != n);
}

void AbstractRoughTensor::setMetadataForDimension(const unsigned int dimensionId, const unsigned long long area, const double shift, double& unitDenominator, vector<string>& ids2LabelsInDimension, vector<FuzzyTuple>& fuzzyTuples)
{
  // Sparse tensor
  const unsigned int nbOfElements = ids2LabelsInDimension.size();
  // Computing positive and, for fuzzy tensors, negative memberships of the elements
  vector<pair<double, unsigned int>> elementPositiveMemberships;
  elementPositiveMemberships.reserve(nbOfElements);
  {
    unsigned int id = 0;
    do
      {
	elementPositiveMemberships.emplace_back(0, id);
      }
    while (++id != nbOfElements);
  }
  const vector<FuzzyTuple>::iterator fuzzyTupleEnd = fuzzyTuples.end();
  vector<FuzzyTuple>::iterator fuzzyTupleIt = fuzzyTuples.begin();
  if (Trie::is01)
    {
      const double oneMinusShift = 1. - shift;
      do
	{
	  elementPositiveMemberships[fuzzyTupleIt->getElementId(dimensionId)].first += oneMinusShift;
	}
      while (++fuzzyTupleIt != fuzzyTupleEnd);
    }
  else
    {
      vector<double> elementNegativeMemberships(nbOfElements, shift * (area / nbOfElements)); // assumes every membership null and correct that in the loop below
      do
	{
	  const unsigned int elementId = fuzzyTupleIt->getElementId(dimensionId);
	  const double membership = fuzzyTupleIt->getMembership();
	  if (membership > 0)
	    {
	      elementPositiveMemberships[elementId].first += membership;
	      elementNegativeMemberships[elementId] -= shift;
	    }
	  else
	    {
	      elementNegativeMemberships[elementId] -= membership + shift;
	    }
	}
      while (++fuzzyTupleIt != fuzzyTupleEnd);
      const double maxNegativeMembership = *max_element(elementNegativeMemberships.begin(), elementNegativeMemberships.end());
      if (maxNegativeMembership > unitDenominator)
	{
	  unitDenominator = maxNegativeMembership;
	}
    }
  fuzzyTupleIt = fuzzyTuples.begin();
  sort(elementPositiveMemberships.begin(), elementPositiveMemberships.end(), [](const pair<double, unsigned int>& elementPositiveMembership1, const pair<double, unsigned int>& elementPositiveMembership2) {return elementPositiveMembership1.first < elementPositiveMembership2.first;});
  if (elementPositiveMemberships.back().first > unitDenominator)
    {
      unitDenominator = elementPositiveMemberships.back().first;
    }
  // Computing the new ids, in increasing order of the positive membership (for faster lower_bound in SparseFuzzyTube::sumOnSlice and to choose the element with the greatest membership in case of equality) and reorder ids2LabelsInDimension accordingly
  vector<unsigned int> mapping(nbOfElements);
  {
    vector<string> newIds2LabelsInDimension;
    newIds2LabelsInDimension.reserve(nbOfElements);
    {
      unsigned int newId = 0;
      vector<pair<double, unsigned int>>::const_iterator elementPositiveMembershipIt = elementPositiveMemberships.begin();
      do
	{
	  newIds2LabelsInDimension.emplace_back(std::move(ids2LabelsInDimension[elementPositiveMembershipIt->second]));
	  mapping[elementPositiveMembershipIt->second] = newId;
	  ++elementPositiveMembershipIt;
	}
      while (++newId != nbOfElements);
    }
    ids2LabelsInDimension = std::move(newIds2LabelsInDimension);
  }
  // Remap the element of the fuzzyTuples accordingly
  do
    {
      unsigned int& elementId = fuzzyTupleIt->getElementId(dimensionId);
      elementId = mapping[elementId];
    }
  while (++fuzzyTupleIt != fuzzyTupleEnd);
}

void AbstractRoughTensor::setMetadata(vector<FuzzyTuple>& fuzzyTuples, const double shift)
{
  // Sparse tensor
  const unsigned long long area = getAreaFromIds2Labels();
  orderDimensionsAndSetExternal2InternalDimensionOrderAndCardinalities();
  double unitDenominator;
  const vector<FuzzyTuple>::iterator fuzzyTupleEnd = fuzzyTuples.end();
  vector<FuzzyTuple>::iterator fuzzyTupleIt = fuzzyTuples.begin();
  if (Trie::is01)
    {
      nullModelRSS = (static_cast<double>(shift) * area - 2 * fuzzyTuples.size()) * shift + fuzzyTuples.size();
      // Shift the membership of every tuple
      do
	{
	  fuzzyTupleIt->shiftMembership(shift);
	}
      while (++fuzzyTupleIt != fuzzyTupleEnd);
      unitDenominator = unitDenominatorGivenNullModelRSS();
      const double maxElementNegativeMembership = shift * (area / cardinalities.back());
      if (maxElementNegativeMembership > unitDenominator)
	{
	  unitDenominator = maxElementNegativeMembership;
	}
    }
  else
    {
      nullModelRSS = static_cast<double>(shift) * shift * (area - fuzzyTuples.size());
      // Shift the membership of every tuple
      do
	{
	  fuzzyTupleIt->shiftMembership(shift);
	  nullModelRSS += fuzzyTupleIt->getMembershipSquared();
	}
      while (++fuzzyTupleIt != fuzzyTupleEnd);
      unitDenominator = unitDenominatorGivenNullModelRSS();
    }
  fuzzyTupleIt = fuzzyTuples.begin();
  {
    vector<vector<string>>::iterator ids2LabelsInDimensionIt = ids2Labels.begin();
    setMetadataForDimension(0, area, shift, unitDenominator, *ids2LabelsInDimensionIt, fuzzyTuples);
    const unsigned int n = ids2Labels.end() - ids2LabelsInDimensionIt;
    unsigned int dimensionId = 1;
    do
      {
	setMetadataForDimension(dimensionId, area, shift, unitDenominator, *++ids2LabelsInDimensionIt, fuzzyTuples);
      }
    while (++dimensionId != n);
    ConcurrentPatternPool::setNbOfDimensions(n);
  }
  // Reorder fuzzy tuples, according to the new dimension order
  do
    {
      fuzzyTupleIt->reorder(external2InternalDimensionOrder);
      ConcurrentPatternPool::addFuzzyTuple(fuzzyTupleIt->getTuple(), fuzzyTupleIt->getMembership());
    }
  while (++fuzzyTupleIt != fuzzyTupleEnd);
  // Set unit
#if defined NUMERIC_PRECISION && defined DETAILED_TIME && defined GNUPLOT
  cout << '\t' << unitDenominator / numeric_limits<int>::max();
#endif
  setUnit(static_cast<double>(numeric_limits<int>::max()) / unitDenominator);
}

void AbstractRoughTensor::setMetadataForDimension(vector<pair<double, unsigned int>>& elementPositiveMembershipsInDimension, double& unitDenominator, vector<string>& ids2LabelsInDimension)
{
  // Dense tensor
  sort(elementPositiveMembershipsInDimension.begin(), elementPositiveMembershipsInDimension.end(), [](const pair<double, unsigned int>& elementPositiveMembership1, const pair<double, unsigned int>& elementPositiveMembership2) {return elementPositiveMembership1.first < elementPositiveMembership2.first;});
  const unsigned int nbOfElements = ids2LabelsInDimension.size();
  // Computing the new ids, in increasing order of the positive membership (for faster lower_bound in SparseFuzzyTube::sumOnSlice and to choose the element with the greatest membership in case of equality) and reorder ids2LabelsInDimension accordingly
  vector<string> newIds2LabelsInDimension;
  newIds2LabelsInDimension.reserve(nbOfElements);
  const vector<pair<double, unsigned int>>::const_iterator elementPositiveMembershipEnd = elementPositiveMembershipsInDimension.end();
  vector<pair<double, unsigned int>>::const_iterator elementPositiveMembershipIt = elementPositiveMembershipsInDimension.begin();
  do
    {
      newIds2LabelsInDimension.emplace_back(std::move(ids2LabelsInDimension[elementPositiveMembershipIt->second]));
    }
  while (++elementPositiveMembershipIt != elementPositiveMembershipEnd);
  ids2LabelsInDimension = std::move(newIds2LabelsInDimension);
  if (elementPositiveMembershipsInDimension.back().first > unitDenominator)
    {
      unitDenominator = elementPositiveMembershipsInDimension.back().first;
    }
}

void AbstractRoughTensor::setMetadata(vector<vector<pair<double, unsigned int>>>& elementPositiveMemberships, const double maxNegativeMembership)
{
  // Dense tensor
  orderDimensionsAndSetExternal2InternalDimensionOrderAndCardinalities();
  double unitDenominator = unitDenominatorGivenNullModelRSS();
  if (maxNegativeMembership > unitDenominator)
    {
      unitDenominator = maxNegativeMembership;
    }
  vector<vector<pair<double, unsigned int>>>::iterator elementPositiveMembershipsInDimensionIt = elementPositiveMemberships.begin();
  vector<vector<string>>::iterator ids2LabelsInDimensionIt = ids2Labels.begin();
  setMetadataForDimension(*elementPositiveMembershipsInDimensionIt, unitDenominator, *ids2LabelsInDimensionIt);
  ++ids2LabelsInDimensionIt;
  const vector<vector<string>>::iterator ids2LabelsInDimensionEnd = ids2Labels.end();
  do
    {
      setMetadataForDimension(*++elementPositiveMembershipsInDimensionIt, unitDenominator, *ids2LabelsInDimensionIt);
    }
  while (++ids2LabelsInDimensionIt != ids2LabelsInDimensionEnd);
#if defined NUMERIC_PRECISION && defined DETAILED_TIME && defined GNUPLOT
  cout << '\t' << unitDenominator / numeric_limits<int>::max();
#endif
  setUnit(static_cast<double>(numeric_limits<int>::max()) / unitDenominator);
}

void AbstractRoughTensor::projectMetadataForDimension(const unsigned int internalDimensionId, const unsigned int nbOfPatternsHavingAllElements, const bool isReturningOld2New, vector<string>& ids2LabelsInDimension, vector<unsigned int>& newIds2OldIdsInDimension)
{
  unsigned int& cardinality = cardinalities[internalDimensionId];
  dynamic_bitset<> elementsInDimension(cardinality);
  const vector<vector<vector<unsigned int>>>::const_iterator end = candidateVariables.end();
  vector<vector<vector<unsigned int>>>::const_iterator patternIt = end - nbOfPatternsHavingAllElements;
  do
    {
      const vector<unsigned int>::const_iterator idEnd = (*patternIt)[internalDimensionId].end();
      vector<unsigned int>::const_iterator idIt = (*patternIt)[internalDimensionId].begin();
      do
	{
	  elementsInDimension.set(*idIt);
	}
      while (++idIt != idEnd);
    }
  while (++patternIt != end);
  vector<unsigned int> oldIds2NewIdsInDimension(cardinality, numeric_limits<unsigned int>::max());
  cardinality = 0;
  if (isReturningOld2New)
    {
      dynamic_bitset<>::size_type id = elementsInDimension.find_first();
      do
	{
	  ids2LabelsInDimension[id].swap(ids2LabelsInDimension[cardinality]);
	  oldIds2NewIdsInDimension[id] = cardinality++;
	  id = elementsInDimension.find_next(id);
	}
      while (id != dynamic_bitset<>::npos);
    }
  else
    {
      newIds2OldIdsInDimension.reserve(elementsInDimension.count());
      dynamic_bitset<>::size_type id = elementsInDimension.find_first();
      do
	{
	  newIds2OldIdsInDimension.push_back(id);
	  ids2LabelsInDimension[id].swap(ids2LabelsInDimension[cardinality]);
	  oldIds2NewIdsInDimension[id] = cardinality++;
	  id = elementsInDimension.find_next(id);
	}
      while (id != dynamic_bitset<>::npos);
    }
  const vector<vector<vector<unsigned int>>>::iterator candidateVariableEnd = candidateVariables.end();
  vector<vector<vector<unsigned int>>>::iterator candidateVariableIt = candidateVariables.begin();
  do
    {
      const vector<unsigned int>::iterator idEnd = (*candidateVariableIt)[internalDimensionId].end();
      vector<unsigned int>::iterator idIt = (*candidateVariableIt)[internalDimensionId].begin();
      do
	{
	  *idIt = oldIds2NewIdsInDimension[*idIt];
	}
      while (++idIt != idEnd);
    }
  while (++candidateVariableIt != candidateVariableEnd);
  if (isReturningOld2New)
    {
      newIds2OldIdsInDimension = std::move(oldIds2NewIdsInDimension);
    }
}

vector<vector<unsigned int>> AbstractRoughTensor::projectMetadata(const unsigned int nbOfPatternsHavingAllElements, const bool isReturningOld2New)
{
  // Compute new cardinalities, new ids and rewrite ids2Labels and candidateVariables accordingly
  vector<vector<string>>::iterator ids2LabelsIt = ids2Labels.begin();
  vector<vector<unsigned int>> idMapping(external2InternalDimensionOrder.size());
  const vector<unsigned int>::const_iterator internalDimensionIdEnd = external2InternalDimensionOrder.end();
  vector<unsigned int>::const_iterator internalDimensionIdIt = external2InternalDimensionOrder.begin();
  projectMetadataForDimension(*internalDimensionIdIt, nbOfPatternsHavingAllElements, isReturningOld2New, *ids2LabelsIt, idMapping[*internalDimensionIdIt]);
  ++internalDimensionIdIt;
  do
    {
      projectMetadataForDimension(*internalDimensionIdIt, nbOfPatternsHavingAllElements, isReturningOld2New, *++ids2LabelsIt, idMapping[*internalDimensionIdIt]);
    }
  while (++internalDimensionIdIt != internalDimensionIdEnd);
  return idMapping;
}

void AbstractRoughTensor::insertCandidateVariables(vector<vector<vector<unsigned int>>>& additionalCandidateVariables)
{
  std::move(additionalCandidateVariables.begin(), additionalCandidateVariables.end(), back_inserter(candidateVariables));
}

vector<vector<vector<unsigned int>>>& AbstractRoughTensor::getCandidateVariables()
{
  return candidateVariables;
}

double AbstractRoughTensor::getNullModelRSS()
{
  return nullModelRSS * unit * unit;
}

#if defined DEBUG_MODIFY || defined ASSERT
void AbstractRoughTensor::printElement(const unsigned int dimensionId, const unsigned int elementId, ostream& out)
{
  vector<unsigned int>::const_iterator internalDimensionIdIt = external2InternalDimensionOrder.begin();
  for (; *internalDimensionIdIt != dimensionId; ++internalDimensionIdIt)
    {
    }
  const unsigned int externalId = internalDimensionIdIt - external2InternalDimensionOrder.begin();
  out << ids2Labels[externalId][elementId] << " of dimension " << externalId;
}
#endif

#ifdef TIME
void AbstractRoughTensor::printCurrentDuration()
{
#ifdef GNUPLOT
#if defined NUMERIC_PRECISION || defined NB_OF_PATTERNS || defined DETAILED_TIME
  cout << '\t';
#endif
  cout << duration_cast<duration<double>>(steady_clock::now() - overallBeginning).count();
#else
  cout << "Total time: " << duration_cast<duration<double>>(steady_clock::now() - overallBeginning).count() << "s\n";;
#endif
}
#endif
