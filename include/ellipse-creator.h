#ifndef ELLIPSE_CREATOR_H_INCLUDED__
#define ELLIPSE_CREATOR_H_INCLUDED__

#include <vec2.h>

namespace rn {
  template<class T>
  class EllipseCreator {
    vec2<T> center_;
    double angle_;
    T param_a_;
    T param_b_;

  public:
    // a, b - полуоси
    EllipseCreator(T a, T b) :
      angle_(0.0),
      param_a_(a),
      param_b_(b)
    {

    }

    void setCenter(T x, T y) {
      center_.x = x;
      center_.y = y;
    }

    void setCenter(const vec2<T>& center) {
      setCenter(center.x, center.y);
    }

    void setAngle(double angle) { // угол поворота эллипса вокруг Oz (т.е. по/против часовой стрелке)
      angle_ = angle;
    }

    QVector<vec2<T>> create(int count) { // Создает эллипс, образуемый указанным количеством точек
      double _cos = cos(angle_), _sin = sin(angle_);
      double step = 2.0 * math::Pi / count;

      QVector<vec2<T>> dst;
      dst.reserve(count);
      for (double angle = 0.0; angle < math::Pi * 2.0; angle += step) {
        double t1 = param_a_*cos(angle), t2 = param_b_*sin(angle);
        vec2<T> point(T(t1*_cos - t2*_sin), T(t1*_sin + t2*_cos));
        dst.push_back(point + center_);
      }

      return dst;
    }
  };
}

#endif // ELLIPSE_CREATOR_H_INCLUDED__
