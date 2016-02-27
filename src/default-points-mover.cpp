#include "default-points-mover.h"
#include <QtGlobal>
#include <cmath>

#include <algebra.h>

void DefaultPointsMover::move(QVector<vec2i>& points)  {
  // TODO Проверять параметры - заданы ли нужные

  for (int index = 0; index < 2/*points.size()*/; ++index) {
    int viewed = 1;
    vec2i& point = points[index];
    point += offset_ + rn::round(growth_dir_ * growth_length_).to<int>();
    Q_ASSERT(grad_->isCorrect(point.x, point.y));

    int ind;
    for (;;) {
      bool flag = false;
      double price = grad_->at(point);
      for (int i = -1; i <= 1; i += 2) {
        int nx = point.x + std::round(i * change_dir_.x * viewed);
        int ny = point.y + std::round(i * change_dir_.y * viewed);
        if (grad_->isCorrect(nx, ny)) {
          double temp = grad_->at(nx, ny);
          if (temp > price) {
            price = temp;
            flag = true;
            ind = i;
          }
        }
      }

      if (!flag) {
        if (look_ahead_) {
          ++viewed;
          if (viewed > 4) break;
        }
        else break;
      }
      else {
        point.x += std::round(ind * change_dir_.x * viewed);
        point.y += std::round(ind * change_dir_.y * viewed);
        viewed = 1;
      }
    }

    point -= offset_;
  }
}
