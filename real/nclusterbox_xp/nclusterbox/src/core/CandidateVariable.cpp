// Copyright 2018-2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "CandidateVariable.h"

#include <algorithm>

CandidateVariable::CandidateVariable(CandidateVariable&& otherCandidateVariable): nSet(std::move(otherCandidateVariable.nSet)), density(otherCandidateVariable.density), rssVariation(otherCandidateVariable.rssVariation)
{
}

CandidateVariable::CandidateVariable(vector<vector<unsigned int>>& nSetParam, const int densityParam): nSet(std::move(nSetParam)), density(densityParam), rssVariation(static_cast<long long>(-densityParam) * densityParam)
{
  multiplyRSSVariationByArea();
}

CandidateVariable& CandidateVariable::operator=(CandidateVariable&& otherCandidateVariable)
{
  if (this != &otherCandidateVariable) // Guard self assignment
    {
      nSet = std::move(otherCandidateVariable.nSet);
      density = otherCandidateVariable.density;
      rssVariation = otherCandidateVariable.rssVariation;
    }
  return *this;
}

const vector<vector<unsigned int>>& CandidateVariable::getNSet() const
{
  return nSet;
}

int CandidateVariable::getDensity() const
{
  return density;
}

long long CandidateVariable::getRSSVariation() const
{
  return rssVariation;
}

bool CandidateVariable::overlaps(const vector<vector<unsigned int>>::const_iterator otherDimensionBegin) const
{
  vector<vector<unsigned int>>::const_iterator dimensionIt = nSet.begin();
  if (*--dimensionIt->end() < *--otherDimensionBegin->end())
    {
      if (emptyIntersection(*dimensionIt, otherDimensionBegin->begin()))
	{
	  return false;
	}
    }
  else
    {
      if (emptyIntersection(*otherDimensionBegin, dimensionIt->begin()))
	{
	  return false;
	}
    }
  ++dimensionIt;
  const vector<vector<unsigned int>>::const_iterator dimensionEnd = nSet.end();
  vector<vector<unsigned int>>::const_iterator otherDimensionIt = otherDimensionBegin;
  do
    {
      if (*--dimensionIt->end() < *--(++otherDimensionIt)->end())
	{
	  if (emptyIntersection(*dimensionIt, otherDimensionIt->begin()))
	    {
	      return false;
	    }
	}
      else
	{
	  if (emptyIntersection(*otherDimensionIt, dimensionIt->begin()))
	    {
	      return false;
	    }
	}
    }
  while (++dimensionIt != dimensionEnd);
  return true;
}

vector<vector<unsigned int>> CandidateVariable::inter(const vector<vector<unsigned int>>::const_iterator otherDimensionBegin) const
{
  vector<vector<unsigned int>>::const_iterator otherDimensionIt = otherDimensionBegin;
  vector<vector<unsigned int>>::const_iterator dimensionIt = nSet.begin();
  vector<vector<unsigned int>> intersection;
  intersection.reserve(nSet.size());
  intersection.emplace_back();
  set_intersection(dimensionIt->begin(), dimensionIt->end(), otherDimensionIt->begin(), otherDimensionIt->end(), back_inserter(intersection.back()));
  if (intersection.back().empty())
    {
      return {};
    }
  intersection.emplace_back();
  ++dimensionIt;
  ++otherDimensionIt;
  set_intersection(dimensionIt->begin(), dimensionIt->end(), otherDimensionIt->begin(), otherDimensionIt->end(), back_inserter(intersection.back()));
  if (intersection.back().empty())
    {
      return {};
    }
  for (const vector<vector<unsigned int>>::const_iterator dimensionEnd = nSet.end(); ++dimensionIt != dimensionEnd; )
    {
      intersection.emplace_back();
      ++otherDimensionIt;
      set_intersection(dimensionIt->begin(), dimensionIt->end(), otherDimensionIt->begin(), otherDimensionIt->end(), back_inserter(intersection.back()));
      if (intersection.back().empty())
	{
	  return {};
	}
    }
  return intersection;
}

void CandidateVariable::addToRSSVariation(const long long delta)
{
  rssVariation += delta;
}

void CandidateVariable::reset()
{
  rssVariation = static_cast<long long>(-density) * density;
  multiplyRSSVariationByArea();
}

void CandidateVariable::multiplyRSSVariationByArea()
{
  const vector<vector<unsigned int>>::const_iterator dimensionEnd = nSet.end();
  vector<vector<unsigned int>>::const_iterator dimensionIt = nSet.begin();
  rssVariation *= dimensionIt->size();
  ++dimensionIt;
  do
    {
      rssVariation *= dimensionIt->size();
    }
  while (++dimensionIt != dimensionEnd);
}

bool CandidateVariable::emptyIntersection(const vector<unsigned int>& dimension, vector<unsigned int>::const_iterator otherDimensionElementIt)
{
  const vector<unsigned int>::const_iterator elementEnd = dimension.end();
  vector<unsigned int>::const_iterator elementIt = dimension.begin();
  do
    {
      for (; *otherDimensionElementIt < *elementIt; ++otherDimensionElementIt)
	{
	}
      if (*otherDimensionElementIt == *elementIt)
	{
	  return false;
	}
    }
  while (++elementIt != elementEnd);
  return true;
}
