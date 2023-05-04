// Copyright 2022,2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "RankPatterns.h"

#include <thread>
#include <iostream>
#include <iomanip>
#include <boost/lexical_cast.hpp>

TrieWithPrediction RankPatterns::tensor;
vector<CandidateVariable> RankPatterns::candidates;
vector<CandidateVariable>::iterator RankPatterns::selectedIt;
vector<CandidateVariable>::iterator RankPatterns::selectionEnd;
vector<CandidateVariable>::iterator RankPatterns::candidateBegin;
vector<CandidateVariable>::iterator RankPatterns::candidateEnd;
long long RankPatterns::rssVariation;
long long RankPatterns::maxRSSVariation;

#ifdef DEBUG_SELECT
AbstractRoughTensor* RankPatterns::roughTensorForDebug;
#endif
#ifdef DETAILED_TIME
steady_clock::time_point RankPatterns::startingPoint;
#endif

void RankPatterns::printProgressionOnSTDIN(const float stepInSeconds)
{
  const chrono::duration<float> duration(stepInSeconds);
  unsigned int nbOfDigitsInNbOfCandidates = log10(candidateEnd - candidateBegin);
  ++nbOfDigitsInNbOfCandidates;
  while (candidateEnd != candidateBegin)
    {
      cout << "\rSelecting patterns: " << right << setw(nbOfDigitsInNbOfCandidates) << candidateEnd - candidateBegin << " candidates remain" << flush;
      this_thread::sleep_for(duration);
    }
}

void RankPatterns::updateCandidate(const CandidateVariable& lastSelectedPattern, CandidateVariable& candidate)
{
  const vector<vector<unsigned int>> intersection = lastSelectedPattern.inter(candidate.getNSet().begin());
  if (!intersection.empty())
    {
      candidate.addToRSSVariation(tensor.deltaOfRSSVariationAdding(intersection, min(lastSelectedPattern.getDensity(), candidate.getDensity())));
    }
}

void RankPatterns::selectForAddition(const CandidateVariable& lastSelectedPattern)	// selectionEnd points to the selected pattern; the patterns after selectionEnd and before candidatesEnd are the non-selected candidates to update; selectedIt will point to one of them (the next one to select, unless rssVariation == 0, which indicates the selection must end)
{
  rssVariation = maxRSSVariation;
  for (vector<CandidateVariable>::iterator candidateIt = ++selectionEnd; candidateIt != candidateEnd; ++candidateIt)
    {
      updateCandidate(lastSelectedPattern, *candidateIt);
      // Figure out if *candidateIt is best candidate to select so far
      if (candidateIt->getRSSVariation() < rssVariation)
	{
	  rssVariation = candidateIt->getRSSVariation();
	  selectedIt = candidateIt;
	}
    }
}

void RankPatterns::updatePreviouslySelected(const CandidateVariable& lastSelectedPattern, CandidateVariable& previouslySelected)
{
  const vector<vector<unsigned int>> intersection = lastSelectedPattern.inter(previouslySelected.getNSet().begin());
  if (!intersection.empty())
    {
      if (lastSelectedPattern.getDensity() < previouslySelected.getDensity())
	{
	  previouslySelected.addToRSSVariation(tensor.deltaOfRSSVariationRemovingIfSparserSelected(intersection, previouslySelected.getDensity(), lastSelectedPattern.getDensity()));
	  return;
	}
      previouslySelected.addToRSSVariation(tensor.deltaOfRSSVariationRemovingIfDenserSelected(intersection, previouslySelected.getDensity()));
    }
}

void RankPatterns::selectForRemoval(const CandidateVariable& lastSelectedPattern)
{
  rssVariation = maxRSSVariation;
  for (vector<CandidateVariable>::iterator previouslySelectedIt = candidateBegin; previouslySelectedIt != selectionEnd; ++previouslySelectedIt)
    {
      updatePreviouslySelected(lastSelectedPattern, *previouslySelectedIt);
      // Figure out if *previouslySelectedIt is best candidate to select so far
      if (previouslySelectedIt->getRSSVariation() >= rssVariation)
	{
	  rssVariation = previouslySelectedIt->getRSSVariation();
	  selectedIt = previouslySelectedIt;
	}
    }
}

void RankPatterns::reselectOne() // Reselect *selectedIt only updating the rest since the next pattern to reselect for addition is known: selectedIt + 1
{
#ifdef DEBUG_SELECT
  cout << "Reselecting ";
  roughTensorForDebug->printPattern(selectedIt->getNSet(), selectedIt->getDensity(), cout);
  cout << " to decrease the RSS the most, by " << static_cast<double>(-selectedIt->getRSSVariation()) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << '\n';
#endif
  vector<CandidateVariable>::iterator patternIt = candidateBegin;
  for (; patternIt != selectedIt; ++patternIt)
    {
      updatePreviouslySelected(*selectedIt, *patternIt);
    }
  while (++patternIt != candidateEnd)
    {
      updateCandidate(*selectedIt, *patternIt);
    }
}

void RankPatterns::reselect()
{
  for (selectedIt = candidateBegin; selectedIt != candidateEnd; ++selectedIt) // using selectedIt to browse the non-selected candidates!
    {
      selectedIt->reset();
    }
  selectedIt = candidateBegin;
  --selectionEnd;
  if (selectionEnd == selectedIt)
    {
#ifdef DEBUG_SELECT
      cout << "Single reselection: ";
      roughTensorForDebug->printPattern(selectedIt->getNSet(), selectedIt->getDensity(), cout);
      cout << " to decrease the RSS the most, by " << static_cast<double>(-selectedIt->getRSSVariation()) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << '\n';
#endif
      const CandidateVariable& selectedPattern = *selectionEnd;
      selectForAddition(selectedPattern);
      tensor.addFirstPatternToModel(selectedPattern.getNSet(), selectedPattern.getDensity());
      return;
    }
  // More than one pattern to reselect
  reselectOne();
  tensor.addFirstPatternToModel(selectedIt->getNSet(), selectedIt->getDensity());
  while (++selectedIt != selectionEnd)
    {
      reselectOne();
      tensor.addPatternToModel(selectedIt->getNSet(), selectedIt->getDensity());
    }
#ifdef DEBUG_SELECT
  cout << "Last reselection: ";
  roughTensorForDebug->printPattern(selectedIt->getNSet(), selectedIt->getDensity(), cout);
  cout << " to decrease the RSS the most, by " << static_cast<double>(-selectedIt->getRSSVariation()) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << '\n';
#endif
  const CandidateVariable& selectedPattern = *selectionEnd;
  selectForAddition(selectedPattern);
  tensor.addPatternToModel(selectedPattern.getNSet(), selectedPattern.getDensity());
}

void RankPatterns::output(const AbstractRoughTensor* roughTensor, const bool isRSSPrinted, const vector<double>& rssHistory, const float verboseStep)
{
  if (verboseStep)
    {
      if (verboseStep > 0)
	{
	  const unsigned int nbOfDigitsInNbOfCandidates = log10(candidateEnd - candidateBegin);
	  candidateEnd = candidateBegin; // for printProgressionOnSTDIN to terminate
	  cout << left << setw(nbOfDigitsInNbOfCandidates + 40) << "\rSelecting patterns: " + boost::lexical_cast<string>(selectionEnd - candidateBegin) + " patterns selected." << '\n';
	}
      else
	{
	  cout << "\rSelecting patterns: " << selectionEnd - candidateBegin << " patterns selected.\n";
	}
    }
  if (isRSSPrinted)
    {
      vector<double>::const_iterator rssIt = rssHistory.begin();
      for (selectedIt = candidateBegin; selectedIt != selectionEnd; ++selectedIt)
	{
	  roughTensor->output(selectedIt->getNSet(), selectedIt->getDensity(), *rssIt++);
	}
    }
  else
    {
      for (selectedIt = candidateBegin; selectedIt != selectionEnd; ++selectedIt)
	{
	  roughTensor->output(selectedIt->getNSet(), selectedIt->getDensity());
	}
    }
#ifdef NB_OF_PATTERNS
#ifdef GNUPLOT
  cout << '\t' << selectionEnd - candidateBegin;
#else
  cout << "Nb of selected patterns: " << selectionEnd - candidateBegin << '\n';
#endif
#endif
#ifdef DETAILED_TIME
#ifdef GNUPLOT
  cout << '\t' << duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#else
  cout << "Selection time: " << duration_cast<duration<double>>(steady_clock::now() - startingPoint).count() << "s\n";
#endif
#endif
}

void RankPatterns::rank(const unsigned int nbOfCandidateVariablesHavingAllElements, AbstractRoughTensor* roughTensor, const float verboseStep, const int maxSelectionSize, const SelectionCriterion selectionCriterion, const bool isRSSPrinted)
{
#ifdef NB_OF_PATTERNS
#ifdef GNUPLOT
  cout << '\t' << AbstractRoughTensor::getCandidateVariables().size();
#else
  cout << "Nb of patterns candidates for selection: " << AbstractRoughTensor::getCandidateVariables().size() << '\n';
#endif
#endif
  // Set RSS multiplier depending on the criterion (note that AbstractRoughTensor::getArea must be called before the projection of the tensor)
  double rssMultiplier;
  if (selectionCriterion == bic)
    {
      const long long area = AbstractRoughTensor::getArea();
      rssMultiplier = exp(-log(area) / area) - 1;
    }
  else
    {
      if (selectionCriterion == rss)
	{
	  rssMultiplier = 0;
	}
      else
	{
	  // AIC
	  rssMultiplier = exp(-2. / AbstractRoughTensor::getArea()) - 1;
	}
    }
  // Set tensor
#ifdef DETAILED_TIME
  startingPoint = steady_clock::now();
#endif
  if (verboseStep)
    {
      cout << "Reducing fuzzy tensor to elements in patterns ... " << flush;
    }
  tensor = roughTensor->projectTensor(nbOfCandidateVariablesHavingAllElements);
  if (verboseStep)
    {
      cout << "\rReducing fuzzy tensor to elements in patterns: " << AbstractRoughTensor::getArea() << " tuples.\n";
    }
#ifdef DETAILED_TIME
#ifdef GNUPLOT
  cout << '\t' << duration_cast<duration<double>>(steady_clock::now() - startingPoint).count();
#else
  cout << "Tensor reduction time: " << duration_cast<duration<double>>(steady_clock::now() - startingPoint).count() << "s\n";
  startingPoint = steady_clock::now();
#endif
#endif
  if (verboseStep)
    {
      cout << "Selecting patterns ... " << flush;
    }
  // Construct the candidates and search the first candidate to select
  maxRSSVariation = rssMultiplier * AbstractRoughTensor::getNullModelRSS();
  rssVariation = maxRSSVariation;
  {
    vector<vector<vector<unsigned int>>>& raw = AbstractRoughTensor::getCandidateVariables();
    candidates.reserve(raw.size());
    for (vector<vector<unsigned int>>& pattern : raw)
      {
	const int density = tensor.density(pattern);
	if (density > 0)
	  {
	    candidates.emplace_back(pattern, density);
	    if (candidates.back().getRSSVariation() < rssVariation)
	      {
		rssVariation = candidates.back().getRSSVariation();
		selectedIt = --candidates.end();
	      }
	  }
      }
    raw.clear();
    raw.shrink_to_fit();
  }
  candidateBegin = candidates.begin();
  selectionEnd = candidateBegin;
  candidateEnd = candidates.end();
  vector<double> rssHistory;
  if (rssVariation < maxRSSVariation)
    {
      if (verboseStep > 0)
	{
	  thread(printProgressionOnSTDIN, verboseStep).detach();
	}
#ifdef DEBUG_SELECT
      cout << "Selecting first ";
      roughTensor->printPattern(selectedIt->getNSet(), selectedIt->getDensity(), cout);
      cout << " to decrease the RSS the most, by " << static_cast<double>(-rssVariation) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << " > " << static_cast<double>(-maxRSSVariation) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << ", the minimum allowed\n";
      roughTensorForDebug = roughTensor;
#endif
      rssHistory.reserve(min(static_cast<int>(candidateEnd - selectionEnd), maxSelectionSize));
      rssHistory.push_back(AbstractRoughTensor::getNullModelRSS() + rssVariation);
      maxRSSVariation = rssMultiplier * rssHistory.back();
      swap(*selectionEnd, *selectedIt);
      {
	// Add *selectedIt to selection
	const CandidateVariable& selectedPattern = *selectionEnd;
	selectForAddition(selectedPattern);
	tensor.addFirstPatternToModel(selectedPattern.getNSet(), selectedPattern.getDensity());
      }
      while (rssVariation < maxRSSVariation)
	{
	  {
	    // Search previously selected pattern that would most decrease the RSS if eliminated after *selectedIt added
	    const CandidateVariable& selectedPattern = *selectedIt;
#ifdef DEBUG_SELECT
	    cout << "Selecting ";
	    roughTensor->printPattern(selectedPattern.getNSet(), selectedPattern.getDensity(), cout);
	    cout << " to decrease the RSS the most, by " << static_cast<double>(-rssVariation) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << " > " << static_cast<double>(-maxRSSVariation) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << ", the minimum allowed\n";
#endif
	    maxRSSVariation = rssMultiplier * (rssHistory.back() + rssVariation);
	    selectForRemoval(selectedPattern);
	  }
	  if (selectedIt->getRSSVariation() < maxRSSVariation)
	    {
	      // No pattern to eliminate
	      if (selectionEnd - candidateBegin == maxSelectionSize)
		{
		  // Selecting *selectedIt would make the selection too large
		  output(roughTensor, isRSSPrinted, rssHistory, verboseStep);
		  return;
		}
	      // Selecting *selectedIt
	      rssHistory.push_back(rssHistory.back() + selectedIt->getRSSVariation());
	      maxRSSVariation = rssMultiplier * rssHistory.back();
	      swap(*selectionEnd, *selectedIt);
	      const CandidateVariable& selectedPattern = *selectionEnd;
	      selectForAddition(selectedPattern);
	      tensor.addPatternToModel(selectedPattern.getNSet(), selectedPattern.getDensity());
	      continue;
	    }
	  // *selectedIt to eliminate: remove it and all the patterns that were subsequently selected and reselect what remains, if anything
#ifdef DEBUG_SELECT
	  cout << "Eliminating ";
	  roughTensor->printPattern(selectedIt->getNSet(), selectedIt->getDensity(), cout);
#endif
	  {
	    // Reset all predictions (no need to reset those for tuples uncovered by selection)
	    vector<CandidateVariable>::iterator patternIt = candidateBegin;
	    rssHistory.resize(selectedIt - patternIt);
	    do
	      {
		tensor.reset(patternIt->getNSet());
	      }
	    while (++patternIt != selectionEnd);
	  }
	  // Truncate selection at selectedIt, which is eliminated
	  selectionEnd = selectedIt;
	  *selectionEnd = std::move(*--candidateEnd);
	  if (selectionEnd != candidateBegin)
	    {
	      // Some pattern(s) to reselect
#ifdef DEBUG_SELECT
	      cout << " because selecting it now would decrease the RSS by " << static_cast<double>(-rssVariation) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << " < " << static_cast<double>(-maxRSSVariation) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << ", the minimum allowed -> reselect the patterns selected before it and go on without it\n";
#endif
	      maxRSSVariation = rssMultiplier * rssHistory.back();
	      reselect();
	      continue;
	    }
	  // No pattern to reselect: restart from scratch, selecting a first pattern
#ifdef DEBUG_SELECT
	  cout << ", the first selected pattern, because selecting it now would decrease the RSS by " << static_cast<double>(-rssVariation) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << " < " << static_cast<double>(-maxRSSVariation) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << ", the minimum allowed -> restart from scratch without it\nSelecting first ";
#endif
	  rssVariation = rssMultiplier * AbstractRoughTensor::getNullModelRSS();
	  vector<CandidateVariable>::iterator patternIt = selectionEnd;
	  do
	    {
	      patternIt->reset();
	      if (patternIt->getRSSVariation() < rssVariation)
		{
		  rssVariation = patternIt->getRSSVariation();
		  selectedIt = patternIt;
		}
	    }
	  while (++patternIt != candidateEnd);
#ifdef DEBUG_SELECT
	  roughTensor->printPattern(selectedIt->getNSet(), selectedIt->getDensity(), cout);
	  cout << " to decrease the RSS the most, by " << static_cast<double>(-rssVariation) / AbstractRoughTensor::getUnit() / AbstractRoughTensor::getUnit() << '\n';
	  roughTensorForDebug = roughTensor;
#endif
	  rssHistory.push_back(AbstractRoughTensor::getNullModelRSS() + rssVariation);
	  maxRSSVariation = rssMultiplier * rssHistory.back();
	  swap(*selectionEnd, *selectedIt);
	  const CandidateVariable& selectedPattern = *selectionEnd;
	  selectForAddition(selectedPattern);
	  tensor.addFirstPatternToModel(selectedPattern.getNSet(), selectedPattern.getDensity());
	}
    }
  output(roughTensor, isRSSPrinted, rssHistory, verboseStep);
}
