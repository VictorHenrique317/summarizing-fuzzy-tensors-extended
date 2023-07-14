// Copyright 2018-2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef CANDIDATE_VARIABLE_H_
#define CANDIDATE_VARIABLE_H_

#include <vector>

using namespace std;

class CandidateVariable
{
 public:
  CandidateVariable(const CandidateVariable& otherCandidateVariable) = delete;
  CandidateVariable(CandidateVariable&& otherCandidateVariable);
  CandidateVariable(vector<vector<unsigned int>>& nSet, const int density);

  CandidateVariable& operator=(CandidateVariable&& otherCandidateVariable);

  const vector<vector<unsigned int>>& getNSet() const;
  int getDensity() const;
  long long getRSSVariation() const;

  bool overlaps(const vector<vector<unsigned int>>::const_iterator otherDimensionBegin) const;
  vector<vector<unsigned int>> inter(const vector<vector<unsigned int>>::const_iterator otherDimensionBegin) const;

  void addToRSSVariation(const long long delta);
  void reset();

private:
  vector<vector<unsigned int>> nSet;
  int density;
  long long rssVariation;

  void multiplyRSSVariationByArea();

  static bool emptyIntersection(const vector<unsigned int>& dimension, vector<unsigned int>::const_iterator otherDimensionElementIt);
};

#endif	/* CANDIDATE_VARIABLE_H_ */
