#include <trackball.h>

namespace rn
{
  Trackball::Trackball(int flags, const QSize& sceneSize) :
    flags_(flags),
    scale_(1.0),
    is_clicked_(false),
    scene_size_(sceneSize),
    end_vector_(0.0f, 0.0f, 0.0f),
    start_vector_(0.0f, 0.0f, 0.0f)
  {

  }

  const float* Trackball::getMat() const {
    return cur_rotation_.data();
  }

  bool Trackball::isClicked() const {
    return is_clicked_;
  }

  void Trackball::setSceneSize(const QSize& size) {
    scene_size_ = size;
  }

  void Trackball::click(int x, int y) {
    if (flags_ & VerticalInverse) y = scene_size_.height() - y;
    is_clicked_ = true;
    //last_rotation_ = cur_rotation_;
    start_vector_ = mapToSphere(QPoint(x, y));
  }

  void Trackball::release() {
    is_clicked_ = false;
  }

  QMatrix4x4 Trackball::rotate(int x, int y) {
    if (flags_ & VerticalInverse) y = scene_size_.height() - y;
    end_vector_ = mapToSphere(QPoint(x, y));

    QVector3D cross = QVector3D::crossProduct(start_vector_, end_vector_);
    if (cross.length() > 1e-5) {
      QQuaternion q;
      q.setVector(cross);
      q.setScalar(QVector3D::dotProduct(start_vector_, end_vector_));

      QVector3D pan(0, 0, 0);
      cur_rotation_ = matrixConversion(pan, q);
      cur_rotation_ = cur_rotation_ * last_rotation_;
    }

    return cur_rotation_;
  }

  QVector3D Trackball::mapToSphere(const QPoint& point) const {
    QVector3D vector;
    float x = 2 * (point.x() * 1.0 / scene_size_.width()) - 1.0f;
    float y = 2 * (point.y() * 1.0 / scene_size_.height()) - 1.0f;
    float len = x * x + y * y;

    if (len > 1.0f) {
      float norm = 1.0f / sqrt(len);
      vector.setX(x * norm);
      vector.setY(y * norm);
      vector.setZ(0);
    }
    else {
      vector.setX(x);
      vector.setY(y);
      vector.setZ(sqrt(1.0f - len));
    }

    return vector;
  }

  QMatrix4x4 Trackball::matrixConversion(const QVector3D& pan, const QQuaternion& rot) {
    float n, s;
    float xs, ys, zs;
    float wx, wy, wz;
    float xx, xy, xz;
    float yy, yz, zz;
    n = rot.x() * rot.x() + rot.y() * rot.y() + rot.z()*rot.z() + rot.scalar()*rot.scalar();
    s = (n > 0.0f) ? 2.0f / n : 0.0f;

    xs = rot.x() * s;
    ys = rot.y() * s;
    zs = rot.z() * s;
    wx = rot.scalar()* xs;
    wy = rot.scalar() * ys;
    wz = rot.scalar() * zs;
    xx = rot.x()* xs;
    xy = rot.x() * ys;
    xz = rot.x() * zs;
    yy = rot.y() * ys;
    yz = rot.y() * zs;
    zz = rot.z() * zs;
    // rotation
    QMatrix4x4 m;
    m(0, 0) = 1.0f - (yy + zz);
    m(0, 1) = xy - wz;
    m(0, 2) = xz + wy;
    m(0, 3) = pan.x();

    m(1, 0) = xy + wz;
    m(1, 1) = 1.0f - (xx + zz);
    m(1, 2) = yz - wx;
    m(1, 3) = pan.y();

    m(2, 0) = xz - wy;
    m(2, 1) = yz + wx;
    m(2, 2) = 1.0f - (xx + yy);
    m(2, 3) = 0.0;

    m(3, 0) = 0.0f;
    m(3, 1) = 0.0f;
    m(3, 2) = 0.0f;
    m(3, 3) = 1.0f;

    return m;
  }
}
