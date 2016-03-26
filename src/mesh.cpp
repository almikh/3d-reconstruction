#include <mesh.h>
#include <QtOpenGL>
#include <fstream>
#include <aabb.h>
#include <mesh.h>
#include <defs.h>
#include <plane.h>

#define TOP_VERT_INDEX			-1
#define BOTTOM_VERT_INDEX		-2

void Mesh::saveAsObj(const char* file) {
  std::ofstream out(file);

  out << "g mesh" << std::endl;
  out << std::endl;

  out << "# Список вершин" << std::endl;
  for (auto &e : vertices) {
    out << "v " << e.x << " " << -e.y << " " << e.z << std::endl;
  }
  out << std::endl;

  out << "# Текстурные координаты" << std::endl;
  for (auto &e : tex_coord) {
    out << "vt " << e.x << " " << e.y << " " << std::endl;
  }
  out << std::endl;

  out << "# Нормали для треугольников" << std::endl;
  for (auto &e : triangles) {
    out << "vn " << e.normal.x << " " << e.normal.x << " " << e.normal.z << std::endl;
  }
  out << std::endl;

  out << "# Треугольники" << std::endl;
  /* f v/vt/vn v/vt/vn v/vt/vn */
  if (tex_coord.size() > 0) {
    for (int i = 0; i < triangles.size(); ++i) {
      auto& tri = triangles[i];
      out << "f ";
      out << tri[0] + 1 << "/" << tri[0] + 1 << "/" << i + 1 << " ";
      out << tri[1] + 1 << "/" << tri[1] + 1 << "/" << i + 1 << " ";
      out << tri[2] + 1 << "/" << tri[2] + 1 << "/" << i + 1 << " ";
      out << std::endl;
    }
  }
  else {
    for (int i = 0; i < triangles.size(); ++i) {
      auto& tri = triangles[i];
      out << "f ";
      out << tri[0] + 1 << "//" << i + 1 << " ";
      out << tri[1] + 1 << "//" << i + 1 << " ";
      out << tri[2] + 1 << "//" << i + 1 << " ";
      out << std::endl;
    }
  }

  out << std::endl;
  out.close();
}

void Mesh::clear() {
  layers_.clear();
  vertices.clear();
  triangles.clear();
  tex_coord.clear();
}

Mesh& Mesh::mirror(int anchor) {
  if (!anchor) {
    anchor = center().x;
  }

  bottom_cover.vertex.x = anchor - (bottom_cover.vertex.x - anchor);
  top_cover.vertex.x = anchor - (top_cover.vertex.x - anchor);

  for (auto& e : vertices) {
    e.x = anchor - (e.x - anchor);
  }

  for (auto &layer : anchor_points) {
    for (auto &e : layer) {
      e.x = anchor - (e.x - anchor);
    }
  }

  updateNormals();
  return *this;
}

Mesh& Mesh::swap(Mesh* mesh) {
  std::swap(texture_id, mesh->texture_id);
  anchor_points.swap(mesh->anchor_points);
  triangles.swap(mesh->triangles);
  vertices.swap(mesh->vertices);
  tex_coord.swap(mesh->tex_coord);
  layers_.swap(mesh->layers_);
  top_cover.triangles.swap(mesh->top_cover.triangles);
  std::swap(top_cover.vertex, mesh->top_cover.vertex);
  bottom_cover.triangles.swap(mesh->bottom_cover.triangles);
  std::swap(bottom_cover.vertex, mesh->bottom_cover.vertex);

  return *this;
}

Mesh& Mesh::move(const vec3i& diff) {
  bottom_cover.vertex += diff;
  top_cover.vertex += diff;
  for (auto& e : vertices) {
    e += diff;
  }

  vec2i proj = diff.projXY();
  for (auto &layer : anchor_points) {
    for (auto &e : layer) {
      e += proj;
    }
  }

  return *this;
}

vec3i Mesh::center() const {
  return createAABB<int>(vertices.begin(), vertices.end()).center();
}

double Mesh::dist(const Mesh& other, bool by_covers) const {
  QVector<vec3i> verts[2];
  if (!by_covers) {
    verts[0] = vertices;
    verts[1] = other.vertices;
  }
  else {
    // первый и последний слои
    auto last_layer = layers_.last();
    auto first_layer = layers_.first();
    verts[0].reserve((last_layer.second - last_layer.first) + (first_layer.second - first_layer.first));
    for (int i = first_layer.first; i < first_layer.second; ++i) verts[0] << vertices[i];
    for (int i = last_layer.first; i < last_layer.second; ++i) verts[0] << vertices[i];

    // первый и последний слои other
    auto last_layer2 = other.layers_.last();
    auto first_layer2 = other.layers_.first();
    verts[1].reserve((last_layer2.second - last_layer2.first) + (first_layer2.second - first_layer2.first));
    for (int i = first_layer2.first; i < first_layer2.second; ++i) verts[1] << other.vertices[i];
    for (int i = last_layer2.first; i < last_layer2.second; ++i) verts[1] << other.vertices[i];
  }

  double dist = Double::max();
  for (auto& e1 : verts[0]) {
    for (auto& e2 : verts[1]) {
      dist = std::min(dist, e1.projXY().dist(e2.projXY()));
    }
  }

  return dist;
}

bool Mesh::fallsInto(const QRect& rect) const {
  for (auto& v : vertices) {
    if (rect.contains(QPoint(v.x, v.y))) return true;
  }

  return false;
}

bool Mesh::contains(const QPoint& point) const {
  int n = anchor_points.size();
  for (int i = 0; i < n - 1; ++i) {
    QPolygon polygon;
    polygon << QPoint(anchor_points[i][0].x, anchor_points[i][0].y);
    polygon << QPoint(anchor_points[i][1].x, anchor_points[i][1].y);
    polygon << QPoint(anchor_points[i + 1][1].x, anchor_points[i + 1][1].y);
    polygon << QPoint(anchor_points[i + 1][0].x, anchor_points[i + 1][0].y);

    if (polygon.containsPoint(point, Qt::FillRule::OddEvenFill)) {
      return true;
    }
  }

  return false;
}

Mesh::HardPtr Mesh::clone() const {
  Mesh::HardPtr mesh(new Mesh());
  mesh->layers_ = layers_;
  mesh->vertices = vertices;
  mesh->triangles = triangles;
  mesh->texture_id = texture_id;
  mesh->tex_coord = tex_coord;
  mesh->anchor_points = anchor_points;
  mesh->bottom_cover = bottom_cover;
  mesh->top_cover = top_cover;

  return mesh;
}

Mesh::HardPtr Mesh::unite(const Mesh::HardPtr& first, const Mesh::HardPtr& second) {
  auto near_layers = findNearestLayers(*first, *second);

  int vertices_count = static_cast<int>(first->vertices.size());
  auto triangles_copy = second->triangles;
  for (auto &e : triangles_copy) {
    e[0] += vertices_count;
    e[1] += vertices_count;
    e[2] += vertices_count;
  }

  auto layers_copy = second->layers_;
  for (auto &e : layers_copy) {
    e.first += vertices_count;
    e.second += vertices_count;
  }

  Mesh::HardPtr dst(new Mesh());

  // выбираем ненулевую текстуру, если она есть
  dst->texture_id = first->texture_id ? first->texture_id : second->texture_id;

  dst->vertices << first->vertices;
  dst->vertices << second->vertices;

  dst->triangles << first->triangles;
  dst->triangles << triangles_copy;

  dst->tex_coord << first->tex_coord;
  dst->tex_coord << second->tex_coord;

  dst->anchor_points << first->anchor_points;
  dst->anchor_points << second->anchor_points;

  dst->layers_ << first->layers_;
  dst->layers_ << layers_copy;

  // промежуточная часть между моделями - нужно ее триангулировать
  dst->triangleLayers(first->layers_[near_layers.first], layers_copy[near_layers.second]);
  dst->updateNormals();
  return dst;
}

Mesh::HardPtr Mesh::merge(const Mesh::HardPtr& first, const Mesh::HardPtr& second) {
  int vertices_count = static_cast<int>(first->vertices.size());
  auto triangles_copy = second->triangles;
  for (auto &e : triangles_copy) {
    e[0] += vertices_count;
    e[1] += vertices_count;
    e[2] += vertices_count;
  }

  auto layers_copy = second->layers_;
  for (auto &e : layers_copy) {
    e.first += vertices_count;
    e.second += vertices_count;
  }

  Mesh::HardPtr dst(new Mesh());

  // выбираем ненулевую текстуру, если она есть
  dst->texture_id = first->texture_id ? first->texture_id : second->texture_id;

  dst->vertices << first->vertices;
  dst->vertices << second->vertices;

  dst->triangles << first->triangles;
  dst->triangles << triangles_copy;;

  dst->tex_coord << first->tex_coord;
  dst->tex_coord << second->tex_coord;

  dst->anchor_points << first->anchor_points;
  dst->anchor_points << second->anchor_points;

  dst->layers_ << first->layers_;
  dst->layers_ << layers_copy;

  dst->updateNormals();
  return dst;
}

QPair<int, int> Mesh::findNearestLayers(const Mesh& first, const Mesh& second) {
  QVector<int> first_covers, second_covers; // индексы слоев
  first_covers << first.outLayers().first;
  first_covers << first.outLayers().second;
  second_covers << second.outLayers().first;
  second_covers << second.outLayers().second;

  QPair<int, int> dst = { 0, 0 };
  double min_dist = Double::max();
  for (auto i : first_covers) {
    // будем сравнивать расстояния между центрами слоев
    auto first_center = first.layerCenter(first.layers_[i]);
    for (auto j : second_covers) {
      auto second_center = second.layerCenter(second.layers_[j]);
      double dist = first_center.dist(second_center);
      if (dist < min_dist) {
        min_dist = dist;
        dst.first = i;
        dst.second = j;
      }
    }
  }

  return dst;
}

vec3i& Mesh::vert(int index) {
  return (*this)[index];
}

const vec3i& Mesh::vert(int index) const {
  return (*this)[index];
}

vec2d& Mesh::tex(int index) {
  if (index == BOTTOM_VERT_INDEX) index = bottom_cover.triangles.front()[0];
  else if (index == TOP_VERT_INDEX) index = top_cover.triangles.front()[0];
  return tex_coord[index];
}

const vec2d& Mesh::tex(int index) const {
  if (index == BOTTOM_VERT_INDEX) index = bottom_cover.triangles.front()[0];
  else if (index == TOP_VERT_INDEX) index = top_cover.triangles.front()[0];
  return tex_coord[index];
}

vec3i& Mesh::operator[](int index) {
  if (index == BOTTOM_VERT_INDEX) return bottom_cover.vertex;
  else if (index == TOP_VERT_INDEX) return top_cover.vertex;
  return vertices[index];
}

const vec3i& Mesh::operator[](int index) const {
  if (index == BOTTOM_VERT_INDEX) return bottom_cover.vertex;
  else if (index == TOP_VERT_INDEX) return top_cover.vertex;
  return vertices[index];
}

void Mesh::updateNormals() {
  auto update_for = [=](QVector<Trid>& triangles, bool is_cover) {
    for (auto& tri : triangles) {
      vec3d p = vert(tri[0]).to<double>() - vert(tri[1]).to<double>();
      vec3d q = vert(tri[2]).to<double>() - vert(tri[1]).to<double>();
      vec3d n = p.cross(q);

      vec3d center;
      if (is_cover) { // смотрим на центр всей модели
        center = createAABB<int>(vertices.begin(), vertices.end()).center();
      }
      else { // смотрим на центр слоя
        auto layer_it = getLayerPoints(getLayer(tri[0])); // слой одной из точек треугольника
        center = createAABB<int>(layer_it.first, layer_it.second).center();
      }

      // опред. правильное направление нормали - наружу из центра
      if (n.angle(vert(tri[0]).to<double>() - center) > math::Pi_2) n *= -1;

      tri.normal = n.normalize();
    }
  };

  update_for(triangles, false);
  update_for(top_cover.triangles, true);
  update_for(bottom_cover.triangles, true);
}

vec3d Mesh::layerCenter(const layer_t& layer) const {
  auto begin = vertices.begin();
  auto layer_aabb = createAABB<int>(begin + layer.first, begin + layer.second);
  return layer_aabb.center();
}

QPair<int, int> Mesh::outLayers() const {
  QPair<int, int> out_layers = { 0, 0 };
  QPair<int, int> out_layers_price = { Int::max(), Int::min() }; // вспомогательная переменная, оценивает можность слоя
  for (int i = 0; i < layers_.size(); ++i) {
    auto layer = layers_[i];
    if (layer.second - layer.first < 3) continue;

    auto f = vec3i(vert(layer.first)).to<double>();
    auto s = vec3i(vert(layer.first + 1)).to<double>();
    auto ray_start = layerCenter(layer);

    if (f.dist(ray_start) < 2.5 && f.dist(s) < 2.5 && s.dist(ray_start) < 2.5) {
      // то это слой, в котором точки практически слиты в одну
      continue;
    }

    Plane<double> plane(f, s, ray_start);
    vec3d ray_normal = plane.normal().normalize();

    int counter[2] = { 0, 0 }; // сколько плоскостей пересекает нормаль и инверированная нормаль текущего слоя
    for (auto other : layers_) {
      if (other == layer) continue;
      if (other.second - other.first < 3) continue;

      auto f = vec3i(vert(other.first)).to<double>();
      auto s = vec3i(vert(other.first + 1)).to<double>();
      auto t = layerCenter(other);

      if (f.dist(t) < 2.5 && f.dist(s) < 2.5 && s.dist(t) < 2.5) {
        continue; // очень `плотный` слой
      }

      Plane<double> other_plane(f, s, t);
      if (other_plane.intersect(ray_start, ray_normal)) counter[0] += 1;
      if (other_plane.intersect(ray_start, ray_normal * -1.0)) counter[1] += 1;
    }

    int price = counter[0] - counter[1];
    if (price < out_layers_price.first) {
      out_layers_price.first = price;
      out_layers.first = i;
    }

    if (price > out_layers_price.second) {
      out_layers_price.second = price;
      out_layers.second = i;
    }
  }

  if (layerCenter(layers_[out_layers.first]).y > layerCenter(layers_[out_layers.second]).y) {
    std::swap(out_layers.first, out_layers.second);
  }

  return out_layers;
}

void Mesh::triangulateLastLayer() {
  top_cover.need_triangulate = true;

  auto layer = layers_[outLayers().second];
  top_cover.vertex = layerCenter(layer); // точка схода вершин последнего слоя
  top_cover.triangles.clear();

  // третий индекс - фиктивный, вместо него будет bottom_cover.first
  top_cover.triangles.push_back(Trid(layer.second - 1, layer.first, TOP_VERT_INDEX));
  for (int i = layer.first; i < layer.second - 1; ++i) {
    top_cover.triangles.push_back(Trid(i, i + 1, TOP_VERT_INDEX));
  }

  updateNormals();
}

void Mesh::triangulateFirstLayer() {
  bottom_cover.need_triangulate = true;

  auto layer = layers_[outLayers().first];
  bottom_cover.vertex = layerCenter(layer); // точка схода вершин первого слоя
  bottom_cover.triangles.clear();

  // третий индекс - фиктивный, вместо него будет top_cover.first
  bottom_cover.triangles.push_back(Trid(layer.second - 1, layer.first, BOTTOM_VERT_INDEX));
  for (int i = layer.first; i < layer.second - 1; ++i) {
    bottom_cover.triangles.push_back(Trid(i, i + 1, BOTTOM_VERT_INDEX));
  }

  updateNormals();
}

void Mesh::triangulateLayer(int index) {
  auto layer = layers_[index];
  for (int i = layer.first; i < layer.second - 2; ++i) {
    addTriangle(i, i + 1, layer.second - 1);
  }
}

void Mesh::removeLayer(int index) {
  auto layer = layers_[index];

  auto removedTri = [&layer](const Trid& tri) {
    return (layer.first <= tri[0] && tri[0]<layer.second) ||
        (layer.first <= tri[1] && tri[1]<layer.second) ||
        (layer.first <= tri[2] && tri[2]<layer.second);
  };

  triangles.erase(std::remove_if(triangles.begin(), triangles.end(), removedTri), triangles.end());
  vertices.erase(vertices.begin() + layer.first, vertices.begin() + layer.second);
}

void Mesh::removeLastLayer() {
  removeLayer(layers_.size() - 1);
}

void Mesh::addTexCoords(const QVector<vec2d>& coords) {
  tex_coord << coords;
}

void Mesh::addLayer(const QVector<vec3i>& layer) {
  if (layers_.empty()) {
    newLayer(layer);
    return;
  }

  auto prevLayer = layers_.back();
  newLayer(layer);
  auto curLayer = layers_.back();

  triangleLayers(prevLayer, curLayer);
}

void Mesh::newLayer(const QVector<vec3i>& vs) {
  QPair<size_t, size_t> layer;
  layer.first = vertices.size();
  layer.second = layer.first + vs.size();

  layers_.push_back(layer);
  vertices << vs;
}

void Mesh::triangleLayers(const layer_t& first, const layer_t& second) {
  int cur = second.first;
  for (int i = first.first; i < first.second - 1; ++i) {
    addTriangle(i, cur, i + 1);
    addTriangle(cur, cur + 1, i + 1);
    ++cur;
  }

  addTriangle(first.second - 1, cur, first.first);
  addTriangle(second.second - 1, second.first, first.first);
}

int Mesh::getLayer(int vert_index) const {
  int i = 0;
  int size = static_cast<int>(layers_.size());
  while (i < size) {
    if (layers_[i].first <= vert_index && vert_index < layers_[i].second) {
      return i;
    }

    ++i;
  }

  return static_cast<int>(layers_.size()); // нет такого слоя
}

QPair<Mesh::vertices_t::iterator, Mesh::vertices_t::iterator> Mesh::getLayerPoints(int layer) {
  QPair<Mesh::vertices_t::iterator, Mesh::vertices_t::iterator> dst;
  dst.first = vertices.begin() + layers_[layer].first;
  dst.second = vertices.begin() + layers_[layer].second;

  return dst;
}

size_t Mesh::addTriangle(size_t ind1, size_t ind2, size_t ind3) {
  triangles.push_back(Trid(ind1, ind2, ind3));
  return triangles.size() - 1;
}

void Mesh::render(const vec3b& color, bool texturing, bool selected) const {
  bool use_texture = texturing && !tex_coord.empty();

  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);

  if (use_texture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);
  }

  auto& mesh = *this;

  auto triangles = mesh.triangles;
  triangles << top_cover.triangles;
  triangles << bottom_cover.triangles;
  glBegin(GL_TRIANGLES);
  for (auto& tri : triangles) {
    if (selected) glColor3ub(255, 0, 0);
    else if (use_texture) glColor3ub(255, 255, 255);
    else glColor3ubv(color.coords);

    glNormal3dv(tri.normal.coords);
    if (use_texture) glTexCoord2dv(tex(tri[0]).coords);
    glVertex3iv(mesh[tri[0]].coords);

    glNormal3dv(tri.normal.coords);
    if (use_texture) glTexCoord2dv(tex(tri[1]).coords);
    glVertex3iv(mesh[tri[1]].coords);

    glNormal3dv(tri.normal.coords);
    if (use_texture) glTexCoord2dv(tex(tri[2]).coords);
    glVertex3iv(mesh[tri[2]].coords);
  }
  glEnd();

  if (use_texture) {
    glDisable(GL_TEXTURE_2D);
  }

  glDisable(GL_LIGHTING);
  glDisable(GL_NORMALIZE);

#ifndef NDEBUG
  if (selected && use_texture) {
    glLineWidth(5);
    glColor3d(1, 0, 0);
    glBegin(GL_LINE_STRIP);
    for(auto &e: anchor_points) glVertex2iv(e[0].coords);
    glEnd();
    glBegin(GL_LINE_STRIP);
    for (auto &e: anchor_points) glVertex2iv(e[1].coords);
    glEnd();
    glLineWidth(1);
  }
#endif
  glColor3d(1, 1, 1);
}
