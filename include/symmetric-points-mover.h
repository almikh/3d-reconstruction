#ifndef SYMMETRIC_POINTS_MOVER_H_INCLUDED__
#define SYMMETRIC_POINTS_MOVER_H_INCLUDED__

#include <points-mover.h>

class SymmetricPointsMover : public PointsMover {
public:
  SymmetricPointsMover() = default;

  void move(QVector<vec2i>& points) override;
};

#endif // SYMMETRIC_POINTS_MOVER_H_INCLUDED__
