#ifndef CYLINDICAL_MODEL_CREATOR_H_INCLUDED__
#define CYLINDICAL_MODEL_CREATOR_H_INCLUDED__

#include <model-creator.h>
#include <points-mover.h>

namespace rn {
  class CylindricalModelCreator : public ModelCreator {
  protected:
    vec2i coord_;
    vec2i offset_mouse_; // центр системы координат - центр изображения
    int clicks_counter_; // подсчитывает клики по изображению
    double inclination_angle_; // угол наклона эллипса вокруг оси z
    double rotation_angle_; // угол кручения эллипса вокруг оси x
    Mesh::HardPtr current_mesh_;

    std::shared_ptr<PointsMover> points_mover_;

    QVector<QVector<vec2i>> prev_layers_; // предыдущие слои (сформированные)
    QVector<vec2i> basis_; // основание модели (задается первыми кликами)

  private:
    void correctStep();
    void updateMover();
    void goToOverview();
    void toProcessBasis();
    void defInclinationAngle();
    void toSpecify(QVector<vec2i>& points, vec2d normal = vec2d(0, 0), double length = 0.0, vec2d* dir = nullptr);

    std::vector<vec2d> defTexCoord(QVector<vec3i> src, const QVector<vec2i>& base);

    void updateMesh();
    void goToUpdateMesh();

    QVector<vec3i> createLayerPoints(const QVector<vec2i>& key_points); // создает слой искомой модели
    QVector<vec2i> createEllipseByThreePoints(const QVector<vec2i>& points);

  public:
    static int min_available_distance;
    // int smooth;

    CylindricalModelCreator();

    void setPointsMover(const QString& mode) override;

    void place(Mesh::HardPtr mesh, int radius) override;
    Mesh::HardPtr createMeshFromLayers(const QVector<QVector<vec2i>>& layers) override;

    void onMousePress(Qt::MouseButton button) override;
    void onMouseRelease(Qt::MouseButton button) override;
    void onMouseMove(int x, int y) override;
    void OnInterruptRequest() override;

    void render() override;
  };
}


#endif // CYLINDICAL_MODEL_CREATOR_H_INCLUDED__
