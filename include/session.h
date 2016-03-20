#ifndef SESSION_H_INCLUDED__
#define SESSION_H_INCLUDED__

#include <QtOpenGL>
#include <QVector>
#include <QImage>
#include <memory>
#include <QPair>

#include <vec2.h>
#include <mesh.h>
#include <image.h>

class QGLWidget;
#define MIN_SCENE_HEIGHT	600
#define MIN_SCENE_WIDTH		900

namespace rn {
  struct Session {
  public:
    typedef std::shared_ptr<Session> HardPtr;

  private:
    QGLWidget* parent_;
    QVector<QVector<Mesh::HardPtr>> backups_;

    GLuint texture_;
    GLuint gvf_texture_;

    void checkOpenGLErrors();

  public:
    vec2i offsets;
    vec2i screen_size;
    int slices, step; // параметры детализации меша

    QImage image;
    std::shared_ptr<ip::Image<double>> gvf;
    std::shared_ptr<ip::Image<double>> gvf_dir;

    QVector<Mesh::HardPtr> meshes;

    QVector<vec2i> first_layer;
    QVector<vec2i> last_layer;

  public:
    QList<Mesh::HardPtr> selected_meshes;

    Session() = default;
    Session(const QImage& image, QGLWidget* parent);
    ~Session();

    void commit();
    void rollback();
    bool hasBackups() const;

    void invertStep();
    void addMesh(Mesh::HardPtr mesh);
    void setLastLayer(const QVector<vec2i>& layer);
    void setFirstLayer(const QVector<vec2i>& layer);

    int width() const;
    int height() const;
    vec2i screenCenter() const;

    GLuint texture() const;
    GLuint gvfTexture() const;
  };
}

#endif // SESSION_H_INCLUDED__
