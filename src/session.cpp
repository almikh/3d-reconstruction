#include <session.h>
#include <cmath>

namespace rn {
  Session::Session(const QImage& image, QGLWidget* parent) :
    parent_(parent),
    slices(16),
    step(4),
    image(image)
  {
    Q_ASSERT(parent_);

    //image_ = image_.convertToFormat(QImage::Format_RGB888);

    ip::Image<double> source(image);
    ip::Image<double> u(source.size()), v(source.size());
    source.gvf(0.05, 64, u, v);

    gvf.reset(new ip::Image<double>(ip::Image<double>::unite(u, v, std::hypot).scale(0, 255))); // модуль поля потока градиента
    gvf_dir.reset(new ip::Image<double>(ip::Image<double>::unite(v, u, std::atan2))); // модуль поля потока градиента - `atan (v, u)`

    texture_ = parent_->bindTexture(image, GL_TEXTURE_2D);
    gvf_texture_ = parent_->bindTexture(gvf->toQImage(), GL_TEXTURE_2D);

    checkOpenGLErrors();
  }

  Session::~Session() {
    parent_->deleteTexture(texture_);
    parent_->deleteTexture(gvf_texture_);
  }

  void Session::invertStep() {
    step = -step;
  }

  void Session::addMesh(Mesh::HardPtr mesh) {
    meshes.push_back(mesh);
  }

  void Session::setLastLayer(const QVector<vec2i>& layer) {
    last_layer = layer;
  }

  void Session::setFirstLayer(const QVector<vec2i>& layer) {
    first_layer = layer;
  }

  int Session::width() const {
    return image.width();
  }

  int Session::height() const {
    return image.height();
  }

  vec2i Session::screenCenter() const {
    return screen_size / 2;
  }

  void Session::checkOpenGLErrors() {
    GLenum err_code;
    if ((err_code = glGetError()) != GL_NO_ERROR) {
      qDebug() << "OpenGL error:" << err_code << endl;
    }
  }

  GLuint Session::texture() const {
    return texture_;
  }

  GLuint Session::gvfTexture() const {
    return gvf_texture_;
  }
}
