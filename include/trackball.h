#ifndef TRACKBALL_H_INCLUDED__
#define TRACKBALL_H_INCLUDED__

#include <memory>
#include <QPoint>
#include <QVector3D>
#include <QMatrix4x4>

namespace rn
{
  class Trackball {
    int flags_;
    float scale_;
    bool is_clicked_;
    QSize scene_size_;
    QVector3D end_vector_;
    QVector3D start_vector_;
    QMatrix4x4 cur_rotation_;
    QMatrix4x4 last_rotation_;

    QVector3D mapToSphere(const QPoint& point) const;
    QMatrix4x4 matrixConversion(const QVector3D& pan, const QQuaternion& rot);

  public:
    typedef std::shared_ptr<Trackball> HardPtr;

  public:
    enum {
      VerticalInverse = 1
    };

    Trackball(int flags = 0, const QSize& sceneSize = QSize());

    const float* getMat() const;
    bool isClicked() const;

    void setSceneSize(const QSize& size);
    template<class T> void setLastMatrix(T* data) {
      static float temp[16];
      for (int i = 0; i < 16; ++i) {
        temp[i] = static_cast<float>(data[i]);
      }

      last_rotation_ = QMatrix4x4(temp).transposed();
    }

    void release();
    void click(int x, int y);
    QMatrix4x4 rotate(int x, int y);
  };
}

#endif // TRACKBALL_H_INCLUDED__
