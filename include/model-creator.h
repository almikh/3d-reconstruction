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

  public:
    enum TexturingMode {
      Mirror,
      Cyclically
    };

    enum CreatingMode {
      Normal,
      Symmetrically
    };

  protected:
    vec2i mouse_;
    QMap<Qt::MouseButton, bool> buttons_;
    rn::Session::HardPtr data_; // данные текущей сессии

  public:
    int creating_mode;
    int texturing_mode;
    bool using_texturing; // текстурировать ли создаваемую модель (режим)

  public:
    ModelCreator();
    virtual ~ModelCreator();

    std::shared_ptr<rn::Session> data() const;
    void setSessionData(std::shared_ptr<rn::Session> data);
    virtual void setPointsMover(const CreatingMode& mode);

    virtual void OnInterruptRequest();
    virtual void onMouseMove(int x, int y);
    virtual void onMousePress(Qt::MouseButton button);
    virtual void onMouseRelease(Qt::MouseButton button);

    virtual void place(Mesh::HardPtr mesh, int radius);
    virtual Mesh::HardPtr createMeshFromLayers(const QVector<QVector<vec2i>>& layers);

    virtual void render() {};

  signals:
    void signalBeforeNewModelCreating();
  };
}

#endif // MODEL_CREATOR_H_INCLUDED__
