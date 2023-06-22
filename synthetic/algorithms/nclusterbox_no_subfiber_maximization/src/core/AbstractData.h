// Copyright 2018-2022 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef ABSTRACT_DATA_H_
#define ABSTRACT_DATA_H_

#include <vector>

using namespace std;

class AbstractData
{
 public:
  virtual ~AbstractData();

  virtual bool isFullSparseTube() const;
  virtual void setTuple(const vector<unsigned int>::const_iterator idIt);
  virtual void setTuple(const vector<unsigned int>::const_iterator idIt, const int membership);
  virtual void sortTubes();

  virtual void sumOnPattern(const vector<vector<unsigned int>>::const_iterator dimensionIt, int& sum) const = 0;
  virtual void minusSumOnPattern(const vector<vector<unsigned int>>::const_iterator dimensionIt, int& sum) const;
  virtual int sumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const = 0;
  virtual int minusSumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const;
  virtual void increaseSumsOnHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const = 0;
  virtual void decreaseSumsOnHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const;
  virtual int increaseSumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const = 0;
  virtual int decreaseSumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const;
};

#endif /*ABSTRACT_DATA_H_*/
