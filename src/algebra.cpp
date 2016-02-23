#include "algebra.h"
#include <vector>

namespace slae {
  void diag3rd(double* a, double* b, double* c, double* d, double* dst, int n) {
    std::vector<double> ksi(n, 0);
    std::vector<double> eta(n);

    ksi[0] = c[0] / b[0];
    eta[0] = d[0] / b[0];
    for (int i = 1; i<n; i++) {
      ksi[i] = c[i] / (b[i] - a[i] * ksi[i - 1]);
      eta[i] = (d[i] - a[i] * eta[i - 1]) / (b[i] - a[i] * ksi[i - 1]);
    }

    dst[n - 1] = eta[n - 1];
    for (int i = n - 2; i >= 0; i--) {
      dst[i] = eta[i] - ksi[i] * dst[i + 1];
    }
  }
}
