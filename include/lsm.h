#ifndef LSM_H_INCLUDED__
#define LSM_H_INCLUDED__

#include <array>
#include <vector>

#include <matrix.h>

namespace lsm {

	class QuadFunc {
		std::array<double, 3> coefs_;

	public:
		QuadFunc(void) :
			coefs_()
		{

		}

		template<class T> 
		QuadFunc(T xIter, T yIter, int n) {
			build(xIter, yIter, n);
		}

		// x - узлы сетки, должны быть упорядочены по возрастанию, кратные узлы запрещены
		// y - значения функции в узлах сетки
		// n - количество узлов сетки
		template<class T>
		void build(T xIter, T yIter, int n) {
			double sum[4] = { 0, 0, 0, 0 };
			std::vector<double> rhs(3, 0);
			for (int i = 0; i<n; ++i) {
				double x = *xIter++, y = *yIter++;

				sum[0] += x;
				sum[1] += x*x;
				sum[2] += x*x*x;
				sum[3] += x*x*x*x;

				rhs[0] += x*x*y;
				rhs[1] += x*y;
				rhs[2] += y;
			}

			mat3d mat;
			mat(0, 0) = sum[3]; mat(0, 1) = sum[2]; mat(0, 2) = sum[1];
			mat(1, 0) = sum[2]; mat(1, 1) = sum[1]; mat(1, 2) = sum[0];
			mat(2, 0) = sum[1]; mat(2, 1) = sum[0]; mat(2, 2) = n;
			mat.transpose();

			coefs_ = mat.gauss(rhs);
		}

		double operator()(double x) {
			return coefs_[2] + x*coefs_[1] + x*x*coefs_[0];
		}
	};
}

#endif //LSM_H_INCLUDED__
