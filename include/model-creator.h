#ifndef MODEL_CREATOR_H_INCLUDED__
#define MODEL_CREATOR_H_INCLUDED__
#include <memory>
#include <QVector>
#include <QObject>

#include <vec2.h>
#include <session.h>
#include <model-creator.h>

namespace rn {
  using layer_t = QVector<vec2i>;

  class ModelCreator: public QObject {
    Q_OBJECT

  protected:
    vec2i mouse_;
    QMap<Qt::MouseButton, bool> buttons_;
    rn::Session::HardPtr data_; // данные текущей сессии

  public:
    bool using_texturing; // текстурировать ли создаваемую модель
    bool triangulate_first_layer; // создавать ли "нижнюю крышку" модели
    bool triangulate_last_layer; // создавать ли "верхнюю крышку" модели
    bool merge_models; // "склеивать" близкие модели

  public:
    ModelCreator();
    virtual ~ModelCreator();

    std::shared_ptr<rn::Session> data() const;
    void setSessionData(std::shared_ptr<rn::Session> data);

    virtual void OnInterruptRequest();
    virtual void onMouseMove(int x, int y);
    virtual void onMousePress(Qt::MouseButton button);
    virtual void onMouseRelease(Qt::MouseButton button);

    virtual void place(Mesh::HardPtr mesh, int radius);
    virtual Mesh::HardPtr createMeshFromLayers(const QVector<QVector<vec2i>>& layers);

    virtual void render() {};

  signals:
    void signalModelCreated(Mesh::HardPtr mesh);
  };
}

#endif // MODEL_CREATOR_H_INCLUDED__
