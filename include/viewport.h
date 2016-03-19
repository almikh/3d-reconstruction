#ifndef VIEWPORT_H_INCLUDED__
#define VIEWPORT_H_INCLUDED__

#include <memory>
#include <QGLWidget>
#include <QtOpenGL>
#include <QtGui>

#include <vec3.h>
#include <session.h>
#include <trackball.h>
#include <model-creator.h>

namespace rn
{
  class Trackball;

  struct triangle {
    vec3d vertices[3];

    triangle() = default;
    triangle(vec3d x, vec3d y, vec3d z);

    vec3d& operator[](int index);
    vec3d operator[](int index) const;
  };

  using mesh_t = QVector<triangle>;

  class Viewport : public QGLWidget {
    Q_OBJECT

  protected:
    QSize scene_size_;
    rn::Session::HardPtr session_;

    mesh_t makeCone(double radius, double height, int slices = 36);
    mesh_t makeCylinder(double radius, double height, int slices = 36);

    void drawMesh(const mesh_t& mesh);
    void drawAxis();

  public:
    bool hide_image;
    bool show_force_field;
    Trackball::HardPtr trackball;
    QVector<QPoint> selected_area;
    std::shared_ptr<ModelCreator> model_creator;

    explicit Viewport(QWidget* parent = nullptr);
    virtual ~Viewport(void);

    void setSession(rn::Session::HardPtr session);

    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void paintGL() override;
    void updateView();

    void wheelEvent(QWheelEvent* event) override;
    void mouseMoveEvent(QMouseEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void mouseReleaseEvent(QMouseEvent* ev) override;

  signals:
    void signalWheelEvent(QWheelEvent* event);
    void signalMouseMoveEvent(QMouseEvent* event);
    void signalMousePressEvent(QMouseEvent* event);
    void signalMouseReleaseEvent(QMouseEvent* event);

  public slots:
    void updateGL();
  };
}

#endif // VIEWPORT_H_INCLUDED__
