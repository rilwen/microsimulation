#include "eigen.hpp"
#include "csv_file_reader.hpp"

namespace averisera {
	namespace EigenUtils {
		Eigen::Ref<Eigen::VectorXd> from_vec(std::vector<double>& vec) {
			return Eigen::Map<Eigen::VectorXd>(&vec[0], vec.size());
		}

		Eigen::Ref<const Eigen::VectorXd> from_vec(const std::vector<double>& vec) {
			return Eigen::Map<const Eigen::VectorXd>(&vec[0], vec.size());
		}

		Eigen::MatrixXd read_matrix(CSVFileReader& reader, const std::vector<size_t>& indices, bool fill_with_nans) {
			const CSVFileReader::index_type nrows = reader.count_data_rows();
			const size_t ncols = indices.size();
			return matrix_from_iters<double>(reader.begin_double(indices, fill_with_nans), reader.end_double(indices, fill_with_nans), nrows, ncols);
		}

		Eigen::MatrixXd read_matrix(CSVFileReader& reader, const std::vector<std::string>& names, bool fill_with_nans) {
			return read_matrix(reader, reader.convert_names_to_indices(names), fill_with_nans);
		}		

		std::vector<std::vector<double>> as_vector_of_cols(const Eigen::MatrixXd& m) {
			std::vector<std::vector<double>> result;
			result.resize(m.cols());
			for (Eigen::Index c = 0; c < m.cols(); ++c) {
				auto& vc = result[c];
				vc.resize(m.rows());
				for (Eigen::Index r = 0; r < m.rows(); ++r) {
					vc[r] = m(r, c);
				}
			}
			return result;
		}
	}
}
