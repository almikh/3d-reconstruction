#include <viewport.h>
#include <QTimer>
#include <QDebug>

namespace rn
{
  /* triangle */
  triangle::triangle(vec3d x, vec3d y, vec3d z) {
    vertices[0] = x;
    vertices[1] = y;
    vertices[2] = z;
  }

  vec3d& triangle::operator[](int index) {
    return vertices[index];
  }

  vec3d triangle::operator[](int index) const {
    return vertices[index];
  }

  /* Viewport */
  Viewport::Viewport(QWidget* parent) :
    QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer | QGL::Rgba), parent),
    trackball(new Trackball(/*Trackball::VerticalInverse*/)),
    hide_image(false)
  {
    setMouseTracking(true);
    setAutoBufferSwap(false);
  }

  Viewport::~Viewport(void) {

  }

  void Viewport::drawMesh(const mesh_t& mesh) {
    glBegin(GL_TRIANGLES);
    for (auto tri : mesh) {
      glVertex3dv(tri[0].coords);
      glVertex3dv(tri[1].coords);
      glVertex3dv(tri[2].coords);
    }
    glEnd();
  }

  void Viewport::drawAxis() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    static const double colors[3][3] = {
      { 1, 0, 0 },
      { 0, 1, 0 },
      { 0, 0, 1 }
    };
    static const double rotations[3][4] = {
      { 90, 0, 1, 0 },
      { -90, 1, 0, 0 },
      { 0, 0, 0, 0 }
    };

    glViewport(0, 0, 96, 96);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-3, 3, 3, -3, -3, 3);
    glMatrixMode(GL_MODELVIEW);

    double axe_height = 1.5;
    auto cylinder = makeCylinder(0.15, axe_height, 16);
    auto cone = makeCone(0.45, axe_height / 2, 16);
    for (int i = 0; i < 3; ++i) {
      glPushMatrix();
      glColor3dv(colors[i]);
      glRotated(rotations[i][0], rotations[i][1], rotations[i][2], rotations[i][3]);
      drawMesh(cylinder);
      glTranslated(0, 0, axe_height);
      drawMesh(cone);
      glPopMatrix();
    }

    glViewport(0, 0, width(), height());
    updateView();
  }

  mesh_t Viewport::makeCone(double r, double h, int slices) {
    mesh_t cone;

    const double end = 2 * 3.1415; // TODO
    const double step = end / slices;
    auto makeFirstTriangle = [&](double angle, double step) {
      vec3d v1(r * qSin(angle), r * qCos(angle), 0);
      vec3d v2(0.0, 0.0, h);
      vec3d v3(r * qSin(angle + step), r * qCos(angle + step), 0);

      return triangle(v1, v2, v3);
    };
    auto makeSecondTriangle = [&](double angle, double step) {
      vec3d v1(r * qSin(angle), 0.0, 0);
      vec3d v2(0.0, 0.0, 0.0);
      vec3d v3(r * qSin(angle + step), 0.0, 0);

      return triangle(v1, v2, v3);
    };

    for (float angle = 0.0; angle < end; angle += step) {
      cone.push_back(makeFirstTriangle(angle, step));
      cone.push_back(makeSecondTriangle(angle, step));
    }
    cone.push_back(makeFirstTriangle(end - step, step));
    cone.push_back(makeSecondTriangle(end - step, step));

    return cone;
  }

  mesh_t Viewport::makeCylinder(double r, double h, int slices) {
    mesh_t cylinder;

    const double end = 2 * 3.1415; // TODO
    const double step = end / slices;
    auto makeFirstTriangle = [r, h](double angle, double step) {
      vec3d v1(r * qSin(angle), r * qCos(angle), 0);
      vec3d v2(r * qSin(angle), r * qCos(angle), h);
      vec3d v3(r * qSin(angle + step), r * qCos(angle + step), 0);

      return triangle(v1, v2, v3);
    };
    auto makeSecondTriangle = [r, h](double angle, double step) {
      vec3d v1(r * qSin(angle), r * qCos(angle), h);
      vec3d v2(r * qSin(angle + step), r * qCos(angle + step), 0);
      vec3d v3(r * qSin(angle + step), r * qCos(angle + step), h);

      return triangle(v1, v2, v3);
    };
    auto makeBaseTriangle = [r](double angle, double height, double step) {
      vec3d v1(r * qSin(angle), r * qCos(angle), height);
      vec3d v2(0.0, 0.0, height);
      vec3d v3(r * qSin(angle + step), r * qCos(angle + step), height);

      return triangle(v1, v2, v3);
    };

    for (float angle = 0.0; angle < end; angle += step) {
      cylinder.push_back(makeFirstTriangle(angle, step));
      cylinder.push_back(makeSecondTriangle(angle, step));
      cylinder.push_back(makeBaseTriangle(angle, 0.0, step));
      cylinder.push_back(makeBaseTriangle(angle, h, step));
    }
    cylinder.push_back(makeFirstTriangle(end - step, step));
    cylinder.push_back(makeSecondTriangle(end - step, step));
    cylinder.push_back(makeBaseTriangle(end - step, 0.0, step));
    cylinder.push_back(makeBaseTriangle(end - step, h, step));

    return cylinder;
  }

  void Viewport::setSession(rn::Session::HardPtr session) {
    session_ = session;
    session_->screen_size = vec2i(scene_size_.width(), scene_size_.height());
    session_->offsets.x = (scene_size_.width() - session_->width()) / 2;
    session_->offsets.y = (scene_size_.height() - session_->height()) / 2;
  }

  void Viewport::initializeGL() {
    QGLWidget::initializeGL();
    qglClearColor(Qt::black);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

    //glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    //glFrontFace(GL_CW);

    //glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
    //glShadeModel(GL_SMOOTH );
  }

  void Viewport::resizeGL(int width, int height) {
    QGLWidget::resizeGL(width, height);
    scene_size_ = QSize(width, height);
    trackball->setSceneSize(scene_size_);

    if (session_) {
      session_->screen_size = vec2i(width, height);
      session_->offsets.x = (width - session_->width()) / 2;
      session_->offsets.y = (height - session_->height()) / 2;
    }

    glViewport(0, 0, width, height);
    updateView();
  }

  void Viewport::updateView() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-width() / 2, width() / 2, height() / 2, -height() / 2, -width(), width());

    glMatrixMode(GL_MODELVIEW);
  }

  void Viewport::paintGL() {
    QGLWidget::paintGL();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawAxis();

    if (session_) {
      glColor3d(1.0, 1.0, 1.0);
      glBindTexture(GL_TEXTURE_2D, session_->texture());

      if (!hide_image) { // рисуем текстуру изображения
        glPushMatrix();
        glLoadIdentity();
        glEnable(GL_TEXTURE_2D);

        int w = session_->width(), h = session_->height();

        glBegin(GL_QUADS);
        glTexCoord2d(0, 0); glVertex3i(-w / 2, h / 2, -w);
        glTexCoord2d(1, 0); glVertex3i(w / 2, h / 2, -w);
        glTexCoord2d(1, 1); glVertex3i(w / 2, -h / 2, -w);
        glTexCoord2d(0, 1); glVertex3i(-w / 2, -h / 2, -w);
        glEnd();

        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
      }

      model_creator->render();

      for (auto& mesh : session_->meshes) {
        if (!session_->selected_meshes.contains(mesh)) {
          mesh->render();
        }
      }

      for (auto& mesh : session_->selected_meshes) {
        mesh->render(vec3b(180, 0, 0), true, true);
      }
    }

    if (!selected_area.isEmpty()) {
      glPushMatrix();
      glLoadIdentity();

      Q_ASSERT(selected_area.size() == 2);

      auto origin = selected_area.front();
      auto other = selected_area.back();
      int z = scene_size_.width() - 1;

      glLineWidth(2);
      glColor3d(0, 1, 0);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glBegin(GL_QUADS);
      glVertex3i(origin.x(), origin.y(), z);
      glVertex3i(other.x(), origin.y(), z);
      glVertex3i(other.x(), other.y(), z);
      glVertex3i(origin.x(), other.y(), z);
      glEnd();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glColor3d(1, 1, 1);
      glLineWidth(1);

      glPopMatrix();
    }

    makeCurrent();
    swapBuffers();
  }

  void Viewport::updateGL() {
    QGLWidget::updateGL();
    paintGL();
  }

  void Viewport::wheelEvent(QWheelEvent* event) {
    emit signalWheelEvent(event);
  }

  void Viewport::mouseMoveEvent(QMouseEvent* event) {
    emit signalMouseMoveEvent(event);
  }

  void Viewport::mousePressEvent(QMouseEvent* event) {
    emit signalMousePressEvent(event);
  }

  void Viewport::mouseReleaseEvent(QMouseEvent* event) {
    emit signalMouseReleaseEvent(event);
  }
}
