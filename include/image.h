#pragma once
#include <cmath>
#include <vector>
#include <QImage>
#include <functional>

#include <defs.h>
#include <vec2.h>

namespace ip {
  struct Size {
    int width, height;

    Size(int width = 0, int height = 0) : width(width), height(height) {}

    bool operator==(const Size& other) {
      return width == other.width && height == other.height;
    }

    Size& operator * (int val) {
      height *= val;
      width *= val;
      return *this;
    }
  };

  template<typename T>
  class Image {
    T* data_;
    int width_, height_;

    void recreate(int width, int height, const T& val = T()) {
      if (width_ * height_ != width * height) {
        release();
        data_ = new T[width * height];
      }

      width_ = width;
      height_ = height;

      clear(val);
    }

    void release() {
      if (data_) {
        delete[] data_;
        data_ = nullptr;
      }

      width_ = height_ = 0;
    }

    Image<T> convolution(const Image<T>& kernel) const {
      static_assert(std::is_floating_point<T>::value, "Value with floating point required.");

      Image<T> dst(size(), 0);
      int r = kernel.width() / 2;

      /* центральная область */
      for (int i = r; i < width_ - r; ++i) {
        for (int j = r; j < height_ - r; ++j) {
          for (int dx = -r; dx <= r; dx++) {
            for (int dy = -r; dy <= r; dy++)
              dst(i, j) += at(i + dx, j + dy)*kernel(dx + r, dy + r);
          }
        }
      }

      /* слева */
      for (int i = 0; i < r; ++i) {
        for (int j = r; j < height_ - r; ++j) {
          for (int dx = -r; dx <= r; dx++) {
            for (int dy = -r; dy <= r; dy++)
              dst(i, j) += at(abs(i + dx), j + dy)*kernel(dx + r, dy + r);
          }
        }
      }

      /* справа */
      int temp = 2 * width_ - 1, tempInd;
      for (int i = width_ - r; i < width_; ++i) {
        for (int j = r; j < height_ - r; ++j) {
          for (int dx = -r; dx <= r; dx++) {
            for (int dy = -r; dy <= r; dy++) {
              tempInd = (i + dx >= width_) ? temp - i - dx : i + dx;
              dst(i, j) += at(tempInd, j + dy)*kernel(dx + r, dy + r);
            }
          }
        }
      }

      /* сверху */
      for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < r; ++j) {
          for (int dx = -r; dx <= r; dx++)
            for (int dy = -r; dy <= r; dy++) {
              tempInd = (i + dx >= width_) ? temp - i - dx : abs(i + dx);
              dst(i, j) += at(tempInd, abs(j + dy))*kernel(dx + r, dy + r);
            }
        }
      }

      /*снизу*/
      int tempH = 2 * height_ - 1;
      for (int i = 0; i < width_; ++i) {
        for (int j = height_ - r; j < height_; ++j) {
          for (int dx = -r; dx <= r; dx++) {
            for (int dy = -r; dy <= r; dy++) {
              tempInd = (i + dx >= width_) ? temp - i - dx : abs(i + dx);
              dst(i, j) += at(tempInd, (j + dy >= height_) ? tempH - j - dy : j + dy)*kernel(dx + r, dy + r);
            }
          }
        }
      }

      return dst;
    }

    Image<T> gradient(const Image<T>& kernel, T(*valueInPoint)(T, T)) const {
      static_assert(std::is_floating_point<T>::value, "Value with floating point required.");

      Image<T> dst(size(), 0.0);
      int r = kernel.width() / 2;

      /* центральная область */
      double fx, fy;
      for (int i = r; i < width_ - r; ++i) {
        for (int j = r; j < height_ - r; ++j) {
          fx = fy = 0;
          for (int dx = -r; dx <= r; dx++) {
            for (int dy = -r; dy <= r; dy++) {
              fx += at(i + dx, j + dy)*kernel(dx + r, dy + r);
              fy += at(i + dx, j + dy)*kernel(dy + r, dx + r);
            }
          }
          dst(i, j) = valueInPoint(fy, fx);
        }
      }

      /* слева */
      for (int i = 0; i < r; ++i) {
        for (int j = r; j < height_ - r; ++j) {
          fx = fy = 0;
          for (int dx = -r; dx <= r; dx++) {
            for (int dy = -r; dy <= r; dy++) {
              fx += at(abs(i + dx), j + dy)*kernel(dx + r, dy + r);
              fy += at(abs(i + dx), j + dy)*kernel(dy + r, dx + r);
            }
          }
          dst(i, j) = valueInPoint(fy, fx);
        }
      }

      /* справа */
      int temp = 2 * width_ - 1, tempInd;
      for (int i = width_ - r - 1; i < width_; ++i) {
        for (int j = r; j < height_ - r; ++j) {
          fx = fy = 0;
          for (int dx = -r; dx <= r; dx++) {
            for (int dy = -r; dy <= r; dy++) {
              tempInd = (i + dx >= width_) ? temp - i - dx : i + dx;
              fx += at(tempInd, j + dy)*kernel(dx + r, dy + r);
              fy += at(tempInd, j + dy)*kernel(dy + r, dx + r);
            }
          }
          dst(i, j) = valueInPoint(fy, fx);
        }
      }

      /* сверху */
      for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < r; ++j) {
          fx = fy = 0;
          for (int dx = -r; dx <= r; dx++) {
            for (int dy = -r; dy <= r; dy++) {
              tempInd = (i + dx >= width_) ? temp - i - dx : abs(i + dx);
              fx += at(tempInd, abs(j + dy))*kernel(dx + r, dy + r);
              fy += at(tempInd, abs(j + dy))*kernel(dy + r, dx + r);
            }
          }
          dst(i, j) = valueInPoint(fy, fx);
        }
      }

      /* снизу */
      int tempH = 2 * height_ - 1;
      for (int i = 0; i < width_; ++i) {
        for (int j = height_ - r; j < height_; ++j) {
          fx = fy = 0;
          for (int dx = -r; dx <= r; dx++) {
            for (int dy = -r; dy <= r; dy++) {
              tempInd = (i + dx >= width_) ? temp - i - dx : abs(i + dx);
              fx += at(tempInd, (j + dy >= height_) ? tempH - j - dy : j + dy)*kernel(dx + r, dy + r);
              fy += at(tempInd, (j + dy >= height_) ? tempH - j - dy : j + dy)*kernel(dy + r, dx + r);
            }
          }
          dst(i, j) = valueInPoint(fy, fx);
        }
      }

      return dst;
    }

  public:
    Image() : data_(nullptr), width_(0), height_(0) {}
    Image(const Image<T>& matrix) :
      data_(new T[matrix.width_*matrix.height_]),
      width_(matrix.width_),
      height_(matrix.height_)
    {
      memcpy(data_, matrix.data_, sizeof(T)*width_*height_);
    }
    Image(Image<T>&& matrix) :
      data_(matrix.data_),
      width_(matrix.width_),
      height_(matrix.height_)
    {
      matrix.data_ = nullptr;
    }
    Image(const Size& size, const T& val = 0) : data_(0) {
      recreate(size.width, size.height, val);
    }
    Image(int width, int height, const T& val = 0) : data_(0) {
      recreate(width, height, val);
    }
    Image(const QImage& image) :
      data_(new T[image.width()*image.height()]),
      width_(image.width()),
      height_(image.height())
    {
      for (int i = 0; i<image.width(); ++i) {
        for (int j = 0; j<image.height(); ++j) {
          QRgb rgb = image.pixel(i, j);
          T gray = static_cast<T>(qRed(rgb) * 0.114 + qGreen(rgb) * 0.587 + qBlue(rgb) * 0.299);
          at(i, j) = gray;
        }
      }
    }
    ~Image() {
      release();
    }

    Image<T>& operator = (const Image<T>& matrix) {
      if (this == &matrix) return *this;

      if (width_*height_ != matrix.width_*matrix.height_) {
        release();
        width_ = matrix.width_;
        height_ = matrix.height_;
        data_ = new T[width_*height_];
      }

      memcpy(data_, matrix.data_, sizeof(T)*width_*height_);
      return *this;
    }
    Image<T>& operator = (Image<T>&& other) {
      if (this == &other) return *this;

      release();
      width_ = other.width_;
      height_ = other.height_;
      std::swap(data_, other.data_);
      return *this;
    }

    Image<T>& operator = (const QImage& image) {
      release();
      data_ = new T[image.width()*image.height()];
      width_ = image.width();
      height_ = image.height();

      for (int i = 0; i<image.width(); ++i) {
        for (int j = 0; j<image.height(); ++j) {
          QRgb rgb = image.pixel(i, j);
          T gray = static_cast<T>(qRed(rgb) * 0.114 + qGreen(rgb) * 0.587 + qBlue(rgb) * 0.299);
          at(i, j) = gray;
        }
      }

      return *this;
    }

    void save(const char* filename) const {
      QImage canvas = toQImage();
      canvas.save(filename);
    }

    template<typename T2>
    Image<T>& from(const Image<T2>& src) {
      if (data_) delete[] data_;
      data_ = new T[src.width() * src.height()];
      width_ = src.width();
      height_ = src.height();

      for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
          at(i, j) = static_cast<T>(src(i, j));
        }
      }

      return *this;
    }

    template<typename T2>
    Image<T2> to() const {
      Image<T2> dst(size());

      for (int i = 0; i<width_; ++i) {
        for (int j = 0; j<height_; ++j) {
          dst(i, j) = static_cast<T2>((*this)(i, j));
        }
      }
      return dst;
    }

    QImage toQImage() const {
      QImage canvas(width_, height_, QImage::Format_RGB888);
      for (int i = 0; i<canvas.width(); ++i) {
        for (int j = 0; j<canvas.height(); ++j) {
          canvas.setPixel(i, j, qRgb(at(i, j), at(i, j), at(i, j)));
        }
      }

      return canvas;
    }

    static Image<T> unite(const Image<T>& lhs, const Image<T>& rhs, T(*uniteFunc)(T, T)) {
      Image<T> dst(lhs.size());
      for (int i = 0; i < lhs.width(); ++i) {
        for (int j = 0; j < lhs.height(); ++j) {
          dst(i, j) = uniteFunc(lhs(i, j), rhs(i, j));
        }
      }

      return dst;
    }

    std::vector<T> selectValues(std::function<bool(const T&)> selector) {
      std::vector<T> dst;

      T* cur = data_;
      dst.reserve(width_*height_);
      for (int i = 0, n = width_*height_; i < n; ++i) {
        if (selector(*cur)) dst.push_back(*cur);
        ++cur;
      }

      return dst;
    }

    void swap(Image<T>& matrix) {
      std::swap(data_, matrix.data_);
      std::swap(width_, matrix.width_);
      std::swap(height_, matrix.height_);
    }

    T* data() const {
      return data_;
    }
    bool isNull() const {
      return !data_;
    }
    Size size() const {
      return Size(width_, height_);
    }
    int height() const {
      return height_;
    }
    int width() const {
      return width_;
    }

    bool isCorrect(int x, int y) const {
      return (x >= 0 && y >= 0 && x<width_ && y<height_);
    }

    T& operator () (int i, int j) {
      return data_[i + j*width_];
    }
    const T operator () (int i, int j) const {
      return data_[i + j*width_];
    }

    T* line(int j) {
      return data_ + j*width_;
    }
    const T* line(int j) const {
      return data_ + j*width_;
    }

    T& at(int i, int j) {
      return data_[i + j*width_];
    }
    const T at(int i, int j) const {
      return data_[i + j*width_];
    }

    T& at(const vec2i& point) {
      return data_[point.x + point.y*width_];
    }
    const T at(const vec2i& point) const {
      return data_[point.x + point.y*width_];
    }

    T sum() const {
      T acc = 0;
      T* cur = data_;
      for (int i = 0, n = width_*height_; i<n; ++i) {
        acc += *cur++;
      }

      return acc;
    }
    T medium() const {
      double acc = 0;
      T* cur = data_;
      for (int i = 0, n = width_*height_; i<n; ++i) {
        acc += *cur++;
      }

      return T(acc / (width_*height_));
    }
    T maximum() const {
      T* cur = data_;
      T fmax = *cur++;
      for (int i = 1, n = width_*height_; i<n; ++i) {
        if (*cur>fmax) fmax = *cur;
        ++cur;
      }

      return fmax;
    }
    T minimum() const {
      T* cur = data_;
      T fmin = *cur++;
      for (int i = 1, n = width_*height_; i<n; ++i) {
        if (*cur<fmin) fmin = *cur;
        ++cur;
      }

      return fmin;
    }

    Image<T>& transform(T(*func)(T val)) {
      T* cur = data_;
      for (int i = 0, n = width_*height_; i<n; ++i) {
        *cur = func(*cur);
        ++cur;
      }

      return *this;
    }
    Image<T>& scale(T down, T up) {
      T* cur = data_;
      T fmin = minimum();
      double temp = double(up - down) / (maximum() - fmin);
      for (int i = 0, n = width_*height_; i<n; ++i) {
        *cur = T(down + (*cur - fmin) * temp);
        ++cur;
      }

      return *this;
    }
    Image<T>& transpose() {
      T* dst = new T[height_*width_];
      for (int i = 0; i<width_; ++i) {
        for (int j = 0; j<height_; ++j) dst[i + j*width_] = data_[j + i*width_];
      }

      delete[] data_;
      data_ = dst;
      std::swap(height_, width_);
      return *this;
    }
    Image<T>& clear(const T& val) {
      T* cur = data_;
      for (int i = 0, n = width_*height_; i<n; ++i) {
        *cur++ = val;
      }

      return *this;
    }

    Image<T>& bilateralFiltering(double sigmaS, double sigmaR) { // Если радиус = 0, то используется радиус 2*sigma
      int radius = 2 * static_cast<int>(sigmaS);

      auto w = [&](int i, int j, int k, int l) -> double {
        double first = -((i - k)*(i - k) + (j - l)*(j - l)) * 0.5 / sigmaS / sigmaS;
        double second = -math::sqr(std::abs(at(i, j) - at(k, l))) * 0.5 / sigmaR / sigmaR;
        return exp(first + second);
      };

      auto src = to<double>();
      double factor = 0, color = 0, temp;
      for (int i = radius; i < width_ - radius; ++i) {
        for (int j = radius; j < height_ - radius; ++j) {
          color = 0;
          factor = 0;
          for (int dx = -radius; dx <= radius; dx++) {
            for (int dy = -radius; dy <= radius; dy++) {
              temp = w(i, j, i + dx, j + dy);
              color += src(i + dx, j + dy) * temp;
              factor += temp;
            }
          }

          at(i, j) = T(std::round(color / factor));
        }
      }

      return *this;
    }

    Image<T>& gaussianBlur(int radius, double sigma) { // Если радиус = 0, то используется радиус 3*sigma
      if (radius == 0) {
        radius = static_cast<int>(std::round(3 * sigma));
      }

      *this = convolution(Image<double>::makeGaussianKernel(radius, sigma, true));
      return *this;
    }

    Image<T>& medianBlur(int radius) {
      *this = convolution(matd::averaging(radius));
      return *this;
    }

    Image<T>& kuwahara(int radius) {
      Image<double> src(to<double>());

      int n;
      double medium[4];
      double variance[4];
      int dx[4] = { -1, 1, -1, 1 };
      int dy[4] = { -1, -1, 1, 1 };

      for (int i = 1; i < width_ - 1; ++i) {
        for (int j = 1; j < height_ - 1; ++j) {
          for (int k = 0; k < 4; ++k) {
            n = 0;
            variance[k] = medium[k] = 0;
            for (int ii = 0; abs(ii) <= radius; ii += dx[k]) {
              for (int jj = 0; abs(jj) <= radius; jj += dy[k]) {
                if (!isCorrect(i + ii, j + jj)) continue;
                variance[k] += src(i + ii, j + jj)*src(i + ii, j + jj);
                medium[k] += src(i + ii, j + jj);
                ++n;
              }
            }

            medium[k] /= n;
            variance[k] = 1.0 / n*variance[k] - medium[k] * medium[k];
          }

          int target = min_element(variance, variance + 4) - variance; //ptr. diff
          at(i, j) = static_cast<T>(medium[target]);
        }
      }

      return *this;
    }

    Image<T>& laplace(int mode = 4) {
      switch (mode) {
      case 4:
        *this = convolution(matd::makeLaplace4Kernel()).scale(0.0, 255.0);
        break;
      case 8:
        *this = convolution(matd::makeLaplace8Kernel()).scale(0.0, 255.0);
        break;
      case 12:
        *this = convolution(matd::makeLaplace12Kernel()).scale(0.0, 255.0);
        break;
      }

      return *this;
    }

    Image<T>& sobel() {
      auto modulus = [](double fy, double fx) -> double {
        return std::sqrt(fx*fx + fy*fy);
      };

      *this = gradient(matd::makeSobelKernel(), modulus).scale(0.0, 255.0);
      return *this;
    }

    Image<T>& kirsh() {
      matd src(to<double>());
      for (int i = 1; i < width_ - 1; ++i) {
        for (int j = 1; j < height_ - 1; ++j) {
          int F = 0;
          for (int ind = 0; ind < 8; ++ind) {
            int S = 0, T = 0;
            for (int k = 0; k < 3; ++k) {
              int index = normalize(k + ind, 8);
              S += src(i + ip::cdx[index], j + ip::cdy[index]);
            }

            for (int k = 3; k < 8; ++k) {
              int index = normalize(k + ind, 8);
              T += src(i + ip::cdx[index], j + ip::cdy[index]);
            }

            F = std::max(F, abs(5 * S - 3 * T));
          }

          at(i, j) = F;
        }
      }

      scale(0, 255);
      return *this;
    }

    void gvf(double mu, int iters, Image<T>& u, Image<T>& v) {
      static_assert(std::is_floating_point<T>::value, "Value with floating point required.");

      Image<double> f = to<double>();
      f.gaussianBlur(0, 0.66).scale(0, 1);

      u.recreate(f.width(), f.height(), 0.0);
      v.recreate(f.width(), f.height(), 0.0);

      /* Compute derivative */
      for (int i = 1; i < width() - 1; ++i) {
        for (int j = 1; j < height() - 1; ++j) {
          u(i, j) = 0.5*(f(i + 1, j) - f(i - 1, j));
          v(i, j) = 0.5*(f(i, j + 1) - f(i, j - 1));
        }
      }

      for (int j = 0; j < height(); ++j) {
        u(0, j) = 0.5*(f(1, j) - f(0, j));
        u(width() - 1, j) = 0.5*(f(width() - 1, j) - f(width() - 2, j));
      }

      for (int i = 0; i < width(); ++i) {
        v(i, 0) = 0.5*(f(i, 1) - f(i, 0));
        v(i, height() - 1) = 0.5*(f(i, height() - 1) - f(i, height() - 2));
      }

      /* Compute parameters and initializing arrays */
      matd b(f.size()), c1(f.size()), c2(f.size());
      for (int i = 0; i < width(); ++i) {
        for (int j = 0; j < height(); ++j) {
          b(i, j) = math::sqr(u(i, j)) + math::sqr(v(i, j));
          c1(i, j) = b(i, j)*u(i, j);
          c2(i, j) = b(i, j)*v(i, j);
        }
      }

      /* Solve GVF = (u,v) */
      matd Lu(size()), Lv(size());
      for (int it = 0; it < iters; ++it) {
        /* corners */
        int n = width() - 1;
        int m = height() - 1;
        Lu(0, 0) = (2 * u(1, 0) + 2 * u(0, 1)) - 4 * u(0, 0);
        Lv(0, 0) = (2 * v(1, 0) + 2 * v(0, 1)) - 4 * v(0, 0);
        Lu(n, m) = (2 * u(n - 1, m) + u(n, m - 1)) - 4 * u(n, m);
        Lv(n, m) = (2 * v(n - 1, m) + v(n, m - 1)) - 4 * v(n, m);
        Lu(n, 0) = (2 * u(n - 1, 0) + 2 * u(n, 1)) - 4 * u(n, 0);
        Lv(n, 0) = (2 * v(n - 1, 0) + 2 * v(n, 1)) - 4 * v(n, 0);
        Lu(0, m) = (2 * u(1, m) + 2 * u(0, m - 1)) - 4 * u(0, m);
        Lv(0, m) = (2 * v(1, m) + 2 * v(0, m - 1)) - 4 * v(0, m);

        /* interior Lu, Lv*/
        double* uCur, *uPrev, *uNext;
        double* vCur, *vPrev, *vNext;
        double* curLu = Lu.data();
        double* curLv = Lv.data();
        for (int j = 1; j < m; ++j) {
          uCur = u.line(j) + 1;
          uPrev = u.line(j - 1) + 1;
          uNext = u.line(j + 1) + 1;
          vCur = v.line(j) + 1;
          vPrev = v.line(j - 1) + 1;
          vNext = v.line(j + 1) + 1;
          curLu = Lu.line(j) + 1;
          curLv = Lv.line(j) + 1;
          for (int i = 1; i < n; ++i) {
            *curLu++ = (*(uCur - 1) + *uPrev++ + *(uCur + 1) + *uNext++) - 4 * (*uCur);
            *curLv++ = (*(vCur - 1) + *vPrev++ + *(vCur + 1) + *vNext++) - 4 * (*vCur);
            ++uCur;
            ++vCur;
          }
        }
        /* left and right columns */
        for (int j = 1; j < m; ++j) {
          Lu(0, j) = (u(0, j - 1) + 2 * u(1, j) + u(0, j + 1)) - 4 * u(0, j);
          Lv(0, j) = (v(0, j - 1) + 2 * v(1, j) + v(0, j + 1)) - 4 * v(0, j);
          Lu(n, j) = (u(n, j - 1) + 2 * u(n - 1, j) + u(n, j + 1)) - 4 * u(n, j);
          Lv(n, j) = (v(n, j - 1) + 2 * v(n - 1, j) + v(n, j + 1)) - 4 * v(n, j);
        }

        /* top and bottom rows */
        for (int i = 1; i < n; ++i) {
          Lu(i, 0) = (u(i - 1, 0) + 2 * u(i, 1) + u(i + 1, 0)) - 4 * u(i, 0);
          Lv(i, 0) = (v(i - 1, 0) + 2 * v(i, 1) + v(i + 1, 0)) - 4 * v(i, 0);
          Lu(i, m) = (u(i - 1, m) + 2 * u(i, m - 1) + u(i + 1, m)) - 4 * u(i, m);
          Lv(i, m) = (v(i - 1, m) + 2 * v(i, m - 1) + v(i + 1, m)) - 4 * v(i, m);
        }

        /* Update GVF  */
        double* curU = u.data();
        double* curV = v.data();
        double* curb = b.data();
        double* curC1 = c1.data();
        double* curC2 = c2.data();
        curLu = Lu.data();
        curLv = Lv.data();
        for (int i = 0, n = width()*height(); i < n; ++i) {
          *curU = (1.0 - *curb) * (*curU) + mu * (*curLu++) + *curC1++;
          *curV = (1.0 - *curb) * (*curV) + mu * (*curLv++) + *curC2++;
          ++curU;
          ++curV;
          ++curb;
        }
      }
    }

    Image<T> gvf(double mu, int iters, T(*uniteFunc)(T, T)) {
      Image<T> u(size()), v(size());
      gvf(mu, iters, u, v);

      return Image<T>::unite(u, v, uniteFunc);
    }

    static Image<T> makeSobelKernel() {
      Image<T> kernel(3, 3);

      kernel(0, 0) = -1.0; kernel(0, 1) = 0.0; kernel(0, 2) = 1.0;
      kernel(1, 0) = -2.0; kernel(1, 1) = 0.0; kernel(1, 2) = 2.0;
      kernel(2, 0) = -1.0; kernel(2, 1) = 0.0; kernel(2, 2) = 1.0;

      return kernel;
    }
    static Image<T> makeLaplace4Kernel() {
      Image<T> kernel(3, 3);
      kernel(0, 0) = 0; kernel(0, 1) = -1;  kernel(0, 2) = 0;
      kernel(1, 0) = -1; kernel(1, 1) = 5; kernel(1, 2) = -1;
      kernel(2, 0) = 0; kernel(2, 1) = -1;  kernel(2, 2) = 0;

      return kernel;
    }
    static Image<T> makeLaplace8Kernel() {
      Image<T> kernel(3, 3);
      kernel(0, 0) = 1; kernel(0, 1) = 1;  kernel(0, 2) = 1;
      kernel(1, 0) = 1; kernel(1, 1) = -8; kernel(1, 2) = 1;
      kernel(2, 0) = 1; kernel(2, 1) = 1;  kernel(2, 2) = 1;

      return kernel;
    }
    static Image<T> makeLaplace12Kernel() {
      Image<T> kernel(3, 3);
      /*а это сразу сложение изображения с лаплассианом*/
      kernel(0, 0) = 1; kernel(0, 1) = 2;  kernel(0, 2) = 1;
      kernel(1, 0) = 2; kernel(1, 1) = -12; kernel(1, 2) = 2;
      kernel(2, 0) = 1; kernel(2, 1) = 2;  kernel(2, 2) = 1;

      return kernel;
    }
    static Image<T> makeAveragingKernel(int radius) {
      Image<T> kernel(radius * 2 + 1, radius * 2 + 1);
      kernel.clear(1.0 / ((radius * 2 + 1) * (radius * 2 + 1)));
      return kernel;
    }
    static Image<T> makeGaussianForCannykernel() {
      Image<T> kernel(5, 5);

      kernel(0, 0) = 2.0 / 159; kernel(0, 1) = 4.0 / 159;  kernel(0, 2) = 5.0 / 159;  kernel(0, 3) = 4.0 / 159;  kernel(0, 4) = 2.0 / 159;
      kernel(1, 0) = 4.0 / 159; kernel(1, 1) = 9.0 / 159;  kernel(1, 2) = 12.0 / 159; kernel(1, 3) = 9.0 / 159;  kernel(1, 4) = 4.0 / 159;
      kernel(2, 0) = 5.0 / 159; kernel(2, 1) = 12.0 / 159; kernel(2, 2) = 15.0 / 159; kernel(2, 3) = 12.0 / 159; kernel(2, 4) = 5.0 / 159;
      kernel(3, 0) = 4.0 / 159; kernel(3, 1) = 9.0 / 159;  kernel(3, 2) = 12.0 / 159; kernel(3, 3) = 9.0 / 159;  kernel(3, 4) = 4.0 / 159;
      kernel(4, 0) = 2.0 / 159; kernel(4, 1) = 4.0 / 159;  kernel(4, 2) = 5.0 / 159;  kernel(4, 3) = 4.0 / 159;  kernel(4, 4) = 2.0 / 159;

      return kernel;
    }
    static Image<T> makeGaussianKernel(int radius, double sigma, bool normalize = false) {
      int length = radius * 2 + 1;
      double denom = 2.0*sigma*sigma, sum = 0;
      Image<T> kernel(length, length);
      for (int i = 0; i<length; ++i) {
        for (int j = 0; j<length; ++j) {
          kernel(i, j) = std::exp(-(math::sqr(i - radius) + math::sqr(j - radius)) / denom) / (math::Pi*denom);
          sum += kernel(i, j);
        }
      }

      if (normalize) {
        for (int i = 0; i<length; ++i) {
          for (int j = 0; j<length; ++j) {
            kernel(i, j) /= sum;
          }
        }
      }
      return kernel;
    }
  };

  typedef Image<double> matd;
  typedef Image<bool> matb;
  typedef Image<int> mati;
}
