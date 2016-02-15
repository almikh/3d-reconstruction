#include "cylindical-model-creator.h"
#include <default-points-mover.h>
#include <ellipse-creator.h>
#include <algebra.h>
#include <line.h>
#include <aabb.h>

namespace rn {
	int CylindricalModelCreator::min_available_distance = 32;

	CylindricalModelCreator::CylindricalModelCreator():
		clicks_counter_(0),
		points_mover_(new DefaultPointsMover()),
		basis_(3, vec2i(0, 0))
	{

	}
	
	void CylindricalModelCreator::onMousePress(Qt::MouseButton button) {
		ModelCreator::onMousePress(button);
	
		if (buttons_[Qt::MouseButton::LeftButton]) {
			if (clicks_counter_ < 3) {
				basis_[clicks_counter_++] = offset_mouse_;
				switch (clicks_counter_) {
					//case 1:
					//return FORMING_OBJECT;
				case 2:
					defInclinationAngle();
					break;
				case 3:
					toProcessBasis();
					break;
				}
			}
			else {
				//if (smooth == RN_SMOOTH_USING_ABERAGING) smoothUsingAveraging();
				//else if (smooth == RN_SMOOTH_USING_LSM) smoothUsingLSM(16);

				current_mesh_->auxiliary_points = prev_layers_;
				if (triangulate_first_layer) current_mesh_->triangulateFirstLayer();
				if (triangulate_last_layer) current_mesh_->triangulateLastLayer();
				current_mesh_->updateNormals();

				goToOverview();
				if (merge_models && !data_->meshes.isEmpty()) {
					for (;;) {
						// first - меш, second - минимальное расстояние между ним и current_mesh_
						QPair<Mesh::HardPtr, double> target = qMakePair(nullptr, Double::max());
						for (auto mesh : data_->meshes) {
							double distance = mesh->dist(*current_mesh_);
							if (distance < target.second && distance < min_available_distance) {
								target.first = mesh;
								target.second = distance;
							}
						}

						if (target.first == nullptr) { // нечего объединять
							data_->addMesh(current_mesh_);
							break;
						}

						data_->meshes.removeOne(target.first);
						*current_mesh_ = Mesh::unite(*target.first, *current_mesh_);

						//target.first->unite(*current_mesh_);
						//current_mesh_ = target.first;
					}
				}
				
				data_->addMesh(current_mesh_);

				current_mesh_.reset();
			}
		}
	}

	void CylindricalModelCreator::onMouseRelease(Qt::MouseButton button) {
		ModelCreator::onMouseRelease(button);

	}

	void CylindricalModelCreator::onMouseMove(int x, int y) {
		ModelCreator::onMouseMove(x, y);

		if (data_) {
			offset_mouse_ = mouse_ - data_->screenCenter();
		}

		if (clicks_counter_ >= 3) {
			goToUpdateMesh();
		}
	}

	void CylindricalModelCreator::correctStep() {
		auto layer = prev_layers_.back();
		auto line = Line<int>(layer[0], layer[1]);
		auto n = line.normal().to<double>().normalize() * static_cast<double>(data_->step);
		vec2d other = (offset_mouse_ - prev_layers_.back()[0]).to<double>();
		if (n.angle(other) > math::Pi_2) {
			data_->invertStep();
		}
	}

	void CylindricalModelCreator::updateMover() {
		points_mover_->setLookAhead(false);
		points_mover_->setGradient(data_->gvf);
		points_mover_->setGradientDir(data_->gvf_dir);
		points_mover_->setPrevLayers(prev_layers_);
	}

	void CylindricalModelCreator::toProcessBasis() {
		// Создаем новый объект */
		//data_->backup();
		current_mesh_.reset(new Mesh());
		current_mesh_->texture_id = data_->texture();

		/* Определяем кручение эллипса */
		double a = (basis_[0] - basis_[1]).length();
		double b = Line<int>(basis_[0], basis_[1]).dist(basis_[2]);
		rotation_angle_ = std::abs(math::Pi_2 - std::acos(b / (a / 2.0)));
		prev_layers_.push_back(basis_);

		updateMover();
		toSpecify(prev_layers_.back());

		auto ellipse = createLayerPoints(prev_layers_.back());
		current_mesh_->addLayer(ellipse.toStdVector());

		data_->setFirstLayer(createEllipseByThreePoints(prev_layers_.front()));

		/* Поворот камеры для 'кручения эллипса' */
		glLoadIdentity();
		auto rotation_axis = (basis_[0] - basis_[1]).to<double>().normalize();
		glRotated(qRadiansToDegrees(rotation_angle_), rotation_axis.x, rotation_axis.y, 0);
	}

	void CylindricalModelCreator::goToOverview() {
		clicks_counter_ = 0;
		prev_layers_.clear();
		rotation_angle_ = 0.0;
		inclination_angle_ = 0.0;
		basis_.fill(vec2i(0, 0), basis_.size());
	}

	void CylindricalModelCreator::defInclinationAngle() {
		inclination_angle_ = vec2d::i.angle((basis_[1] - basis_[0]).to<double>());
		if (basis_[1].y < basis_[0].y) {
			inclination_angle_ = -inclination_angle_;
		}

		if (abs(inclination_angle_) > math::Pi_2) {
			inclination_angle_ -= math::Pi * math::sign(static_cast<int>(inclination_angle_));
		}
	}

	void CylindricalModelCreator::toSpecify(QVector<vec2i>& points, vec2d normal, double length, vec2d* dir) {
		//Q_ASSERT(points.size() == 2);

		// нужен для пересчета сцены модели в координаты изображения, где (0, 0) - в нижнем левом углу
		vec2i offset = data_->screenCenter() - data_->offsets;

		vec2d change_dir;
		if (!dir) {
			change_dir = (points[0] - points[1]).to<double>().normalize();
		}

		points_mover_->setChangeDir(change_dir);
		points_mover_->setGrowthDir(normal);
		points_mover_->setGrowthLength(length);
		points_mover_->setOffset(offset);
		points_mover_->setLookAhead(true);

		points_mover_->move(points);
	}
	
	std::vector<vec2d> CylindricalModelCreator::defTexCoord(QVector<vec3i> src, const QVector<vec2i>& base) {
		vec2i offset = data_->screenCenter() - data_->offsets;
		auto axis = vec3d((base[0] - base[1]).to<double>(), ProjectionPlane::OXY, 0).normalize();

		vec3i center = createAABB<int>(src.begin(), src.end()).center();
		auto rotation_matax = mat3d::rotation(axis, rotation_angle_);
		for (auto &e : src) {
			e -= center;
			e = (e.to<double>()*rotation_matax).to<int>() + center;
		}

		std::vector<vec2d> uv;
		int T = src.size() / 2;
		uv.reserve(src.size());
		for (int i = 0; i<src.size(); ++i) {
			vec2d coord;
			if (src[i].z >= 0) {
				auto e = src[i];
				coord.x = double(e.x + offset.x) / data_->width();
				coord.y = 1.0 - double(e.y + offset.y) / data_->height();
			}
			else {
				int ind = i + T; //это если отражать зеркально
				if (ind >= src.size()) ind -= src.size();

				auto e = src[ind];
				coord.x = double(e.x + offset.x) / data_->width();
				coord.y = 1.0 - double(e.y + offset.y) / data_->height();
			}

			uv.push_back(coord);
		}

		return uv;
	}

	void CylindricalModelCreator::updateMesh() {
		prev_layers_.resize(1);
		auto layer = prev_layers_.back();
		double dist = Line<int>(layer[0], layer[1]).dist(offset_mouse_);

		current_mesh_->clear();

		while (std::abs(dist) >= abs(data_->step)) {
			correctStep();

			auto layer = prev_layers_.back();
			auto n = Line<int>(layer[0], layer[1]).normal().to<double>().normalize();
			
			toSpecify(layer, n, data_->step);
			if ((layer[0] - layer[1]).length() < 1.0) {
				break; // слой выродился в точку
			}


			layer[2] = layer[0] + (basis_[2] - basis_[0]);

			auto ellipse = createLayerPoints(layer);
			current_mesh_->addLayer(ellipse.toStdVector());
			prev_layers_.push_back(layer);

			if (using_texturing) {
				auto uv = defTexCoord(ellipse, layer);
				current_mesh_->addTexCoords(uv);
			}

			layer = prev_layers_.back();
			dist = Line<int>(layer[0], layer[1]).dist(offset_mouse_);
		}

		data_->setLastLayer(createEllipseByThreePoints(prev_layers_.back()));
		current_mesh_->updateNormals();
	}

	void CylindricalModelCreator::goToUpdateMesh() {
		auto points = prev_layers_.back();
		double diff = Line<int>(points[0], points[1]).dist(offset_mouse_);

		if (diff >= data_->step) {
			updateMesh();
		}
	}

	QVector<vec3i> CylindricalModelCreator::createLayerPoints(const QVector<vec2i>& key_points) {
		Q_ASSERT(key_points.size() == 3);
		Q_ASSERT(data_);

		vec3i center((key_points[0] + key_points[1]) / 2, ProjectionPlane::OXY);
		int a = (key_points[0] - key_points[1]).length() / 2;

		EllipseCreator<int> creator(a, a);
		auto source = creator.create(data_->slices);

		mat3d rotation(mat3d::rotZ(inclination_angle_)); // матрица преобразования точки

		QVector<vec3i> ellipse;
		ellipse.reserve(source.size());
		for (auto &e : source) {
			vec3i vertex(e, ProjectionPlane::OXZ);
			vertex = (vertex.to<double>() * rotation).to<int>() + center;

			ellipse.push_back(vertex);
		}

		return ellipse;
	}

	QVector<vec2i> CylindricalModelCreator::createEllipseByThreePoints(const QVector<vec2i>& points) {
		Q_ASSERT(points.size() == 3);
		Q_ASSERT(data_);

		int a = (points[0] - points[1]).length() / 2; // главная полуось
		int b = Line<int>(points[0], points[1]).dist(points[2]); // малая полуось

		EllipseCreator<int> creator(a, b);
		creator.setAngle(inclination_angle_);
		creator.setCenter((points[0] + points[1]) / 2);
		return creator.create(data_->slices);
	}

	void CylindricalModelCreator::render() {
		if (clicks_counter_ > 0) {
			bool draw_line = false;
			bool draw_circle = false;

			QVector<vec2i> line, circle;
			line.push_back(basis_[0]);
			if (clicks_counter_ == 1) {
				line.push_back(offset_mouse_);
				draw_line = true;
			}
			else {
				line.push_back(basis_[1]);
				draw_line = true;

				QVector<vec2i> temp(basis_);
				if (clicks_counter_ == 2) temp[2] = offset_mouse_;
				circle = createEllipseByThreePoints(temp);

				draw_circle = true;
			}

			glPushMatrix();
			glLoadIdentity();
			if (draw_line) {
				glLineWidth(3);
				glColor3d(0, 0, 1);
				glBegin(GL_LINES);
				glVertex3d(line.front().x, line.front().y, -100);
				glVertex3d(line.back().x, line.back().y, -100);
				glEnd();
			}

			if (draw_circle) {
				glLineWidth(3);
				glColor3d(1, 0, 0);
				glBegin(GL_LINE_LOOP);
				for (auto &e : circle) glVertex3d(e.x, e.y, 100);
				glEnd();
			}

			glPopMatrix();

			if (current_mesh_) {
				current_mesh_->render(false);
			}
			
			glLineWidth(1);
			glColor3d(1, 1, 1);
		}
	}
}
