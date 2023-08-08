// Copyright 2022,2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "ModifiedPattern.h"

#include <algorithm>

#include "ConcurrentPatternPool.h"

#if REMEMBER != 0
#include "VisitedPatterns.h"
#endif

const AbstractRoughTensor* ModifiedPattern::roughTensor;
bool ModifiedPattern::isIntermediaryPatternsOutput;
Trie ModifiedPattern::tensor;

void addFirstNonInitialAndSubsequentInitialInDimension(const vector<unsigned int>& dimension, vector<unsigned int>& firstNonInitialAndSubsequentInitial)
{
  const unsigned int sizeOfDimension = dimension.size();
  unsigned int elementId = 0;
  while (dimension[elementId] == elementId && ++elementId != sizeOfDimension)
    {
    }
  firstNonInitialAndSubsequentInitial.reserve(sizeOfDimension - elementId + 1);
  firstNonInitialAndSubsequentInitial.push_back(elementId);
  firstNonInitialAndSubsequentInitial.insert(firstNonInitialAndSubsequentInitial.end(), dimension.begin() + elementId, dimension.end());
}

ModifiedPattern::ModifiedPattern(): nSet(), area(), membershipSum(), sumsOnHyperplanes(), nextStep(), bestG(), bestDimensionIt(), bestSumIt(), candidateVariables(), patternsToOutput()
{
  const vector<unsigned int>& cardinalities = AbstractRoughTensor::getCardinalities();
  vector<unsigned int>::const_iterator cardinalityIt = cardinalities.begin();
  const vector<unsigned int>::const_iterator cardinalityEnd = cardinalities.end();
  sumsOnHyperplanes.reserve(cardinalityEnd - cardinalityIt);
  sumsOnHyperplanes.emplace_back(*cardinalityIt);
  ++cardinalityIt;
  do
    {
      sumsOnHyperplanes.emplace_back(*cardinalityIt);
    }
  while (++cardinalityIt != cardinalityEnd);
}

void ModifiedPattern::modify()
{
  for (nSet = ConcurrentPatternPool::next(); !nSet.empty(); nSet = ConcurrentPatternPool::next())
    {
#if REMEMBER != 0
      if (VisitedPatterns::visited(nSet))
	{
	  continue;
	}
#endif
      init();
      do
	{
	  initStep();
	  // Decide modification step
	  vector<vector<int>>::const_iterator sumsInDimensionIt = sumsOnHyperplanes.begin();
	  vector<vector<unsigned int>>::iterator dimensionIt = nSet.begin();
	  considerDimensionForNextModificationStep(dimensionIt, *sumsInDimensionIt);
	  ++dimensionIt;
	  ++sumsInDimensionIt;
	  const vector<vector<unsigned int>>::const_iterator dimensionEnd = nSet.end();
	  do
	    {
	      considerDimensionForNextModificationStep(dimensionIt, *sumsInDimensionIt);
	      ++sumsInDimensionIt;
	    }
	  while (++dimensionIt != dimensionEnd);
#ifdef ASSERT
	  assertAreaAndSums();
#endif
	}
      while (doStep());
    }
}

void ModifiedPattern::grow()
{
  for (nSet = ConcurrentPatternPool::next(); !nSet.empty(); nSet = ConcurrentPatternPool::next())
    {
      // assuming no input pattern is a subpattern of another input pattern; if not, the commented code below is useful to never reconsider several times the superpattern (and have it output several times, if AbstractRoughTensor::isDirectOutput())
// #if REMEMBER != 0
//       if (VisitedPatterns::visited(nSet))
// 	{
// 	  continue;
// 	}
// #endif
      init();
      const vector<vector<unsigned int>>::iterator dimensionEnd = nSet.end();
      vector<vector<unsigned int>>::iterator dimensionIt = nSet.begin();
      vector<vector<unsigned int>> firstNonInitialAndSubsequentInitial;
      firstNonInitialAndSubsequentInitial.reserve(dimensionEnd - dimensionIt);
      firstNonInitialAndSubsequentInitial.emplace_back();
      addFirstNonInitialAndSubsequentInitialInDimension(*dimensionIt, firstNonInitialAndSubsequentInitial.back());
      ++dimensionIt;
      do
	{
	  firstNonInitialAndSubsequentInitial.emplace_back();
	  addFirstNonInitialAndSubsequentInitialInDimension(*dimensionIt, firstNonInitialAndSubsequentInitial.back());
	}
      while (++dimensionIt != dimensionEnd);
      do
	{
	  initStep();
	  // Decide growing step
	  dimensionIt = nSet.begin();
	  vector<vector<unsigned int>>::const_iterator firstNonInitialAndSubsequentInitialIt = firstNonInitialAndSubsequentInitial.begin();
	  vector<vector<int>>::const_iterator sumsInDimensionIt = sumsOnHyperplanes.begin();
	  considerDimensionForNextGrowingStep(dimensionIt, *sumsInDimensionIt, *firstNonInitialAndSubsequentInitialIt);
	  ++sumsInDimensionIt;
	  ++firstNonInitialAndSubsequentInitialIt;
	  ++dimensionIt;
	  do
	    {
	      considerDimensionForNextGrowingStep(dimensionIt, *sumsInDimensionIt, *firstNonInitialAndSubsequentInitialIt);
	      ++sumsInDimensionIt;
	      ++firstNonInitialAndSubsequentInitialIt;
	    }
	  while (++dimensionIt != dimensionEnd);
#ifdef ASSERT
	  assertAreaAndSums();
#endif
	}
      while (doStep());
    }
}

void ModifiedPattern::insertCandidateVariables()
{
  AbstractRoughTensor::insertCandidateVariables(candidateVariables);
}

void ModifiedPattern::output() const
{
  for (const pair<vector<vector<unsigned int>>, float>& patternAndDensity : patternsToOutput)
    {
      roughTensor->output(patternAndDensity.first, patternAndDensity.second);
    }
}

unsigned int ModifiedPattern::outputAndGetSizeOfOutput() const
{
  output();
  return patternsToOutput.size();
}

void ModifiedPattern::init()
{
  vector<vector<unsigned int>>::const_iterator dimensionIt = nSet.begin();
  area = dimensionIt->size();
  ++dimensionIt;
  const vector<vector<unsigned int>>::const_iterator dimensionEnd = nSet.end();
  do
    {
      area *= dimensionIt->size();
    }
  while (++dimensionIt != dimensionEnd);
  if (Trie::is01)
    {
      membershipSum = tensor.sumsOnPatternAndHyperplanes(nSet.begin(), sumsOnHyperplanes, area, AbstractRoughTensor::getUnit());
    }
  else
    {
      membershipSum = tensor.sumsOnPatternAndHyperplanes(nSet.begin(), sumsOnHyperplanes);
    }
  bestG = abs(static_cast<double>(membershipSum)) * membershipSum / area;
#ifdef DEBUG_MODIFY
  roughTensor->printPattern(nSet, static_cast<float>(membershipSum) / area, cout);
  cout << " gives g = " << static_cast<float>(bestG) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << '\n';
#endif
}

void ModifiedPattern::initStep()
{
  nextStep = stop;
  if (isIntermediaryPatternsOutput)
    {
      if (AbstractRoughTensor::isDirectOutput())
	{
	  patternsToOutput.emplace_back(nSet, static_cast<float>(membershipSum) / area);
	  return;
	}
      candidateVariables.emplace_back(nSet);
    }
}

void ModifiedPattern::considerDimensionForNextModificationStep(const vector<vector<unsigned int>>::iterator dimensionIt, const vector<int>& sumsInDimension)
{
  if (sumsInDimension.size() == dimensionIt->size())
    {
      // Every element in the dimension of the tensor is present
      if (dimensionIt->size() != 1)
	{
	  // Any element can be erased from *dimensionIt
	  const vector<int>::const_iterator bestDecreasingSumInDimensionIt = min_element(sumsInDimension.begin(), sumsInDimension.end()); // in case of equality, prefer removing the globally sparsest slice
	  double g = membershipSum - *bestDecreasingSumInDimensionIt;
	  g *= abs(g) / (area / dimensionIt->size() * (dimensionIt->size() - 1));
	  if (g > bestG)
	    {
	      bestG = g;
	      bestDimensionIt = dimensionIt;
	      bestSumIt = bestDecreasingSumInDimensionIt;
	      nextStep = erase;
	    }
	}
      return;
    }
  // Some element absent from *dimensionIt can be added
  vector<int>::const_iterator bestIncreasingSumInDimensionIt;
  vector<int>::const_iterator sumIt;
  if (dimensionIt->size() == 1)
    {
      // No element can be erased from *dimensionIt
      if (dimensionIt->front())
	{
	  sumIt = sumsInDimension.begin() + dimensionIt->front();
	  bestIncreasingSumInDimensionIt = max_element(sumsInDimension.begin(), sumIt, [](const int sum1, const int sum2) {return sum1 <= sum2;}); // in case of equality, prefer adding the globally densest slice
	  ++sumIt;
	}
      else
	{
	  sumIt = ++(sumsInDimension.begin());
	  bestIncreasingSumInDimensionIt = sumIt++;
	}
    }
  else
    {
      // Some element can be erased from *dimensionIt
      vector<int>::const_iterator bestDecreasingSumInDimensionIt;
      const vector<int>::const_iterator sumBegin = sumsInDimension.begin();
      vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
      if (*presentElementIdIt)
	{
	  // Initializing bestDecreasingSumInDimensionIt with the sum relating to the first present element; bestIncreasingSumInDimensionIt with the greatest sum before
	  sumIt = sumBegin + *presentElementIdIt++;
	  bestIncreasingSumInDimensionIt = max_element(sumBegin, sumIt, [](const int sum1, const int sum2) {return sum1 <= sum2;}); // in case of equality, prefer adding the globally densest slice
	  bestDecreasingSumInDimensionIt = sumIt++;
	}
      else
	{
	  // Initializing bestIncreasingSumInDimensionIt with the sum relating to the first absent element; bestDecreasingSumInDimensionIt with the lowest sum before
	  bestDecreasingSumInDimensionIt = sumBegin;
	  sumIt = sumBegin;
	  unsigned int elementId = 1;
	  for (const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end(); ++presentElementIdIt != presentElementIdEnd && *presentElementIdIt == elementId; ++elementId)
	    {
	      if (*++sumIt < *bestDecreasingSumInDimensionIt) // in case of equality, prefer removing the globally sparsest slice
		{
		  bestDecreasingSumInDimensionIt = sumIt;
		}
	    }
	  bestIncreasingSumInDimensionIt = ++sumIt++;
	}
      // Compute bestDecreasingSumInDimensionIt and bestIncreasingSumInDimensionIt considering the sums until the one relating to the last present element
      for (const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end(); presentElementIdIt != presentElementIdEnd; ++presentElementIdIt)
	{
	  for (const vector<int>::const_iterator end = sumBegin + *presentElementIdIt; sumIt != end; ++sumIt)
	    {
	      if (*sumIt >= *bestIncreasingSumInDimensionIt) // in case of equality, prefer adding the globally densest slice
		{
		  bestIncreasingSumInDimensionIt = sumIt;
		}
	    }
	  if (*sumIt < *bestDecreasingSumInDimensionIt) // in case of equality, prefer removing the globally sparsest slice
	    {
	      bestDecreasingSumInDimensionIt = sumIt;
	    }
	  ++sumIt;
	}
      double g = membershipSum - *bestDecreasingSumInDimensionIt;
      g *= abs(g) / (area / dimensionIt->size() * (dimensionIt->size() - 1));
      if (g > bestG)
	{
	  bestG = g;
	  bestDimensionIt = dimensionIt;
	  bestSumIt = bestDecreasingSumInDimensionIt;
	  nextStep = erase;
	}
    }
  // Elements after the last present one
  for (const vector<int>::const_iterator sumEnd = sumsInDimension.end(); sumIt != sumEnd; ++sumIt)
    {
      if (*sumIt >= *bestIncreasingSumInDimensionIt) // in case of equality, prefer adding the globally densest slice
	{
	  bestIncreasingSumInDimensionIt = sumIt;
	}
    }
  double g = membershipSum + *bestIncreasingSumInDimensionIt;
  g *= abs(g) / (area / dimensionIt->size() * (dimensionIt->size() + 1));
  if (g > bestG)
    {
      bestG = g;
      bestDimensionIt = dimensionIt;
      bestSumIt = bestIncreasingSumInDimensionIt;
      nextStep = insert;
    }
}

void ModifiedPattern::considerDimensionForNextGrowingStep(const vector<vector<unsigned int>>::iterator dimensionIt, const vector<int>& sumsInDimension, const vector<unsigned int>& firstNonInitialAndSubsequentInitial)
{
  unsigned int elementId = firstNonInitialAndSubsequentInitial.front();
  vector<int>::const_iterator sumIt = sumsInDimension.begin() + elementId;
  if (sumIt != sumsInDimension.end())
    {
      // Some element can be added or removed
      int bestIncreasingSum;
      vector<int>::const_iterator bestIncreasingSumInDimensionIt;
      vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin() + elementId;
      if (presentElementIdIt == dimensionIt->end())
	{
	  // No element can be removed
	  bestIncreasingSumInDimensionIt = sumIt;
	  bestIncreasingSum = *sumIt;
	}
      else
	{
	  int bestDecreasingSum;
	  vector<int>::const_iterator bestDecreasingSumInDimensionIt;
	  if (*presentElementIdIt == elementId)
	    {
	      // First non initial element is present
	      ++presentElementIdIt;
	      bestDecreasingSumInDimensionIt = sumIt;
	      bestDecreasingSum = *sumIt;
	      bestIncreasingSumInDimensionIt = sumsInDimension.end();
	      bestIncreasingSum = numeric_limits<int>::min();
	    }
	  else
	    {
	      // First non initial element is absent
	      bestDecreasingSumInDimensionIt = sumsInDimension.end();
	      bestDecreasingSum = numeric_limits<int>::max();
	      bestIncreasingSumInDimensionIt = sumIt;
	      bestIncreasingSum = *sumIt;
	    }
	  // Elements until the last last initial one
	  vector<unsigned int>::const_iterator end = firstNonInitialAndSubsequentInitial.end();
	  for (vector<unsigned int>::const_iterator initialElementIt = firstNonInitialAndSubsequentInitial.begin(); ++initialElementIt != end; ++presentElementIdIt)
	    {
	      while (++elementId != *initialElementIt)
		{
		  if (elementId == *presentElementIdIt)
		    {
		      ++presentElementIdIt;
		      if (*++sumIt < bestDecreasingSum) // in case of equality, prefer removing the globally sparsest slice
			{
			  bestDecreasingSum = *sumIt;
			  bestDecreasingSumInDimensionIt = sumIt;
			}
		      continue;
		    }
		  if (*++sumIt >= bestIncreasingSum) // in case of equality, prefer adding the globally densest slice
		    {
		      bestIncreasingSum = *sumIt;
		      bestIncreasingSumInDimensionIt = sumIt;
		    }
		}
	      ++sumIt;
	    }
	  // Elements after the last initial one
	  for (end = dimensionIt->end(); presentElementIdIt != end; ++presentElementIdIt)
	    {
	      while (++elementId != *presentElementIdIt)
		{
		  if (*++sumIt >= bestIncreasingSum) // in case of equality, prefer adding the globally densest slice
		    {
		      bestIncreasingSum = *sumIt;
		      bestIncreasingSumInDimensionIt = sumIt;
		    }
		}
	      if (*++sumIt < bestDecreasingSum) // in case of equality, prefer removing the globally sparsest slice
		{
		  bestDecreasingSum = *sumIt;
		  bestDecreasingSumInDimensionIt = sumIt;
		}
	    }
	  if (bestDecreasingSum != numeric_limits<int>::max())
	    {
	      double g = membershipSum - bestDecreasingSum;
	      g *= abs(g) / (area / dimensionIt->size() * (dimensionIt->size() - 1));
	      if (g > bestG)
		{
		  bestG = g;
		  bestDimensionIt = dimensionIt;
		  bestSumIt = bestDecreasingSumInDimensionIt;
		  nextStep = erase;
		}
	    }
	}
      // Elements after the last present one
      for (const vector<int>::const_iterator sumEnd = sumsInDimension.end(); ++sumIt != sumEnd; )
	{
	  if (*sumIt >= bestIncreasingSum) // in case of equality, prefer adding the globally densest slice
	    {
	      bestIncreasingSum = *sumIt;
	      bestIncreasingSumInDimensionIt = sumIt;
	    }
	}
      if (bestIncreasingSum != numeric_limits<int>::min())
	{
	  double g = membershipSum + bestIncreasingSum;
	  g *= abs(g) / (area / dimensionIt->size() * (dimensionIt->size() + 1));
	  if (g > bestG)
	    {
	      bestG = g;
	      bestDimensionIt = dimensionIt;
	      bestSumIt = bestIncreasingSumInDimensionIt;
	      nextStep = insert;
	    }
	}
    }
}

bool ModifiedPattern::doStep()
{
  if (nextStep == stop)
    {
#ifdef DEBUG_MODIFY
      cout << "    g's local maximum reached\n";
#endif
      if (!isIntermediaryPatternsOutput)
	{
	  if (AbstractRoughTensor::isDirectOutput())
	    {
	      patternsToOutput.emplace_back(std::move(nSet), static_cast<float>(membershipSum) / area);
	      return false;
	    }
	  candidateVariables.emplace_back(std::move(nSet));
	}
      return false;
    }
  area /= bestDimensionIt->size();
#ifdef UPDATE_SUMS
  vector<unsigned int> singleElement {static_cast<unsigned int>(bestSumIt - sumsOnHyperplanes[bestDimensionIt - nSet.begin()].begin())};
  const unsigned int element = singleElement.front();
#else
  const unsigned int element = static_cast<unsigned int>(bestSumIt - sumsOnHyperplanes[bestDimensionIt - nSet.begin()].begin());
#endif
  if (nextStep == insert)
    {
#ifdef DEBUG_MODIFY
      cout << "    Adding slice for ";
      AbstractRoughTensor::printElement(bestDimensionIt - nSet.begin(), element, cout);
      cout << " gives ";
#endif
      bestDimensionIt->insert(lower_bound(bestDimensionIt->begin(), bestDimensionIt->end(), element), element);
#if REMEMBER != 0
      if (VisitedPatterns::visited(nSet))
	{
#ifdef DEBUG_MODIFY
	  roughTensor->printPattern(nSet, static_cast<float>(membershipSum + *bestSumIt) / (area * bestDimensionIt->size()), cout);
	  cout << ", which has already been reached: abort\n";
#endif
	  return false;
	}
#endif
#ifdef UPDATE_SUMS
      singleElement.swap(*bestDimensionIt);
      if (Trie::is01)
	{
	  tensor.increaseSumsOnHyperplanes(nSet.begin(), bestDimensionIt - nSet.begin(), sumsOnHyperplanes, area, AbstractRoughTensor::getUnit());
	}
      else
	{
	  tensor.increaseSumsOnHyperplanes(nSet.begin(), bestDimensionIt - nSet.begin(), sumsOnHyperplanes);
	}
      membershipSum += *bestSumIt;
#endif
    }
  else
    {
#ifdef DEBUG_MODIFY
      cout << "    Removing slice for ";
      AbstractRoughTensor::printElement(bestDimensionIt - nSet.begin(), element, cout);
      cout << " gives ";
#endif
      bestDimensionIt->erase(lower_bound(bestDimensionIt->begin(), bestDimensionIt->end(), element));
#if REMEMBER != 0
      if (VisitedPatterns::visited(nSet))
	{
#ifdef DEBUG_MODIFY
	  roughTensor->printPattern(nSet, static_cast<float>(membershipSum - *bestSumIt) / (area * bestDimensionIt->size()), cout);
	  cout << ", which has already been reached: abort\n";
#endif
	  return false;
	}
#endif
#ifdef UPDATE_SUMS
      singleElement.swap(*bestDimensionIt);
      if (Trie::is01)
	{
	  tensor.increaseSumsOnHyperplanes(nSet.begin(), bestDimensionIt - nSet.begin(), sumsOnHyperplanes, -area, -AbstractRoughTensor::getUnit()); // negating the last two arguments for a decrease
	}
      else
	{
	  tensor.decreaseSumsOnHyperplanes(nSet.begin(), bestDimensionIt - nSet.begin(), sumsOnHyperplanes);
	}
      membershipSum -= *bestSumIt;
#endif
    }
#ifdef UPDATE_SUMS
  area *= singleElement.size();
  singleElement.swap(*bestDimensionIt);
#else
  area *= bestDimensionIt->size();
  if (Trie::is01)
    {
      membershipSum = tensor.sumsOnPatternAndHyperplanes(nSet.begin(), sumsOnHyperplanes, area, AbstractRoughTensor::getUnit());
    }
  else
    {
      membershipSum = tensor.sumsOnPatternAndHyperplanes(nSet.begin(), sumsOnHyperplanes);
    }
#endif
#ifdef DEBUG_MODIFY
  roughTensor->printPattern(nSet, static_cast<float>(membershipSum) / area, cout);
  cout << " and g = " << static_cast<float>(bestG) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << '\n';
#endif
  return true;
}

void ModifiedPattern::setContext(const AbstractRoughTensor* roughTensorParam, const bool isIntermediaryPatternsOutputParam)
{
  isIntermediaryPatternsOutput = isIntermediaryPatternsOutputParam;
  roughTensor = roughTensorParam;
  tensor = roughTensor->getTensor();
#if REMEMBER != 0
  VisitedPatterns::init(AbstractRoughTensor::getCardinalities());
#endif
}

void ModifiedPattern::clearAndFree()
{
  tensor.clearAndFree();
#if REMEMBER != 0
  VisitedPatterns::clear();
#endif
}

#ifdef ASSERT
void ModifiedPattern::assertAreaAndSums()
{
  {
    // Asserting area
    vector<vector<unsigned int>>::const_iterator dimensionIt = nSet.begin();
    unsigned int actualArea = dimensionIt->size();
    ++dimensionIt;
    const vector<vector<unsigned int>>::const_iterator dimensionEnd = nSet.end();
    do
      {
	actualArea *= dimensionIt->size();
      }
    while (++dimensionIt != dimensionEnd);
    if (actualArea != area)
      {
	cerr << "area is " << actualArea << " and not " << area << ", as computed!\n";
      }
  }
  vector<vector<int>> actualSumsOnHyperplanes;
  {
    // Asserting membershipSum
    long long actualMembershipSum;
    const vector<unsigned int>& cardinalities = AbstractRoughTensor::getCardinalities();
    vector<unsigned int>::const_iterator cardinalityIt = cardinalities.begin();
    const vector<unsigned int>::const_iterator cardinalityEnd = cardinalities.end();
    actualSumsOnHyperplanes.reserve(cardinalityEnd - cardinalityIt);
    actualSumsOnHyperplanes.emplace_back(*cardinalityIt);
    ++cardinalityIt;
    do
      {
	actualSumsOnHyperplanes.emplace_back(*cardinalityIt);
      }
    while (++cardinalityIt != cardinalityEnd);
    if (Trie::is01)
      {
	actualMembershipSum = tensor.sumsOnPatternAndHyperplanes(nSet.begin(), actualSumsOnHyperplanes, area, AbstractRoughTensor::getUnit());
      }
    else
      {
	actualMembershipSum = tensor.sumsOnPatternAndHyperplanes(nSet.begin(), actualSumsOnHyperplanes);
      }
    if (actualMembershipSum != membershipSum)
      {
	cerr << "membership sum is " << static_cast<double>(actualMembershipSum) / AbstractRoughTensor::getUnit() << " and not " << static_cast<double>(membershipSum) / AbstractRoughTensor::getUnit() << ", as computed!\n";
      }
  }
  // Asserting sumsOnHyperplanes
  vector<vector<int>>::const_iterator actualSumsInDimensionIt = actualSumsOnHyperplanes.begin();
  for (const vector<int>& sumsInDimension : sumsOnHyperplanes)
    {
      for (pair<vector<int>::const_iterator, vector<int>::const_iterator> mismatchingSumIts = mismatch(sumsInDimension.begin(), sumsInDimension.end(), actualSumsInDimensionIt->begin()); mismatchingSumIts.first != sumsInDimension.end(); mismatchingSumIts = mismatch(++mismatchingSumIts.first, sumsInDimension.end(), ++mismatchingSumIts.second))
	{
	  cerr << "sum on ";
	  AbstractRoughTensor::printElement(actualSumsInDimensionIt - actualSumsOnHyperplanes.begin(), mismatchingSumIts.first - sumsInDimension.begin(), cerr);
	  cerr << " is " << static_cast<double>(*mismatchingSumIts.second) / AbstractRoughTensor::getUnit() << " and not " << static_cast<double>(*mismatchingSumIts.first) / AbstractRoughTensor::getUnit() << ", as computed!\n";
	}
      ++actualSumsInDimensionIt;
    }
}
#endif
