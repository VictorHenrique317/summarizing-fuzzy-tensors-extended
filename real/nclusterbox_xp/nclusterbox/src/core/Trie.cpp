// Copyright 2018-2022 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "Trie.h"

#include "SparseCrispTube.h"
#include "SparseFuzzyTube.h"

bool Trie::is01;

Trie::Trie(): hyperplanes()
{
}

Trie::Trie(Trie&& otherTrie): hyperplanes(std::move(otherTrie.hyperplanes))
{
}

Trie::Trie(const vector<unsigned int>::const_iterator cardinalityIt, const vector<unsigned int>::const_iterator cardinalityEnd): hyperplanes()
{
  unsigned int cardinality = *cardinalityIt;
  hyperplanes.reserve(cardinality);
  if (cardinalityIt + 2 == cardinalityEnd)
    {
      if (is01)
	{
	  do
	    {
	      hyperplanes.push_back(new SparseCrispTube());
	    }
	  while (--cardinality);
	  return;
	}
      do
	{
	  hyperplanes.push_back(new SparseFuzzyTube());
	}
      while (--cardinality);
      return;
    }
  const vector<unsigned int>::const_iterator nextCardinalityIt = cardinalityIt + 1;
  do
    {
      hyperplanes.push_back(new Trie(nextCardinalityIt, cardinalityEnd));
    }
  while (--cardinality);
}

Trie::Trie(vector<double>::const_iterator& membershipIt, const vector<unsigned int>::const_iterator cardinalityIt, const vector<unsigned int>::const_iterator cardinalityEnd): hyperplanes()
{
  unsigned int cardinality = *cardinalityIt;
  hyperplanes.reserve(cardinality);
  if (cardinalityIt + 2 == cardinalityEnd)
    {
      do
	{
	  hyperplanes.push_back(new DenseCrispTube(membershipIt));
	}
      while (--cardinality);
      return;
    }
  const vector<unsigned int>::const_iterator nextCardinalityIt = cardinalityIt + 1;
  do
    {
      hyperplanes.push_back(new Trie(membershipIt, nextCardinalityIt, cardinalityEnd));
    }
  while (--cardinality);
}

Trie::Trie(vector<double>::const_iterator& membershipIt, const int unit, const vector<unsigned int>::const_iterator cardinalityIt, const vector<unsigned int>::const_iterator cardinalityEnd): hyperplanes()
{
  unsigned int cardinality = *cardinalityIt;
  hyperplanes.reserve(cardinality);
  if (cardinalityIt + 2 == cardinalityEnd)
    {
      do
	{
	  hyperplanes.push_back(new DenseFuzzyTube(membershipIt, unit));
	}
      while (--cardinality);
      return;
    }
  const vector<unsigned int>::const_iterator nextCardinalityIt = cardinalityIt + 1;
  do
    {
      hyperplanes.push_back(new Trie(membershipIt, unit, nextCardinalityIt, cardinalityEnd));
    }
  while (--cardinality);
}

Trie::~Trie()
{
  for (AbstractData* hyperplane : hyperplanes)
    {
      delete hyperplane;
    }
}

Trie& Trie::operator=(Trie&& otherTrie)
{
  hyperplanes = std::move(otherTrie.hyperplanes);
  return *this;
}

void Trie::setTuple(const vector<unsigned int>::const_iterator idIt)
{
  AbstractData*& hyperplane = hyperplanes[*idIt];
  if (hyperplane->isFullSparseTube())
    {
      DenseCrispTube* newHyperplane = static_cast<SparseCrispTube*>(hyperplane)->getDenseRepresentation();
      delete hyperplane;
      hyperplane = newHyperplane;
    }
  hyperplane->setTuple(idIt + 1);
}

void Trie::setTuple(const vector<unsigned int>::const_iterator idIt, const int membership)
{
  AbstractData*& hyperplane = hyperplanes[*idIt];
  if (hyperplane->isFullSparseTube())
    {
      DenseFuzzyTube* newHyperplane = static_cast<SparseFuzzyTube*>(hyperplane)->getDenseRepresentation();
      delete hyperplane;
      hyperplane = newHyperplane;
    }
  hyperplane->setTuple(idIt + 1, membership);
}

void Trie::sortTubes()
{
  for (AbstractData* hyperplane : hyperplanes)
    {
      hyperplane->sortTubes();
    }
}

void Trie::clearAndFree()
{
  for (AbstractData* hyperplane : hyperplanes)
    {
      delete hyperplane;
    }
  hyperplanes.clear();
  hyperplanes.shrink_to_fit();
}

void Trie::sumOnPattern(const vector<vector<unsigned int>>::const_iterator dimensionIt, int& sum) const
{
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      hyperplanes[*idIt]->sumOnPattern(nextDimensionIt, sum);
    }
  while (++idIt != idEnd);
}

void Trie::minusSumOnPattern(const vector<vector<unsigned int>>::const_iterator dimensionIt, int& sum) const
{
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  const vector<unsigned int>::const_iterator idEnd = dimensionIt->end();
  vector<unsigned int>::const_iterator idIt = dimensionIt->begin();
  do
    {
      hyperplanes[*idIt]->minusSumOnPattern(nextDimensionIt, sum);
    }
  while (++idIt != idEnd);
}

long long Trie::membershipSum(const vector<vector<unsigned int>>& nSet) const
{
  long long sum = 0;
  const vector<vector<unsigned int>>::const_iterator secondDimensionIt = ++nSet.begin();
  const vector<unsigned int>::const_iterator idEnd = nSet.front().end();
  vector<unsigned int>::const_iterator idIt = nSet.front().begin();
  do
    {
      int sumOnSlice = 0;
      hyperplanes[*idIt]->sumOnPattern(secondDimensionIt, sumOnSlice);
      sum += sumOnSlice;
    }
  while (++idIt != idEnd);
  return sum;
}

long long Trie::nbOfPresentTuples(const vector<vector<unsigned int>>& nSet) const
{
  int nb = 0;
  sumOnPattern(nSet.begin(), nb);
  return nb;
}

long long Trie::sumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator nSetBegin, vector<vector<int>>& sumsOnHyperplanes) const
{
  const vector<vector<int>>::iterator sumsEnd = sumsOnHyperplanes.end();
  vector<vector<int>>::iterator sumsIt = sumsOnHyperplanes.begin();
  do
    {
      fill(sumsIt->begin(), sumsIt->end(), 0);
    }
  while (++sumsIt != sumsEnd);
  sumsIt = sumsOnHyperplanes.begin();
  vector<int>::iterator sumIt = sumsIt->begin();
  ++sumsIt;
  long long sumOnPattern = 0;
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = nSetBegin + 1;
  vector<AbstractData*>::const_iterator hyperplaneIt = hyperplanes.begin();
  {
    // Hyperplanes until the last present one
    const vector<AbstractData*>::const_iterator hyperplaneBegin = hyperplaneIt;
    const vector<unsigned int>::const_iterator presentElementIdEnd = nSetBegin->end();
    vector<unsigned int>::const_iterator presentElementIdIt = nSetBegin->begin();
    do
      {
	for (const vector<AbstractData*>::const_iterator end = hyperplaneBegin + *presentElementIdIt; hyperplaneIt != end; ++hyperplaneIt)
	  {
	    (*hyperplaneIt)->sumOnPattern(nextDimensionIt, *sumIt);
	    ++sumIt;
	  }
	const int sum = (*hyperplaneIt)->sumsOnPatternAndHyperplanes(nextDimensionIt, sumsIt);
	sumOnPattern += sum;
	*sumIt++ += sum;
	++hyperplaneIt;
      }
    while (++presentElementIdIt != presentElementIdEnd);
  }
  // Hyperplanes after the last present one
  for (const vector<AbstractData*>::const_iterator hyperplaneEnd = hyperplanes.end(); hyperplaneIt != hyperplaneEnd; ++hyperplaneIt)
    {
      (*hyperplaneIt)->sumOnPattern(nextDimensionIt, *sumIt);
      ++sumIt;
    }
  return sumOnPattern;
}

long long Trie::sumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator nSetBegin, vector<vector<int>>& sumsOnHyperplanes, const unsigned long long area, const int unit) const
{
  // is01
  const long long sumOnPattern = sumsOnPatternAndHyperplanes(nSetBegin, sumsOnHyperplanes) * unit + SparseCrispTube::getDefaultMembership() * area;
  const vector<vector<int>>::iterator sumsEnd = sumsOnHyperplanes.end();
  vector<vector<int>>::iterator sumsIt = sumsOnHyperplanes.begin();
  vector<vector<unsigned int>>::const_iterator dimensionIt = nSetBegin;
  do
    {
      const int shift = SparseCrispTube::getDefaultMembership() * static_cast<long long>(area / dimensionIt->size());
      const vector<int>::iterator sumEnd = sumsIt->end();
      vector<int>::iterator sumIt = sumsIt->begin();
      do
	{
	  *sumIt *= unit;
	  *sumIt += shift;
	}
      while (++sumIt != sumEnd);
      ++dimensionIt;
    }
  while (++sumsIt != sumsEnd);
  return sumOnPattern;
}

int Trie::sumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  int sumOnPattern = 0;
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  vector<int>::iterator sumIt = sumsIt->begin();
  vector<AbstractData*>::const_iterator hyperplaneIt = hyperplanes.begin();
  {
    // Hyperplanes until the last present one
    const vector<AbstractData*>::const_iterator hyperplaneBegin = hyperplaneIt;
    const vector<vector<int>>::iterator nextSumsIt = sumsIt + 1;
    const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end();
    vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
    do
      {
	for (const vector<AbstractData*>::const_iterator end = hyperplaneBegin + *presentElementIdIt; hyperplaneIt != end; ++hyperplaneIt)
	  {
	    (*hyperplaneIt)->sumOnPattern(nextDimensionIt, *sumIt);
	    ++sumIt;
	  }
	const int sum = (*hyperplaneIt)->sumsOnPatternAndHyperplanes(nextDimensionIt, nextSumsIt);
	sumOnPattern += sum;
	*sumIt++ += sum;
	++hyperplaneIt;
      }
    while (++presentElementIdIt != presentElementIdEnd);
  }
  // Hyperplanes after the last present one
  for (const vector<AbstractData*>::const_iterator hyperplaneEnd = hyperplanes.end(); hyperplaneIt != hyperplaneEnd; ++hyperplaneIt)
    {
      (*hyperplaneIt)->sumOnPattern(nextDimensionIt, *sumIt);
      ++sumIt;
    }
  return sumOnPattern;
}

int Trie::minusSumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  int sumOnPattern = 0;
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  vector<int>::iterator sumIt = sumsIt->begin();
  vector<AbstractData*>::const_iterator hyperplaneIt = hyperplanes.begin();
  {
    // Hyperplanes until the last present one
    const vector<AbstractData*>::const_iterator hyperplaneBegin = hyperplaneIt;
    const vector<vector<int>>::iterator nextSumsIt = sumsIt + 1;
    const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end();
    vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
    do
      {
	for (const vector<AbstractData*>::const_iterator end = hyperplaneBegin + *presentElementIdIt; hyperplaneIt != end; ++hyperplaneIt)
	  {
	    (*hyperplaneIt)->minusSumOnPattern(nextDimensionIt, *sumIt);
	    ++sumIt;
	  }
	const int sum = (*hyperplaneIt)->minusSumsOnPatternAndHyperplanes(nextDimensionIt, nextSumsIt);
	*sumIt++ -= sum;
	sumOnPattern += sum;
	++hyperplaneIt;
      }
    while (++presentElementIdIt != presentElementIdEnd);
  }
  // Hyperplanes after the last present one
  for (const vector<AbstractData*>::const_iterator hyperplaneEnd = hyperplanes.end(); hyperplaneIt != hyperplaneEnd; ++hyperplaneIt)
    {
      (*hyperplaneIt)->minusSumOnPattern(nextDimensionIt, *sumIt);
      ++sumIt;
    }
  return sumOnPattern;
}

void Trie::increaseSumsOnHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionBegin, const unsigned int increasedDimensionId, vector<vector<int>>& sums) const
{
  if (increasedDimensionId)
    {
      vector<int>& unchangedSums = sums[increasedDimensionId];
      vector<int> empty;
      empty.swap(unchangedSums);
      const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionBegin + 1;
      vector<int>::iterator sumIt = sums.front().begin();
      vector<AbstractData*>::const_iterator hyperplaneIt = hyperplanes.begin();
      {
	// Hyperplanes until the last present one
	const vector<AbstractData*>::const_iterator hyperplaneBegin = hyperplaneIt;
	const vector<vector<int>>::iterator nextSumsIt = ++sums.begin();
	const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionBegin->end();
	vector<unsigned int>::const_iterator presentElementIdIt = dimensionBegin->begin();
	do
	  {
	    for (const vector<AbstractData*>::const_iterator end = hyperplaneBegin + *presentElementIdIt; hyperplaneIt != end; ++hyperplaneIt)
	      {
		(*hyperplaneIt)->sumOnPattern(nextDimensionIt, *sumIt);
		++sumIt;
	      }
	    *sumIt++ += (*hyperplaneIt)->increaseSumsOnPatternAndHyperplanes(nextDimensionIt, nextSumsIt);
	    ++hyperplaneIt;
	  }
	while (++presentElementIdIt != presentElementIdEnd);
      }
      // Hyperplanes after the last present one
      for (const vector<AbstractData*>::const_iterator hyperplaneEnd = hyperplanes.end(); hyperplaneIt != hyperplaneEnd; ++hyperplaneIt)
	{
	  (*hyperplaneIt)->sumOnPattern(nextDimensionIt, *sumIt);
	  ++sumIt;
	}
      empty.swap(unchangedSums);
      return;
    }
  hyperplanes[dimensionBegin->front()]->increaseSumsOnHyperplanes(dimensionBegin + 1, ++sums.begin());
}

void Trie::increaseSumsOnHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  // sumsOnPatternAndHyperplanes without computing the sumOnPattern
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  vector<int>::iterator sumIt = sumsIt->begin();
  vector<AbstractData*>::const_iterator hyperplaneIt = hyperplanes.begin();
  {
    // Hyperplanes until the last present one
    const vector<AbstractData*>::const_iterator hyperplaneBegin = hyperplaneIt;
    const vector<vector<int>>::iterator nextSumsIt = sumsIt + 1;
    const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end();
    vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
    do
      {
	for (const vector<AbstractData*>::const_iterator end = hyperplaneBegin + *presentElementIdIt; hyperplaneIt != end; ++hyperplaneIt)
	  {
	    (*hyperplaneIt)->sumOnPattern(nextDimensionIt, *sumIt);
	    ++sumIt;
	  }
	*sumIt++ += (*hyperplaneIt)->sumsOnPatternAndHyperplanes(nextDimensionIt, nextSumsIt);
	++hyperplaneIt;
      }
    while (++presentElementIdIt != presentElementIdEnd);
  }
  // Hyperplanes after the last present one
  for (const vector<AbstractData*>::const_iterator hyperplaneEnd = hyperplanes.end(); hyperplaneIt != hyperplaneEnd; ++hyperplaneIt)
    {
      (*hyperplaneIt)->sumOnPattern(nextDimensionIt, *sumIt);
      ++sumIt;
    }
}

void Trie::decreaseSumsOnHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionBegin, const unsigned int decreasedDimensionId, vector<vector<int>>& sums) const
{
  // !is01
  if (decreasedDimensionId)
    {
      vector<int>& unchangedSums = sums[decreasedDimensionId];
      vector<int> empty;
      empty.swap(unchangedSums);
      const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionBegin + 1;
      vector<int>::iterator sumIt = sums.front().begin();
      vector<AbstractData*>::const_iterator hyperplaneIt = hyperplanes.begin();
      {
	// Hyperplanes until the last present one
	const vector<AbstractData*>::const_iterator hyperplaneBegin = hyperplaneIt;
	const vector<vector<int>>::iterator nextSumsIt = ++sums.begin();
	const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionBegin->end();
	vector<unsigned int>::const_iterator presentElementIdIt = dimensionBegin->begin();
	do
	  {
	    for (const vector<AbstractData*>::const_iterator end = hyperplaneBegin + *presentElementIdIt; hyperplaneIt != end; ++hyperplaneIt)
	      {
		(*hyperplaneIt)->minusSumOnPattern(nextDimensionIt, *sumIt);
		++sumIt;
	      }
	    *sumIt++ -= (*hyperplaneIt)->decreaseSumsOnPatternAndHyperplanes(nextDimensionIt, nextSumsIt);
	    ++hyperplaneIt;
	  }
	while (++presentElementIdIt != presentElementIdEnd);
      }
      // Hyperplanes after the last present one
      for (const vector<AbstractData*>::const_iterator hyperplaneEnd = hyperplanes.end(); hyperplaneIt != hyperplaneEnd; ++hyperplaneIt)
	{
	  (*hyperplaneIt)->minusSumOnPattern(nextDimensionIt, *sumIt);
	  ++sumIt;
	}
      empty.swap(unchangedSums);
      return;
    }
  hyperplanes[dimensionBegin->front()]->decreaseSumsOnHyperplanes(dimensionBegin + 1, ++sums.begin());
}

void Trie::decreaseSumsOnHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  // minusSumsOnPatternAndHyperplanes without computing the sumOnPattern
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  vector<int>::iterator sumIt = sumsIt->begin();
  vector<AbstractData*>::const_iterator hyperplaneIt = hyperplanes.begin();
  {
    // Hyperplanes until the last present one
    const vector<AbstractData*>::const_iterator hyperplaneBegin = hyperplaneIt;
    const vector<vector<int>>::iterator nextSumsIt = sumsIt + 1;
    const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end();
    vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
    do
      {
	for (const vector<AbstractData*>::const_iterator end = hyperplaneBegin + *presentElementIdIt; hyperplaneIt != end; ++hyperplaneIt)
	  {
	    (*hyperplaneIt)->minusSumOnPattern(nextDimensionIt, *sumIt);
	    ++sumIt;
	  }
	*sumIt++ -= (*hyperplaneIt)->minusSumsOnPatternAndHyperplanes(nextDimensionIt, nextSumsIt);
	++hyperplaneIt;
      }
    while (++presentElementIdIt != presentElementIdEnd);
  }
  // Hyperplanes after the last present one
  for (const vector<AbstractData*>::const_iterator hyperplaneEnd = hyperplanes.end(); hyperplaneIt != hyperplaneEnd; ++hyperplaneIt)
    {
      (*hyperplaneIt)->minusSumOnPattern(nextDimensionIt, *sumIt);
      ++sumIt;
    }
}

int Trie::increaseSumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  if (sumsIt->empty())
    {
      return hyperplanes[dimensionIt->front()]->sumsOnPatternAndHyperplanes(dimensionIt + 1, sumsIt + 1);
    }
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  int sumOnPattern = 0;
  vector<int>::iterator sumIt = sumsIt->begin();
  vector<AbstractData*>::const_iterator hyperplaneIt = hyperplanes.begin();
  {
    // Hyperplanes until the last present one
    const vector<AbstractData*>::const_iterator hyperplaneBegin = hyperplaneIt;
    const vector<vector<int>>::iterator nextSumsIt = sumsIt + 1;
    const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end();
    vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
    do
      {
	for (const vector<AbstractData*>::const_iterator end = hyperplaneBegin + *presentElementIdIt; hyperplaneIt != end; ++hyperplaneIt)
	  {
	    (*hyperplaneIt)->sumOnPattern(nextDimensionIt, *sumIt);
	    ++sumIt;
	  }
	const int sum = (*hyperplaneIt)->increaseSumsOnPatternAndHyperplanes(nextDimensionIt, nextSumsIt);
	*sumIt++ += sum;
	sumOnPattern += sum;
	++hyperplaneIt;
      }
    while (++presentElementIdIt != presentElementIdEnd);
  }
  // Hyperplanes after the last present one
  for (const vector<AbstractData*>::const_iterator hyperplaneEnd = hyperplanes.end(); hyperplaneIt != hyperplaneEnd; ++hyperplaneIt)
    {
      (*hyperplaneIt)->sumOnPattern(nextDimensionIt, *sumIt);
      ++sumIt;
    }
  return sumOnPattern;
}

int Trie::decreaseSumsOnPatternAndHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionIt, const vector<vector<int>>::iterator sumsIt) const
{
  if (sumsIt->empty())
    {
      return hyperplanes[dimensionIt->front()]->minusSumsOnPatternAndHyperplanes(dimensionIt + 1, sumsIt + 1);
    }
  const vector<vector<unsigned int>>::const_iterator nextDimensionIt = dimensionIt + 1;
  int sumOnPattern = 0;
  vector<int>::iterator sumIt = sumsIt->begin();
  vector<AbstractData*>::const_iterator hyperplaneIt = hyperplanes.begin();
  {
    // Hyperplanes until the last present one
    const vector<AbstractData*>::const_iterator hyperplaneBegin = hyperplaneIt;
    const vector<vector<int>>::iterator nextSumsIt = sumsIt + 1;
    const vector<unsigned int>::const_iterator presentElementIdEnd = dimensionIt->end();
    vector<unsigned int>::const_iterator presentElementIdIt = dimensionIt->begin();
    do
      {
	for (const vector<AbstractData*>::const_iterator end = hyperplaneBegin + *presentElementIdIt; hyperplaneIt != end; ++hyperplaneIt)
	  {
	    (*hyperplaneIt)->minusSumOnPattern(nextDimensionIt, *sumIt);
	    ++sumIt;
	  }
	const int sum = (*hyperplaneIt)->decreaseSumsOnPatternAndHyperplanes(nextDimensionIt, nextSumsIt);
	*sumIt++ -= sum;
	sumOnPattern += sum;
	++hyperplaneIt;
      }
    while (++presentElementIdIt != presentElementIdEnd);
  }
  // Hyperplanes after the last present one
  for (const vector<AbstractData*>::const_iterator hyperplaneEnd = hyperplanes.end(); hyperplaneIt != hyperplaneEnd; ++hyperplaneIt)
    {
      (*hyperplaneIt)->minusSumOnPattern(nextDimensionIt, *sumIt);
      ++sumIt;
    }
  return sumOnPattern;
}

void Trie::increaseSumsOnHyperplanes(const vector<vector<unsigned int>>::const_iterator dimensionBegin, const unsigned int increasedDimensionId, vector<vector<int>>& sums, const int area, const int unit) const
{
  // is01
  vector<vector<int>> increases;
  increases.reserve(sums.size());
  vector<vector<int>>::iterator sumsIt = sums.begin();
  for (const vector<vector<int>>::const_iterator end = sums.begin() + increasedDimensionId; sumsIt != end; ++sumsIt)
    {
      increases.emplace_back(sumsIt->size());
    }
  increases.emplace_back();
  for (const vector<vector<int>>::const_iterator sumsEnd = sums.end(); ++sumsIt != sumsEnd; )
    {
      increases.emplace_back(sumsIt->size());
    }
  increaseSumsOnHyperplanes(dimensionBegin, increasedDimensionId, increases);
  sumsIt = sums.begin();
  const int defaultNSetMembership = SparseCrispTube::getDefaultMembership() * area;
  vector<vector<unsigned int>>::const_iterator dimensionIt = dimensionBegin;
  vector<vector<int>>::const_iterator increasesIt = increases.begin();
  for (; !increasesIt->empty(); ++increasesIt)
    {
      const int shift = defaultNSetMembership / static_cast<int>(dimensionIt->size());
      vector<int>::iterator sumIt = sumsIt->begin();
      for (const int increase : *increasesIt)
	{
	  *sumIt++ += increase * unit + shift;
	}
      ++sumsIt;
      ++dimensionIt;
    }
  for (const vector<vector<int>>::const_iterator increasesEnd = increases.end(); ++increasesIt != increasesEnd; )
    {
      const int shift = defaultNSetMembership / static_cast<int>((++dimensionIt)->size());
      vector<int>::iterator sumIt = (++sumsIt)->begin();
      for (const int increase : *increasesIt)
	{
	  *sumIt++ += increase * unit + shift;
	}
    }
}
