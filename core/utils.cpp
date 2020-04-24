/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include <iostream>
#include "daycount.hpp"
#include "csv_file_reader.hpp"
#include "log.hpp"
#include "math_utils.hpp"
#include "period.hpp"
#include "period_type.hpp"
#include "utils.hpp"
#include "preconditions.hpp"
#include "observed_discrete_data.hpp"
#include <boost/lexical_cast.hpp>

namespace averisera {
	namespace Utils {

		std::pair<size_t, size_t> load_probabilities(const char* fname, Eigen::MatrixXd& p, bool check_sum, Eigen::VectorXd& nbr_surveys, std::vector<double>& times) {
			averisera::CSVFileReader reader(fname, CSV::Delimiter::TAB, CSV::QuoteCharacter::DOUBLE_QUOTE, false); // expect no column names in the file
			reader.to_data(); // move to beginning of data rows
			const size_t T = reader.count_data_rows(); // number of rows == number of times for which we have distributions
			const size_t ncol = reader.count_columns(); // number of columns in the file
			if (ncol < 2) {
				throw std::runtime_error("Need at least 3 columns: probability(s), number of surveys, year");
			}
			const size_t dim = ncol - 2;
			p.resize(dim, T); // set the p matrix dimention
			nbr_surveys.resize(T);
			times.resize(T);
			std::vector<double> row(ncol);
			for (size_t t = 0; t < T; ++t) {
				reader.read_data_row(row); // read row from file
				for (unsigned int k = 0; k < dim; ++k) {
					p(k, t) = row[k]; // copy entry to matrix
				}

				// Check if probabilities add up to 1
				const double sum_p = p.col(t).sum(); // calculate the sum of entries in the t-th column of matrix p		
				if (check_sum && std::abs( sum_p - 1.0 ) > 1E-9) {
					// construct error message
					// transpose the column so that it's printed horizontally
					LOG_WARN() << "Utils::load_probabilities: Probabilities do not add up to 1.0 (error " << sum_p - 1.0 << ") in row " << t << ": " << p.col(t).transpose();
				}

				// Read number of surveys 
				nbr_surveys[t] = row[dim];		

				// Read year number 
				times[t] = row[dim + 1];
			}
			return std::make_pair(dim, T);
		}

		
		
		void load_matrix(const std::string& fname, Eigen::MatrixXd& m) {
            CSVFileReader reader(fname, CSV::Delimiter::TAB, CSV::QuoteCharacter::DOUBLE_QUOTE, false);
            unsigned int row_idx = 0;
            reader.to_data();
            std::vector<double> row;
            unsigned int dim = 0;
            while (reader.has_next_data_row()) {
                reader.read_data_row(row);
                dim = std::max(static_cast<unsigned int>(row.size()), std::max(dim, row_idx + 1));
                if (static_cast<unsigned int>(m.rows()) < dim) {
                    m.conservativeResize(dim, dim);
                }
                for (size_t i = 0; i < row.size(); ++i) {
                    m(row_idx, i) = row[i];
                }
                ++row_idx;
            }
            check_equals(row_idx, static_cast<unsigned int>(m.rows()));
        }

		template <> Date from_string<Date>(const std::string& str) {
			return Date::from_string(str);
		}

        template <> signed char from_string<signed char>(const char* str) {
            const int i = from_string<int>(str);
            return MathUtils::safe_cast<signed char>(i);
        }

		template <> Date from_string<Date>(const char* str) {
			return Date::from_string(str);
		}

		template <> Period from_string<Period>(const char* str) {
			return Period(str);
		}

		template <> PeriodType from_string<PeriodType>(const char* str) {
			return period_type_from_string(str);
		}

		template <> std::shared_ptr<const Daycount> from_string<std::shared_ptr<const Daycount>>(const char* str) {
			return Daycount::from_string(str);
		}
	}
}
