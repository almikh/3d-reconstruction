#ifndef MATRIX_H_INCLUDED__
#define MATRIX_H_INCLUDED__

#include <array>
#include <assert.h>

#include <vec3.h>

#undef minor

template<class T, int dim>
class matrix {
protected:
  const int count_;
  T* data_;

  inline T& at(int i, int j) {
    return data_[j*dim + i];
  }
  inline T at(int i, int j) const {
    return data_[j*dim + i];
  }

private:
  int maxInRow(int row, int startCol = 0) {
    int res = startCol;
    for (int j = startCol + 1; j<dim; ++j) {
      if (at(row, j)>at(row, res)) res = j;
    }

    return res;
  }

  int maxInCol(int col, int startRow = 0) {
    int res = startRow;
    for (int i = startRow + 1; i<dim; ++i) {
      if (at(i, col)>at(res, col)) res = i;
    }

    return res;
  }

  matrix<T, dim>& swapRows(int l1, int l2) {
    if (l1 == l2) return *this;
    for (auto j = 0; j<dim; ++j) {
      swap(at(l1, j), at(l2, j));
    }

    return *this;
  }

  matrix<T, dim>& swapCols(int c1, int c2) {
    if (c1 == c2) return *this;
    for (auto i = 0; i<dim; ++i) {
      swap(at(i, c1), at(i, c2));
    }

    return *this;
  }

  matrix<T, dim>& toUpperTriang(int* permutations = nullptr) {
    if (permutations) *permutations = 0;
    for (int k = 0; k<dim; ++k) {
      int ind = maxInCol(k, k);
      if (ind != k) {
        swapRows(k, ind);
        if (permutations) ++(*permutations);
      }

      /* обнуляем стоящие ниже диагонального */
      for (int j = k + 1; j<dim; ++j) {
        double mn = double(at(j, k)) / at(k, k);
        for (int s = 0; s<dim; ++s) {
          at(j, s) -= at(k, s)*mn;
        }
      }
    }

    return *this;
  }

public:
  matrix() : count_(dim*dim), data_(new T[dim*dim]) {}
  explicit matrix(T value) :
    count_(dim*dim),
    data_(new T[dim*dim])
  {
    T* cur = data_;
    for (int i = 0; i < count_; ++i) *cur++ = value;
  }

  matrix(const matrix<T, dim>& matrix) :
    count_(dim*dim),
    data_(new T[dim*dim])
  {
    for (int i = 0; i < count_; ++i) data_[i] = matrix.data_[i];
  }
  matrix(matrix<T, dim>&& matrix) :
    count_(dim*dim),
    data_(matrix.data_)
  {
    matrix.data_ = nullptr;
  }

  ~matrix() {
    if (data_) delete data_;
  }

  inline T& operator ()(int i, int j) {
    return data_[j*dim + i];
  }
  inline const T operator ()(int i, int j) const {
    return data_[j*dim + i];
  }

  matrix<T, dim>& operator =(const matrix<T, dim>& rhs) {
    if (this == &rhs) return *this;
    memcpy(data_, rhs.data_, dim*dim*sizeof(T));
    return *this;
  }
  matrix<T, dim>& operator =(matrix<T, dim>&& rhs) {
    if (this == &rhs) return *this;
    if (data_) delete[] data_;
    data_ = rhs.data_;
    rhs.data_ = nullptr;
    return *this;
  }

  matrix<T, dim>& operator +=(const matrix<T, dim>& rhs) {
    T* cur = data_;
    T* cur2 = rhs.data_;
    for (int i = 0; i < count_; ++i) *cur++ += *cur2++;
    return *this;
  }
  matrix<T, dim>& operator -=(const matrix<T, dim>& rhs) {
    T* cur = data_;
    T* cur2 = rhs.data_;
    for (int i = 0; i < count_; ++i) *cur++ -= *cur2++;
    return *this;
  }
  matrix<T, dim>& operator *=(const matrix<T, dim>& rhs) {
    matrix<T, dim> old(*this);
    for (int i = 0; i < dim; ++i) {
      for (int j = 0; j < dim; ++j) {
        T temp = 0;
        for (int r = 0; r < dim; ++r) temp += old(i, r)*rhs(r, j);
        at(i, j) = temp;
      }
    }
    return *this;
  }

  void create(const T* source) {
    for (int i = 0; i < count_; ++i) data_[i] = source[i];
  }

  inline T* data() const {
    return data_;
  }

  inline int count() const {
    return count_;
  }

  matrix<T, dim>& fill(T val) {
    T* cur = data_;
    for (int i = 0; i < count_; ++i) *cur++ = val;
    return *this;
  }

  matrix<T, dim>& transpose() {
    matrix<T, dim> old(*this);
    for (int i = 0; i < dim; ++i) {
      for (int j = 0; j < dim; ++j) {
        at(i, j) = old(j, i);
      }
    }
    return *this;
  }

  template<class Container> // Container - контейнер с перегруженной операцией индексирования - `[]`
  std::array<T, dim> gauss(const Container& rhs) const {
    // матрица размерности `dim x (dim + 1)`
    using aug_matrix_t = std::array<std::array<T, dim + 1>, dim>;

    aug_matrix_t temp;
    for (int i = 0; i < dim; ++i) {
      for (int j = 0; j < dim; ++j) temp[i][j] = at(i, j);
      temp[i][dim] = rhs[i];
    }

    int num_row;
    for (int k = 0; k < dim - 1; ++k) {
      num_row = k;
      while (abs(temp[num_row][k]) < Double::epsilon() && num_row < dim - 1) ++num_row;
      if (abs(temp[num_row][k]) > Double::epsilon()) {
        swap(temp[k], temp[num_row]);
        for (int j = k + 1; j < dim; ++j) {
          double mn = temp[j][k] / temp[k][k];
          for (int i = k; i < dim + 1; ++i) temp[j][i] -= temp[k][i] * mn;
        }
      }
    }

    std::array<T, dim> dst;
    for (int i = dim - 1; i >= 0; --i) {
      if (std::abs(temp[i][i]) > Double::epsilon()) {
        dst[i] = temp[i][dim];
        for (int j = dim - 1; j > i; --j) dst[i] -= dst[j] * temp[i][j];
        dst[i] /= temp[i][i];
      }
    }

    return dst;
  }

  /* Возвращает значение соответствующего минора матрицы */
  T minor(int row, int col) {
    matrix<T, dim - 1> dst;
    for (int i = 0, ii = 0; i<dim; ++i, ++ii) {
      if (i == row) --ii;
      else {
        for (int j = 0, jj = 0; j<dim; ++j, ++jj) {
          if (j == col) --jj;
          else dst(ii, jj) = at(i, j);
        }
      }
    }

    return dst.det();
  }

  /* Возвращает определитель матрицы */
  T det() const {
    matrix<T, dim> temp(*this);
    int permut;
    temp.toUpperTriang(&permut);
    T dst = 1;
    for (int i = 0; i<dim; ++i) {
      dst *= temp(i, i);
    }
    if (permut % 2 == 1) dst = -dst;
    return dst;
  }

  static matrix<T, dim> zero() {
    return matrix<T, dim>(0);
  }
  static matrix<T, dim> identity() {
    matrix<T, dim> temp(0);
    for (int i = 0; i < dim; ++i) temp(i, i) = 1;
    return temp;
  }
  static matrix<T, dim> scale(T scale) {
    assert(dim >= 3);

    matrix<T, dim> temp(0);
    for (int i = 0; i < 3; ++i) temp(i, i) = scale;
    temp(4, 4) = 1;

    return temp;
  }
  static matrix<T, dim> translation(const vec3<T>& shift) {

    assert(dim >= 4);
    matrix<T, dim> temp(identity());
    temp(3, 0) = shift.x;
    temp(3, 1) = shift.y;
    temp(3, 2) = shift.z;

    return temp;
  }
  static matrix<T, dim> rotation(const vec3<T>& axis, double angle) {
    matrix<T, dim> temp(identity());
    double x = axis.x, y = axis.y, z = axis.z;
    T c = cos(angle), s = sin(angle), t = 1.0 - c;
    temp(0, 0) = c + t*x*x;   temp(0, 1) = t*x*y - s*z; temp(0, 2) = t*x*z + s*y;
    temp(1, 0) = t*y*x + s*z; temp(1, 1) = c + t*y*y;   temp(1, 2) = t*y*z - s*x;
    temp(2, 0) = t*z*x - s*y; temp(2, 1) = t*z*y + s*x; temp(2, 2) = c + t*z*z;

    return temp;
  }

  // Матрица вращения из углов Эйлера
  static matrix<T, dim> rotation(const vec3<T>& angles) {
    auto mx = rotX(angles.x);
    auto my = rotY(angles.y);
    auto mz = rotZ(angles.z);

    return mx * my * mz;
  }
  static matrix<T, dim> rotX(double rad) {
    return rotation(vec3d(1, 0, 0), rad);
  }
  static matrix<T, dim> rotY(double rad) {
    return rotation(vec3d(0, 1, 0), rad);
  }
  static matrix<T, dim> rotZ(double rad) {
    return rotation(vec3d(0, 0, 1), rad);
  }
};

typedef matrix<double, 3> mat3d;
typedef matrix<int, 3> mat3i;
typedef matrix<unsigned int, 3> mat3ui;

typedef matrix<double, 4> mat4d;
typedef matrix<int, 4> mat4i;
typedef matrix<unsigned int, 4> mat4ui;

#endif // MATRIX_H_INCLUDED__
