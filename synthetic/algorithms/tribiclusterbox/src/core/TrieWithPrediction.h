// Copyright 2018-2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef TRIE_WITH_PREDICTION_H_
#define TRIE_WITH_PREDICTION_H_

#include "AbstractDataWithPrediction.h"

class TrieWithPrediction final : public AbstractDataWithPrediction
{
 public:
  TrieWithPrediction();
  TrieWithPrediction(TrieWithPrediction&& otherTrieWithPrediction);
  TrieWithPrediction(const vector<unsigned int>::const_iterator cardinalityIt, const vector<unsigned int>::const_iterator cardinalityEnd);
  TrieWithPrediction(vector<double>::const_iterator& membershipIt, const unsigned int unit, const vector<unsigned int>::const_iterator cardinalityIt, const vector<unsigned int>::const_iterator cardinalityEnd);

  ~TrieWithPrediction();

  TrieWithPrediction& operator=(TrieWithPrediction&& otherTrieWithPrediction);

  void setTuple(const vector<unsigned int>::const_iterator idIt, const int membership);

  int density(const vector<vector<unsigned int>>& nSet) const;
  void membershipSumOnSlice(const vector<vector<unsigned int>>::const_iterator dimensionIt, int& sum) const;
  void addFirstPatternToModel(const vector<vector<unsigned int>>& tuples, const int density);
  void addFirstPatternToModel(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int density);
  void addPatternToModel(const vector<vector<unsigned int>>& tuples, const int density);
  void addPatternToModel(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int density);
  long long deltaOfRSSVariationAdding(const vector<vector<unsigned int>>& tuples, const int minDensityOfSelectedAndUpdated) const;
  void deltaOfRSSVariationAdding(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int minDensityOfSelectedAndUpdated, long long& delta) const;
  long long deltaOfRSSVariationRemovingIfSparserSelected(const vector<vector<unsigned int>> tuples, const int updatedDensity, const int selectedDensity) const;
  void deltaOfRSSVariationRemovingIfSparserSelected(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int updatedDensity, const int selectedDensity, long long& delta) const;
  long long deltaOfRSSVariationRemovingIfDenserSelected(const vector<vector<unsigned int>> tuples, const int updatedDensity) const;
  void deltaOfRSSVariationRemovingIfDenserSelected(const vector<vector<unsigned int>>::const_iterator dimensionIt, const int updatedDensity, long long& delta) const;
  void reset(const vector<vector<unsigned int>>& tuples);
  void reset(const vector<vector<unsigned int>>::const_iterator dimensionIt);

 private:
  vector<AbstractDataWithPrediction*> hyperplanes;
};

#endif /*TRIE_WITH_PREDICTION_H_*/
