// Copyright 2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "BestPatterns.h"

#include <algorithm>

vector<pair<vector<vector<unsigned int>>, double>> BestPatterns::patternsWithG;
unsigned int BestPatterns::nbOfFreeSlots;

void BestPatterns::setMaxNbOfPatterns(const unsigned int maxNbOfPatterns)
{
  patternsWithG.reserve(maxNbOfPatterns);
  nbOfFreeSlots = maxNbOfPatterns;
}

void BestPatterns::push(pair<vector<vector<unsigned int>>, double>&& patternWithG)
{
  if (nbOfFreeSlots)
    {
      patternsWithG.emplace_back(patternWithG);
      if (--nbOfFreeSlots)
	{
	  return;
	}
      make_heap(patternsWithG.begin(), patternsWithG.end(), [](const pair<vector<vector<unsigned int>>, double>& pattern1WithG, const pair<vector<vector<unsigned int>>, double>& pattern2WithG) { return pattern1WithG.second > pattern2WithG.second; });
      pop_heap(patternsWithG.begin(), patternsWithG.end(), [](const pair<vector<vector<unsigned int>>, double>& pattern1WithG, const pair<vector<vector<unsigned int>>, double>& pattern2WithG) { return pattern1WithG.second > pattern2WithG.second; });
      return;
    }
  if (patternWithG.second > patternsWithG.back().second)
    {
      patternsWithG.pop_back();
      patternsWithG.emplace_back(patternWithG);
      pop_heap(patternsWithG.begin(), patternsWithG.end(), [](const pair<vector<vector<unsigned int>>, double>& pattern1WithG, const pair<vector<vector<unsigned int>>, double>& pattern2WithG) { return pattern1WithG.second > pattern2WithG.second; });
    }
}

vector<pair<vector<vector<unsigned int>>, double>>& BestPatterns::get()
{
  return patternsWithG;
}
