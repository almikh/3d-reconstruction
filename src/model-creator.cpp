#include <model-creator.h>

namespace rn {
  ModelCreator::ModelCreator():
    using_texturing(false),
    creating_mode(Normal),
    texturing_mode(Mirror)
  {

  }

  ModelCreator::~ModelCreator() {

  }

  std::shared_ptr<rn::Session> ModelCreator::data() const {
    return data_;
  }

  void ModelCreator::setSessionData(std::shared_ptr<rn::Session> data) {
    data_ = data;
  }

  void ModelCreator::setPointsMover(const CreatingMode& mode) {
    creating_mode = mode;
  }

  void ModelCreator::OnInterruptRequest() {

  }

  void ModelCreator::onMouseMove(int x, int y) {
    mouse_.x = x;
    mouse_.y = y;
  }

  void ModelCreator::onMousePress(Qt::MouseButton button) {
    buttons_[button] = true;
  }

  void ModelCreator::onMouseRelease(Qt::MouseButton button) {
    buttons_[button] = false;
  }

  void ModelCreator::place(Mesh::HardPtr mesh, int radius) {
    Q_UNUSED(radius);
    Q_UNUSED(mesh);

    Q_ASSERT(false);
  }

  Mesh::HardPtr ModelCreator::createMeshFromLayers(const QVector<QVector<vec2i>>& layers) {
    Q_UNUSED(layers);

    Q_ASSERT(false);
    return nullptr;
  }
}
