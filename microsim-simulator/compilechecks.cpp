#include "core/sacado_eigen.hpp"
#include <vector>
#include <memory>

namespace averisera {
# ifndef _WIN32
	void test1() {
		Eigen::MatrixXa a1(2,2);
		Eigen::MatrixXa a2(2,2);
		Eigen::MatrixXa b;
		b = a1 * a2;
	}
#endif

    class Class1 {
    public:
        Class1(std::vector < std::unique_ptr<double>>&& vec)
            : _vec(std::move(vec)) {
        }

        Class1() : Class1(build_vec()) {}
        Class1(const Class1&) = delete;
        Class1& operator=(const Class1&) = delete;
    private:
        std::vector<std::unique_ptr<double>> _vec;

        std::vector<std::unique_ptr<double>> build_vec() {           
            std::vector<std::unique_ptr<double>> v;
            v.push_back(std::unique_ptr<double>(new double(1.0)));
            return v;
        }
    };

    void test2() {
        //Class1 c1(std::vector<std::unique_ptr<double>>({ std::unique_ptr<double>(new double)}));
        std::vector<std::unique_ptr<double>> v;
        v.push_back(std::unique_ptr<double>(new double));
        //Class1 c1(std::move(v));
        Class1 c2;
    }
}