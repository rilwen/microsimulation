#pragma once
/*
(C) Averisera Ltd 2017
*/
#include "kmeans.hpp"
#include <memory>

namespace averisera {
	/** KMeans::InitStrategy implementations */
	namespace KMeansInitStrategies {
		/** @see https://en.wikipedia.org/wiki/K-means_clustering#Initialization_methods */
		std::unique_ptr<const KMeans::InitStrategy> make_forgy();

		/** @see https://en.wikipedia.org/wiki/K-means_clustering#Initialization_methods */
		std::unique_ptr<const KMeans::InitStrategy> make_random_partition();

		/** @see https://en.wikipedia.org/wiki/K-means%2B%2B */
		std::unique_ptr<const KMeans::InitStrategy> make_kmeanspp();
	}
}
