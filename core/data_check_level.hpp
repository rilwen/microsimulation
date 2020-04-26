// (C) Averisera Ltd 2014-2020
#pragma once

namespace averisera {
	/** Level of numeric data checking. For automatic differentiation types, check is applied only to value (not to derivatives). */
	enum class DataCheckLevel {
		ANY, /**< Any value is OK */
		NOT_NAN, /** Value cannot be NaN */
		FINITE /** Value must be finite */
	};
}