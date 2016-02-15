#ifndef CYLINDICAL_MODEL_CREATOR_H_INCLUDED__
#define CYLINDICAL_MODEL_CREATOR_H_INCLUDED__
#include <model-creator.h>
#include <points-mover.h>

namespace rn {
	class CylindricalModelCreator : public ModelCreator {
	protected:
		vec2i coord_;
		vec2i offset_mouse_; // ����� ������� ��������� - ����� �����������
		int clicks_counter_; // ������������ ����� �� �����������
		double inclination_angle_; // ���� ������� ������� ������ ��� z
		double rotation_angle_; // ���� �������� ������� ������ ��� x
		Mesh::HardPtr current_mesh_;

		std::shared_ptr<PointsMover> points_mover_;

		QVector<QVector<vec2i>> prev_layers_; // ���������� ���� (��������������)
		QVector<vec2i> basis_; // ��������� ������ (�������� ������� �������)

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

		QVector<vec3i> createLayerPoints(const QVector<vec2i>& key_points); // ������� ���� ������� ������
		QVector<vec2i> createEllipseByThreePoints(const QVector<vec2i>& points);

	public:
		static int min_available_distance;
		// int smooth;

		CylindricalModelCreator();

		//virtual void place(QVector<vec2i>& src, int radius) {};
		//virtual void place(QVector<QVector<vec2i>>& src, double* incAngle, double* rotAngle) {};
		//virtual Mesh* createMeshFromLayers(const QVector<layer_t>& layers, double inc_angle, double rot_angle) {
		//	return nullptr;
		//}

		void onMousePress(Qt::MouseButton button) override;
		void onMouseRelease(Qt::MouseButton button) override;
		void onMouseMove(int x, int y) override;

		void render() override;
	};
}


#endif // CYLINDICAL_MODEL_CREATOR_H_INCLUDED__
