#include "symmetric-points-mover.h"
#include <QtGlobal>
#include <cmath>

#include <algebra.h>

void SymmetricPointsMover::move(QVector<vec2i>& points)  {
  // TODO Проверять параметры - заданы ли нужные

  // сдвинули на уровень нового слоя
  points[0] += offset_ + rn::round(growth_dir_ * growth_length_).to<int>();
  points[1] += offset_ + rn::round(growth_dir_ * growth_length_).to<int>();

  auto get_price = [&](const vec2i& first, const vec2i& second) {
    return grad_->at(first) + grad_->at(second);
  };

  auto is_correct = [&](const vec2i& first, const vec2i& second) {
    return grad_->isCorrect(first.x, first.y) && grad_->isCorrect(second.x, second.y);
  };

  int viewed = 1;
  for (;;) {
    bool flag = false;
    double price = get_price(points[0], points[1]);

    vec2i first_pair[2], second_pair[2];

    // в сторону сужения
    first_pair[0] = points[0] + rn::round(change_dir_ * viewed).to<int>();
    first_pair[1] = points[1] - rn::round(change_dir_ * viewed).to<int>();

    // в сторону расширения
    second_pair[0] = points[0] - rn::round(change_dir_ * viewed).to<int>();
    second_pair[1] = points[1] + rn::round(change_dir_ * viewed).to<int>();

    if (is_correct(first_pair[0], first_pair[1]) && is_correct(second_pair[0], second_pair[1])) {
      double first_price = get_price(first_pair[0], first_pair[1]);
      double second_price = get_price(second_pair[0], second_pair[1]);

      if (qMax(first_price, second_price) > price) {
        flag = true;
        if (first_price > second_price) {
          points[0] = first_pair[0];
          points[1] = first_pair[1];
        }
        else {
          points[0] = second_pair[0];
          points[1] = second_pair[1];
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
  }

  points[0] -= offset_;
  points[1] -= offset_;
}
