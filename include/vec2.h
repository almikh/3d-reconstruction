#ifndef VEC2_H_INCLUDED__
#define VEC2_H_INCLUDED__

template<class T>
class vec2 {
public:
  union {
    struct { T x, y; };
    T coords[2];
  };

  typedef T value_type;

  vec2() : x(0), y(0) {}
  vec2(const vec2<T>& vertex) = default;

  vec2(T _x, T _y) : x(_x), y(_y) {}

  template<class S>
  vec2(const vec2<S>& vertex) : x(T(vertex.x)), y(T(vertex.y)) {}

  vec2& operator=(const vec2<T>& rhs) {
    if (this == &rhs) return *this;
    x = rhs.x;
    y = rhs.y;

    return *this;
  }
  template<class T2>
  vec2& operator=(const vec2<T2>& rhs) {
    x = T(rhs.x);
    y = T(rhs.y);

    return *this;
  }

  vec2<T>& operator/=(const T& rhs) {
    x /= rhs;
    y /= rhs;
    return *this;
  }
  vec2<T>& operator*=(const T& rhs) {
    x *= rhs;
    y *= rhs;
    return *this;
  }
  vec2<T>& operator+=(const T& rhs) {
    x += rhs;
    y += rhs;
    return *this;
  }
  vec2<T>& operator+=(const vec2<T>& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
  vec2<T>& operator-=(const T& rhs) {
    x -= rhs;
    y -= rhs;
    return *this;
  }
  vec2<T>& operator-=(const vec2<T>& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  vec2& swap() {
    T temp = x;
    x = y;
    y = temp;
    return *this;
  };
  vec2<T>& normalize() {
    static_assert(std::is_floating_point<T>::value, "Value with floating point required.");

    double t = sqrt(double(x*x + y*y));
    x /= t;
    y /= t;
    return *this;
  }

  /* находится ли внутри треугольника */
  bool in(const vec2<T>& a, const vec2<T>& b, const vec2<T>& c) {
    if ((y - a.y)*(b.x - a.x) - (x - b.x)*(b.y - a.y)>0) return false;
    if ((y - b.y)*(c.x - b.x) - (x - c.x)*(c.y - b.y)>0) return false;
    if ((y - c.y)*(a.x - c.x) - (x - a.x)*(a.y - c.y)>0) return false;
    return true;
  }

  /* скалярное произведение */
  double dot(const vec2<T>& rhs) {
    return (x*rhs.x) + (y*rhs.y);
  }
  double length() const {
    return sqrt(x*x + y*y);
  }
  double dist(const vec2<T>& rhs) const {
    return sqrt(double((x - rhs.x)*(x - rhs.x) + (y - rhs.y)*(y - rhs.y)));
  }
  double sqrDist(const vec2<T>& rhs) const {
    return double((x - rhs.x)*(x - rhs.x) + (y - rhs.y)*(y - rhs.y));
  }
  double angle(const vec2<T>& rhs) { //угол между ... и second
    return acos(dot(rhs) / (length()*rhs.length()));
  }

  template<class T2> vec2<T2> normalized() const {
    T2 t = sqrt(x*x + y*y);
    return vec2<T2>(x*1.0 / t, y*1.0 / t);
  }

  template<class T2> vec2<T2> to() {
    return vec2<T2>(T2(x), T2(y));
  }

  static vec2<T> i; /* Базисный вектор i */
  static vec2<T> j; /* Базисный вектор j */
};

typedef vec2<double> vec2d;
typedef vec2<int> vec2i;

template<class T> vec2<T> vec2<T>::i = vec2<T>(1, 0);
template<class T> vec2<T> vec2<T>::j = vec2<T>(0, 1);

#endif
