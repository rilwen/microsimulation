#pragma once
/*
(C) Averisera Ltd 2017
*/

namespace averisera {
	/** Combinatorial functions */
	namespace Combinatorics {
		/**
		Return the number of ways n can be expressed as sum of exactly m positive integers.
		@param m Number of integers
		@param n Partitioned sum
		*/
		unsigned long partial_partition(unsigned int m, int n);

		/**
		Return the number of ways n can be expressed as sum of exactly m positive integers
		greater or equal k.
		@param m Number of integers
		@param n Partitioned sum
		@param k Minimum size
		*/
		unsigned long partial_partition_restricted_size(unsigned int m, int n, unsigned int k);

		/**
		Return the number of ways in which n can be expressed as a sum of a sequence of exactly m positive integers.
		@param m Number of integers
		@param n Partitioned sum
		*/
		unsigned long partial_composition(unsigned int m, int n);

		/**
		Return the number of ways in which n can be expressed as a sum of a sequence of exactly m positive integers
		greater or equal k.
		@param m Number of integers
		@param n Partitioned sum
		*/
		unsigned long partial_composition_restricted_size(unsigned int m, int n, unsigned int k);
	}
}
