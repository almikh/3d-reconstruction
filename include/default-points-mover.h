#ifndef DEFAULT_POINTS_MOVER_H_INCLUDED__
#define DEFAULT_POINTS_MOVER_H_INCLUDED__
#include <points-mover.h>

class DefaultPointsMover: public PointsMover {
public:
  DefaultPointsMover() = default;

  void move(QVector<vec2i>& points) override;
};

#endif // DEFAULT_POINTS_MOVER_H_INCLUDED__
