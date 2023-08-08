// Copyright 2018-2022 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of nclusterbox.

// nclusterbox is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// nclusterbox is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with nclusterbox.  If not, see <https://www.gnu.org/licenses/>.

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

/* UPDATE_SUMS makes nclusterbox update (rather than compute from scratch) the sums of the membership degrees on all elements after each addition/remotion step.  Use #define UPDATE_SUMS. */
#define UPDATE_SUMS

/* REMEMBER makes nclusterbox store or not (#define REMEMBER 0) every pattern that it visits, either in a hash set (#define REMEMBER 1) or in a homemade trie (#define REMEMBER 2) to avoid redundant computation if it is visited again.  Use #define REMEMBER 1. */
#define REMEMBER 1

/* VERBOSE_PARSER turns on the output (on the standard output) of information when the input data are parsed. */
/* #define VERBOSE_PARSER */

/* DEBUG_MODIFY turns on the output (on the standard output) of information during the execution of nclusterbox --jobs 1.  This option may be enabled by who wishes to understand how nclusterbox modifies a small number of patterns. */
/* #define DEBUG_MODIFY */

/* DEBUG_SELECT turns on the output (on the standard output) of information during the selection of the patterns.  This option may be enabled by who wishes to understand how a small number of patterns are selected. */
/* #define DEBUG_SELECT */

/* NUMERIC_PRECISION turns ou the output (on the standard output) of the maximal possible round-off error when internally storing a membership degree for modifying patterns and, then, for selecting them. */
/* #define NUMERIC_PRECISION */

/* NB_OF_PATTERNS turns on the output (on the standard output) of the numbers of patterns candidates for selection, and, then, of selected patterns. */
/* #define NB_OF_PATTERNS */

/* TIME turns on the output (on the standard output) of the run time of nclusterbox. */
#define TIME

/* DETAILED_TIME turns on the output (on the standard output) of a more detailed analysis of how the time is spent.  It lists (in this order): */
/* - the tensor parsing time */
/* - the tensor shifting time */
/* - the explanatory power maximization time */
/* - the tensor reduction time */
/* - the selection time */
#define DETAILED_TIME

/* GNUPLOT modifies the outputs of NUMERIC_PRECISION, NB_OF_PATTERNS, DETAILED_TIME and TIME.  They become tab separated values.  The output order is: */
/* - tensor parsing time (#ifdef DETAILED_TIME) */
/* - numeric precision for modifying patterns (#ifdef NUMERIC_PRECISION) */
/* - tensor shifting time (#ifdef DETAILED_TIME) */
/* - explanatory maximization time (#ifdef DETAILED_TIME) */
/* - number of patterns candidates for selection (#ifdef NB_OF_PATTERNS) */
/* - numeric precision for selecting patterns (#ifdef NUMERIC_PRECISION) */
/* - tensor reduction time (#ifdef DETAILED_TIME) */
/* - number of selected patterns (#ifdef NB_OF_PATTERNS) */
/* - selection time (#ifdef DETAILED_TIME) */
/* - total time (#ifdef TIME) */
/* #define GNUPLOT */

// Assert
/* ASSERT is used to check, after every iteration of nclusterbox, the correctness of the area and of the sums of membership degrees on every element and over the whole pattern. */
/* #define ASSERT */

#endif /*PARAMETERS_H_*/
