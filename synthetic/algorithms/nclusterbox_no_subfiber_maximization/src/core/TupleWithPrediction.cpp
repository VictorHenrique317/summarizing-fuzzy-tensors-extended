// Copyright 2022,2023 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#include "TupleWithPrediction.h"

int TupleWithPrediction::defaultMembership;

TupleWithPrediction::TupleWithPrediction(): estimatedMembership(0), estimatedMembershipIfDensestRemoved(0), realMembership(defaultMembership)
{
}

TupleWithPrediction::TupleWithPrediction(const int realMembershipParam): estimatedMembership(0), estimatedMembershipIfDensestRemoved(0), realMembership(realMembershipParam)
{
}

int TupleWithPrediction::getRealMembership() const
{
  return realMembership;
}

bool TupleWithPrediction::isDensest(const int estimation) const
{
  return estimation == estimatedMembership;
}

bool TupleWithPrediction::isGreaterThanDensest(const int estimation) const
{
  return estimation > estimatedMembership;
}

bool TupleWithPrediction::isGreaterThanSecondDensest(const int estimation) const
{
  return estimation > estimatedMembershipIfDensestRemoved;
}

long long TupleWithPrediction::squaredResidualVariationFromDensest(const int estimation) const
{
  return static_cast<long long>(estimatedMembership - realMembership) * (estimatedMembership - realMembership) - static_cast<long long>(estimation - realMembership) * (estimation - realMembership);
}

long long TupleWithPrediction::squaredResidualVariationFromSecondDensest(const int estimation) const
{
  return static_cast<long long>(estimatedMembershipIfDensestRemoved - realMembership) * (estimatedMembershipIfDensestRemoved - realMembership) - static_cast<long long>(estimation - realMembership) * (estimation - realMembership);
}

void TupleWithPrediction::setRealMembership(const int realMembershipParam)
{
  realMembership = realMembershipParam;
}

void TupleWithPrediction::setEstimatedMembership(const int estimatedMembershipParam)
{
  estimatedMembership = estimatedMembershipParam;
}

void TupleWithPrediction::addPrediction(const int estimation)
{
  if (estimation > estimatedMembership)
    {
      estimatedMembershipIfDensestRemoved = estimatedMembership;
      estimatedMembership = estimation;
      return;
    }
  if (estimation > estimatedMembershipIfDensestRemoved)
    {
      estimatedMembershipIfDensestRemoved = estimation;
    }
}

void TupleWithPrediction::reset()
{
  estimatedMembership = 0;
  estimatedMembershipIfDensestRemoved = 0;
}

void TupleWithPrediction::setDefaultMembership(const int defaultMembershipParam)
{
  defaultMembership = defaultMembershipParam;
}
