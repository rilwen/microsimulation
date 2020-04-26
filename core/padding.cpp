// (C) Averisera Ltd 2014-2020
#include "padding.hpp"

namespace averisera {
	bool Padding::pad_nan_rows(Eigen::MatrixXd& m) {
		typedef decltype(m.rows()) row_index;
		bool result = true;
		for (row_index r = 0; r < m.rows(); ++r) {
            auto row = m.row(r);
			result &= pad_nan(row);
		}
		return result;
	}

	bool Padding::pad_nan_cols(Eigen::MatrixXd& m) {
		typedef decltype(m.cols()) col_index;
		bool result = true;
		for (col_index c = 0; c < m.cols(); ++c) {
            auto col = m.col(c);
			result &= pad_nan(col);
		}
		return result;
	}
}
