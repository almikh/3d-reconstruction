#include "cylindical-model-creator.h"
#include <symmetric-points-mover.h>
#include <default-points-mover.h>
#include <ellipse-creator.h>
#include <algebra.h>
#include <line.h>
#include <aabb.h>
#include <lsm.h>

namespace rn {
  CylindricalModelCreator::CylindricalModelCreator():
    clicks_counter_(0),
    points_mover_(new DefaultPointsMover()),
    basis_(3, vec2i(0, 0))
  {

  }

  void CylindricalModelCreator::setPointsMover(const CreatingMode& mode) {
    ModelCreator::setPointsMover(mode);
    if (mode == Normal) {
      points_mover_.reset(new DefaultPointsMover());
      if (data_) {
        updateMover();
      }
    }
    else { // if (mode == Symmetrically)
      points_mover_.reset(new SymmetricPointsMover());
      if (data_) {
        updateMover();
      }
    }
  }

  void CylindricalModelCreator::recreate(Mesh::HardPtr mesh, const QVector<QVector<vec2i>>& anchor_points) {
    Mesh::HardPtr new_mesh(mesh->clone());
    new_mesh->vertices.clear();
    new_mesh->tex_coord.clear();
    for (auto layer : anchor_points) {
      auto ellipse = createLayerPoints(layer);
      new_mesh->addLayer(ellipse);

      if (using_texturing) {
        auto uv = defTexCoord(ellipse, layer);
        new_mesh->addTexCoords(uv);
      }
    }

    new_mesh->updateNormals();
    new_mesh->anchor_points = anchor_points;
    new_mesh->texture_id = data_->texture();

    new_mesh->top_cover = mesh->top_cover;
    new_mesh->bottom_cover = mesh->bottom_cover;
    new_mesh->updateNormals();
    mesh->swap(new_mesh.get());
  }

  void CylindricalModelCreator::smoothWithLSM(Mesh::HardPtr mesh, int ds) {
    int steps = mesh->anchor_points.size() / ds;
    auto anchor_points = mesh->anchor_points;
    QVector<double> xs, ys;
    xs.reserve(ds * 2);
    ys.reserve(ds * 2);

    for (int i = 1; i < mesh->anchor_points.size() - 1; ++i) {
      auto prev = mesh->anchor_points[i - 1];
      auto cur = mesh->anchor_points[i];
      auto next = mesh->anchor_points[i + 1];


    }

    //for (int i = 0; i <= steps; ++i) {
    //	if (i == steps) {
    //		ds = mPrevLayers.size() - steps*ds;
    //		if (ds<4) goto BUILD;
    //	}

    //	int maxDistX = -(DBL_MAX - 1), maxDistY = -(DBL_MAX - 1);
    //	for (int k = 0; k<ds; ++k) {
    //		for (int j = 0; j<ds; ++j) {
    //			if (i*cDs + j >= 0 && i*cDs + j<mPrevLayers.size() && k*cDs + k >= 0 && i*cDs + k<mPrevLayers.size()) {
    //				maxDistX = max(maxDistX, abs(mPrevLayers[i*cDs + j][0].x - mPrevLayers[i*cDs + k][0].x));
    //				maxDistY = max(maxDistY, abs(mPrevLayers[i*cDs + j][0].y - mPrevLayers[i*cDs + k][0].y));
    //			}
    //		}
    //	}

    //	bool horz = maxDistX >= maxDistY;
    //	for (int point = 0; point<1; ++point) {
    //		xs.clear();
    //		ys.clear();

    //		for (int j = -3; j<ds + 3; ++j) {
    //			if (i*cDs + j >= 0 && i*cDs + j<mPrevLayers.size()) {
    //				xs.push_back(mPrevLayers[i*cDs + j][point].x);
    //				ys.push_back(mPrevLayers[i*cDs + j][point].y);
    //			}
    //		}
    //		lsm::QuadFunc func;
    //		if (horz) {
    //			func.build(xs.begin(), ys.begin(), xs.size());
    //			for (int j = 0; j<ds; ++j) {
    //				mPrevLayers[i*cDs + j][point].y = func(mPrevLayers[i*cDs + j][point].x);
    //			}
    //		}
    //		else {
    //			func.build(ys.begin(), xs.begin(), xs.size());
    //			for (int j = 0; j<ds; ++j) {
    //				mPrevLayers[i*cDs + j][point].x = func(mPrevLayers[i*cDs + j][point].y);
    //			}
    //		}

    //	}

    //BUILD:
    //	for (int j = 0; j<ds; ++j) {
    //		vector<vec2i> base = mPrevLayers[i*cDs + j];
    //		vector<vec3i> ellipse = toFormEllipse(base);
    //		data()->meshes().back()->addLayer(ellipse);
    //		if (usingTexturing) {
    //			vector<vec2d> tex = defTexCoord(ellipse, base);
    //			data()->meshes().back()->addTexCoords(tex);
    //		}
    //	}
    //}

    recreate(mesh, anchor_points);
  }

  void CylindricalModelCreator::smoothWithAveraging(Mesh::HardPtr mesh) {
    auto anchor_points = mesh->anchor_points;
    for (int i = 1; i<mesh->anchor_points.size() - 1; ++i) {
      auto prev = mesh->anchor_points[i - 1];
      auto cur = mesh->anchor_points[i];
      auto next = mesh->anchor_points[i + 1];

      QVector<vec2i> base(3);
      base[0] = vec2i((prev[0].x + cur[0].x + next[0].x) / 3, cur[0].y);
      base[1] = vec2i((prev[1].x + cur[1].x + next[1].x) / 3, cur[1].y);
      base[2] = vec2i((prev[2].x + cur[2].x + next[2].x) / 3, cur[2].y);

      anchor_points[i] = base;
    }

    recreate(mesh, anchor_points);
  }

  void CylindricalModelCreator::place(Mesh::HardPtr mesh, int radius) {
    const int dx[] = { -1, 0, 1, 1, 1, 0, -1, -1 };
    const int dy[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
    QList<vec2i> shifts = { vec2i(0, 0) };
    for (int r = 1; r <= radius; ++r) {
      for (int i = 0; i < 8; ++i) {
          auto p = vec2i(dx[i] * r, dy[i] * r);
          if (!shifts.contains(p)) {
              shifts << p;
          }
      }
    }

    vec2i target_shift(0, 0);
    double energy = Double::min();
    vec2i offset = data_->screenCenter() - data_->offsets;
    for (auto shift : shifts) {
      double cur_energy = 0.0;
      for (auto& layer : mesh->anchor_points) {
        for (auto& p : layer) {
          auto point = p + offset + shift; // СК изображения
          if (data_->gvf->isCorrect(point.x, point.y)) { // в энергии будем учитывать только точки, попадающие в изображение
            cur_energy += data_->gvf->at(point.x, point.y);
          }
        }
      }

      if (cur_energy > energy) {
        target_shift = shift;
        energy = cur_energy;
      }
    }

    if (target_shift != vec2i(0, 0)) {
      mesh->move(vec3i(target_shift, ProjectionPlane::OXY));
    }
  }

  void CylindricalModelCreator::onMousePress(Qt::MouseButton button) {
    ModelCreator::onMousePress(button);

    if (buttons_[Qt::MouseButton::LeftButton]) {
      if (clicks_counter_ < 3) {
        basis_[clicks_counter_++] = offset_mouse_;
        if (clicks_counter_ == 2) {
          defInclinationAngle();
        }
        else if (clicks_counter_ == 3) {
          toProcessBasis();
        }
      }
      else {
        emit signalBeforeNewModelCreating();

        current_mesh_->texture_id = data_->texture();
        current_mesh_->anchor_points = prev_layers_;
        current_mesh_->updateNormals();

        data_->addMesh(current_mesh_);
        current_mesh_.reset();

        goToOverview();
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

  void CylindricalModelCreator::OnInterruptRequest() {
    goToOverview();
    current_mesh_.reset();
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
    current_mesh_.reset(new Mesh());
    current_mesh_->texture_id = data_->texture();

    /* Определяем кручение эллипса */
    double a = (basis_[0] - basis_[1]).length();
    double b = Line<int>(basis_[0], basis_[1]).dist(basis_[2]);
    rotation_angle_ = qAbs(math::Pi_2 - qAcos(b / (a * 0.5)));
    prev_layers_.push_back(basis_);

    updateMover();
    toSpecify(prev_layers_.back());

    auto ellipse = createLayerPoints(prev_layers_.back());
    current_mesh_->addLayer(ellipse);

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

    if (std::abs(inclination_angle_) > math::Pi_2) {
      inclination_angle_ -= math::Pi * math::sign(static_cast<int>(inclination_angle_));
    }
  }

  void CylindricalModelCreator::toSpecify(QVector<vec2i>& points, vec2d normal, double length, vec2d* dir) {
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

  QVector<vec2d> CylindricalModelCreator::defTexCoord(QVector<vec3i> src, const QVector<vec2i>& base) {
    vec2i offset = data_->screenCenter() - data_->offsets;
    auto axis = vec3d((base[0] - base[1]).to<double>(), ProjectionPlane::OXY, 0).normalize();

    vec3i center = createAABB<int>(src.begin(), src.end()).center();
    auto transform = mat3d::rotation(rn::abs(axis), rotation_angle_);
    for (auto &e : src) {
      e -= center;
      e = (e.to<double>()*transform).to<int>() + center;
    }

    QVector<vec2d> uv;
    int T = src.size() / 2;
    uv.reserve(src.size());
    for (int i = 0; i<src.size(); ++i) {
      vec3d e;
      if (texturing_mode == Mirror) { // это если отражать зеркально
        e = (src[i] - center).to<double>() * 0.98 + center.to<double>();

      }
      else if (texturing_mode == Cyclically) { // это если продолжать циклически
        if (src[i].z >= 0) {
          e = (src[i] - center).to<double>() * 0.98 + center.to<double>();
        }
        else {
          int index = i + T;
          e = (src[index % src.size()] - center).to<double>() * 0.98 + center.to<double>();
        }
      }

      vec2d coord;
      coord.x = double(e.x + offset.x) / data_->width();
      coord.y = 1.0 - double(e.y + offset.y) / data_->height();
      uv.push_back(coord);
    }

    return uv;
  }

  void CylindricalModelCreator::updateMesh() {
    prev_layers_.resize(1);
    auto layer = prev_layers_.back();
    double dist = Line<int>(layer[0], layer[1]).dist(offset_mouse_);

    auto calc_bounding_box = [](const QVector<QVector<vec2i>>& points) -> QPair<vec2i, vec2i> {
      if (points.isEmpty()) {
        return qMakePair(vec2i(0, 0), vec2i(0, 0));
      }

      QPair<vec2i, vec2i> box {
        vec2i(Int::max(), Int::max()),
            vec2i(Int::min(), Int::min())
      };

      for (auto& layer : points) {
        for (auto& point : layer) {
          box.first.x = qMin(box.first.x, point.x);
          box.first.y = qMin(box.first.y, point.y);
          box.second.x = qMax(box.second.x, point.x);
          box.second.y = qMax(box.second.y, point.y);
        }
      }

      return box;
    };
    auto calc_square = [](const QPair<vec2i, vec2i>& area) {
      return qAbs((area.second.x - area.first.x) * (area.second.y - area.first.y));
    };

    int prev_square = calc_square(calc_bounding_box(prev_layers_));
    while (std::abs(dist) >= std::abs(data_->step)) {
      correctStep();

      auto layer = prev_layers_.back();
      auto n = Line<int>(basis_[0], basis_[1]).normal().to<double>().normalize();

      toSpecify(layer, n, data_->step);
      if ((layer[0] - layer[1]).length() < 1.0) {
        break; // слой выродился в точку
      }

      layer[2] = layer[0] + (basis_[2] - basis_[0]);
      prev_layers_.push_back(layer);

      int square = calc_square(calc_bounding_box(prev_layers_));
      if (prev_square == square) { // габариты не изменились - что-то не так
        prev_layers_.pop_back(); // последний слой ничего не изменил, отбросим его
        break;
      }

      prev_square = square;
      dist = Line<int>(layer[0], layer[1]).dist(offset_mouse_);
    }

    data_->setLastLayer(createEllipseByThreePoints(prev_layers_.back()));
    current_mesh_ = createMeshFromLayers(prev_layers_);
  }

  Mesh::HardPtr CylindricalModelCreator::createMeshFromLayers(const QVector<QVector<vec2i>>& layers) {
    Mesh::HardPtr mesh(new Mesh());
    for (auto layer : layers) {
      auto ellipse = createLayerPoints(layer);
      mesh->addLayer(ellipse);

      if (using_texturing) {
        auto uv = defTexCoord(ellipse, layer);
        mesh->addTexCoords(uv);
      }
    }

    mesh->updateNormals();
    mesh->anchor_points = layers;
    mesh->texture_id = data_->texture();
    return mesh;
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

    mat3d rotation = mat3d::rotZ(-inclination_angle_); // матрица преобразования точки

    QVector<vec3i> ellipse;
    ellipse.reserve(source.size());
    for (auto& e : source) {
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
        current_mesh_->render(vec3b(70, 130, 180), false);
      }

      glLineWidth(1);
      glColor3d(1, 1, 1);
    }
  }
}
