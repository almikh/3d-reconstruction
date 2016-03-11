#include <model-creator.h>

namespace rn {
  ModelCreator::ModelCreator():
    using_texturing(""),
    merge_models(false)
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

  void ModelCreator::setPointsMover(const QString& mode) {
		Q_UNUSED(mode);
    Q_ASSERT(false);
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
