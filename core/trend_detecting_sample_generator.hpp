#pragma once
/*
(C) Averisera Ltd 2017
*/
namespace averisera {
	class TrendDetectingSampleGenerator {
	public:
		enum class Rescaling {
			BEFORE, /**< Rescale before calculating derivatives */
			AFTER, /** Rescale after calculating derivatives */
			NEVER /** Do not rescale */
		};

		enum class Inputs {
			POINTS, /** Use points */
			SLOPES
		};
	private:
		Inputs inputs_;
		Rescaling rescaling_;
		unsigned int number_derivatives_;		
	};
}
