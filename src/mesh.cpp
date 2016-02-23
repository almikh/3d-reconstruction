#include <mesh.h>
#include <QtOpenGL>
#include <fstream>
#include <aabb.h>

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
  for (auto &e : tex_coord_) {
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
  if (tex_coord_.size() > 0) {
    for (size_t i = 0; i < triangles.size(); ++i) {
      auto& tri = triangles[i];
      out << "f ";
      out << tri[0] + 1 << "/" << tri[0] + 1 << "/" << i + 1 << " ";
      out << tri[1] + 1 << "/" << tri[1] + 1 << "/" << i + 1 << " ";
      out << tri[2] + 1 << "/" << tri[2] + 1 << "/" << i + 1 << " ";
      out << std::endl;
    }
  }
  else {
    for (size_t i = 0; i < triangles.size(); ++i) {
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
  tex_coord_.clear();
}

Mesh& Mesh::swap(Mesh* mesh) {
  anchor_points.swap(mesh->anchor_points);
  triangles.swap(mesh->triangles);
  vertices.swap(mesh->vertices);
  tex_coord_.swap(mesh->tex_coord_);
  layers_.swap(mesh->layers_);

  return *this;
}

Mesh& Mesh::move(const vec3i& diff) {
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

double Mesh::dist(const Mesh& other) const {
  double dist = Double::max();
  for (auto& e1 : vertices) {
    for (auto& e2 : other.vertices) {
      dist = std::min(dist, e1.dist(e2));
    }
  }

  return dist;
}

bool Mesh::fallsInto(const QRect& rect) const {
  for (auto& group : anchor_points) {
    for (auto& e : group) {
      if (rect.contains(QPoint(e.x, e.y))) {
        return true;
      }
    }
  }

  return false;
}

Mesh Mesh::copy() const {
  Mesh mesh;
  mesh.layers_ = layers_;
  mesh.vertices = vertices;
  mesh.triangles = triangles;
  mesh.anchor_points = anchor_points;

  return mesh;
}

Mesh Mesh::unite(const Mesh& first, const Mesh& second) {
  auto near_layers = findNearestLayers(first, second);

  int vertices_count = static_cast<int>(first.vertices.size());
  auto triangles_copy = second.triangles;
  for (auto &e : triangles_copy) {
    e[0] += vertices_count;
    e[1] += vertices_count;
    e[2] += vertices_count;
  }

  auto layers_copy = second.layers_;
  for (auto &e : layers_copy) {
    e.first += vertices_count;
    e.second += vertices_count;
  }

  Mesh dst;
  dst.vertices.insert(dst.vertices.end(), first.vertices.begin(), first.vertices.end());
  dst.vertices.insert(dst.vertices.end(), second.vertices.begin(), second.vertices.end());

  dst.triangles.insert(dst.triangles.end(), first.triangles.begin(), first.triangles.end());
  dst.triangles.insert(dst.triangles.end(), triangles_copy.begin(), triangles_copy.end());

  dst.tex_coord_.insert(dst.tex_coord_.end(), first.tex_coord_.begin(), first.tex_coord_.end());
  dst.tex_coord_.insert(dst.tex_coord_.end(), second.tex_coord_.begin(), second.tex_coord_.end());

  if (near_layers.first == 0) {
    // для первого - слои в обратном порядке
    dst.layers_.insert(dst.layers_.end(), first.layers_.rbegin(), first.layers_.rend());
    if (near_layers.second == 0) {
      dst.layers_.insert(dst.layers_.end(), layers_copy.begin(), layers_copy.end());
    }
    else {
      // для второго - слои в обратном порядке
      dst.layers_.insert(dst.layers_.end(), layers_copy.rbegin(), layers_copy.rend());
    }
  }
  else {
    dst.layers_.insert(dst.layers_.end(), first.layers_.begin(), first.layers_.end());
    if (near_layers.second == 0) {
      dst.layers_.insert(dst.layers_.end(), layers_copy.begin(), layers_copy.end());
    }
    else {
      dst.layers_.insert(dst.layers_.end(), layers_copy.rbegin(), layers_copy.rend());
    }
  }

  // промежуточная часть между моделями - нужно ее триангулировать
  dst.triangleLayers(dst.layers_[first.layers_.size() - 1], dst.layers_[first.layers_.size()]);
  dst.updateNormals();
  return dst;
}

std::pair<int, int> Mesh::findNearestLayers(const Mesh& first, const Mesh& second) {
  std::vector<int> first_covers;
  auto back_layer_1 = first.layers_.back();
  auto front_layer_1 = first.layers_.front();
  for (int i = front_layer_1.first; i < front_layer_1.second; ++i) first_covers.push_back(i);
  for (int i = back_layer_1.first; i < back_layer_1.second; ++i) first_covers.push_back(i);

  std::vector<int> second_covers;
  auto back_layer_2 = second.layers_.back();
  auto front_layer_2 = second.layers_.front();
  for (int i = front_layer_2.first; i < front_layer_2.second; ++i) second_covers.push_back(i);
  for (int i = back_layer_2.first; i < back_layer_2.second; ++i) second_covers.push_back(i);

  int f_ind = 0, s_ind = 0;
  double min_dist = Double::max();
  for (int i = 0, n = first_covers.size(); i<n; ++i) {
    for (int j = 0, m = second_covers.size(); j < m; ++j) {
      double dist = first.vertices[i].dist(second.vertices[j]);
      if (dist < min_dist) {
        min_dist = dist;
        f_ind = i;
        s_ind = j;
      }
    }
  }

  std::pair<int, int> dst;
  dst.first = first.getLayer(f_ind);
  dst.second = second.getLayer(s_ind);
  return dst;
}

vec3i& Mesh::vert(int index) {
  return (*this)[index];
}

const vec3i& Mesh::vert(int index) const {
  return (*this)[index];
}

vec3i& Mesh::operator[](int index) {
  if (index == BOTTOM_VERT_INDEX) return bottom_cover_.vertex;
  else if (index == TOP_VERT_INDEX) return top_cover_.vertex;
  return vertices[index];
}

const vec3i& Mesh::operator[](int index) const {
  if (index == BOTTOM_VERT_INDEX) return bottom_cover_.vertex;
  else if (index == TOP_VERT_INDEX) return top_cover_.vertex;
  return vertices[index];
}

void Mesh::updateNormals() {
  auto update_for = [=](std::vector<Trid>& triangles, bool is_cover) {
    for (auto& tri : triangles) {
      vec3d p = vert(tri[0]).to<double>() - vert(tri[1]).to<double>();
      vec3d q = vert(tri[2]).to<double>() - vert(tri[1]).to<double>();
      vec3d n = p.cross(q);

      vec3d center;
      if (is_cover) { // смотрим на центр всей модели
        center = createAABB<int>(vertices.begin(), vertices.end()).center();
      }
      else { // смотрим на центр слоя
        vertices_t::iterator begin, end; // начало и конец слоя одной из точек
        auto it = getLayerPoints(getLayer(tri[0])); // слой одной из точек треугольника
        center = createAABB<int>(it.first, it.second).center();
      }

      // опред. правильное направление нормали - наружу из центра
      if (n.angle(vert(tri[0]).to<double>() - center) > math::Pi_2) n *= -1;

      tri.normal = n.normalize();
    }
  };

  update_for(triangles, false);
  update_for(top_cover_.triangles, true);
  update_for(bottom_cover_.triangles, true);
}

void Mesh::triangulateLastLayer() {
  top_cover_.need_triangulate = true;

  auto l = layers_.back();
  auto aabb = createAABB<int>(
        vertices.begin() + l.first,
        vertices.begin() + l.second);

  top_cover_.vertex = aabb.center(); // точка схода вершин последнего слоя
  top_cover_.triangles.clear();

  // третий индекс - фиктивный, вместо него будет bottom_cover_.first
  top_cover_.triangles.push_back(Trid(l.second - 1, l.first, TOP_VERT_INDEX));
  for (int i = l.first; i < l.second - 1; ++i) {
    top_cover_.triangles.push_back(Trid(i, i + 1, TOP_VERT_INDEX));
  }
}

void Mesh::triangulateFirstLayer() {
  bottom_cover_.need_triangulate = true;

  auto l = layers_.front();
  auto aabb = createAABB<int>(
        vertices.begin() + l.first,
        vertices.begin() + l.second);

  bottom_cover_.vertex = aabb.center(); // точка схода вершин последнего слоя
  bottom_cover_.triangles.clear();

  // третий индекс - фиктивный, вместо него будет top_cover_.first
  bottom_cover_.triangles.push_back(Trid(l.second - 1, l.first, BOTTOM_VERT_INDEX));
  for (int i = l.first; i < l.second - 1; ++i) {
    bottom_cover_.triangles.push_back(Trid(i, i + 1, BOTTOM_VERT_INDEX));
  }
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
    return layer.first <= tri[0] && tri[0]<layer.second ||
        layer.first <= tri[1] && tri[1]<layer.second ||
        layer.first <= tri[2] && tri[2]<layer.second;
  };

  triangles.erase(std::remove_if(triangles.begin(), triangles.end(), removedTri), triangles.end());
  vertices.erase(vertices.begin() + layer.first, vertices.begin() + layer.second);
}

void Mesh::removeLastLayer() {
  removeLayer(layers_.size() - 1);
}

void Mesh::addTexCoords(const std::vector<vec2d>& coords) {
  tex_coord_.insert(tex_coord_.end(), coords.begin(), coords.end());
}

void Mesh::addLayer(const std::vector<vec3i>& layer) {
  if (layers_.empty()) {
    newLayer(layer);
    return;
  }

  auto prevLayer = layers_.back();
  newLayer(layer);
  auto curLayer = layers_.back();

  triangleLayers(prevLayer, curLayer);
}

void Mesh::newLayer(const std::vector<vec3i>& vs) {
  std::pair<size_t, size_t> layer;
  layer.first = vertices.size();
  layer.second = layer.first + vs.size();

  layers_.push_back(layer);
  vertices.insert(vertices.end(), vs.begin(), vs.end());
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

std::pair<Mesh::vertices_t::iterator, Mesh::vertices_t::iterator> Mesh::getLayerPoints(int layer) {
  std::pair<Mesh::vertices_t::iterator, Mesh::vertices_t::iterator> dst;
  dst.first = vertices.begin() + layers_[layer].first;
  dst.second = vertices.begin() + layers_[layer].second;

  return dst;
}

size_t Mesh::addTriangle(size_t ind1, size_t ind2, size_t ind3) {
  triangles.push_back(Trid(ind1, ind2, ind3));
  return triangles.size() - 1;
}

void Mesh::render(const vec3b& color, bool texturing) const {
  bool use_texture = texturing && !tex_coord_.empty();

  glEnable(GL_NORMALIZE);
  glEnable(GL_LIGHTING);

  if (use_texture) {
    glBindTexture(GL_TEXTURE_2D, texture_id);
  }

  auto& mesh = *this;

  auto triangles = mesh.triangles;
  triangles.insert(triangles.end(), top_cover_.triangles.begin(), top_cover_.triangles.end());
  triangles.insert(triangles.end(), bottom_cover_.triangles.begin(), bottom_cover_.triangles.end());
  glBegin(GL_TRIANGLES);
  for (auto& tri : triangles) {
    bool has_tex_coord = false;
    if (use_texture && tri[2] != BOTTOM_VERT_INDEX && tri[2] != TOP_VERT_INDEX) {
      has_tex_coord = true;
      glEnable(GL_TEXTURE_2D);
      glColor3ub(255, 255, 255);
    }
    else glColor3ubv(color.coords); // для верхней и нижней крышки текстурирование отключим

    glNormal3d(tri.normal.x, tri.normal.y, tri.normal.z);

    if (use_texture && has_tex_coord) glTexCoord2d(tex_coord_[tri[0]].x, tex_coord_[tri[0]].y);
    glVertex3i(mesh[tri[0]].x, mesh[tri[0]].y, mesh[tri[0]].z);

    if (use_texture && has_tex_coord) glTexCoord2d(tex_coord_[tri[1]].x, tex_coord_[tri[1]].y);
    glVertex3i(mesh[tri[1]].x, mesh[tri[1]].y, mesh[tri[1]].z);

    if (use_texture && has_tex_coord) glTexCoord2d(tex_coord_[tri[2]].x, tex_coord_[tri[2]].y);
    glVertex3i(mesh[tri[2]].x, mesh[tri[2]].y, mesh[tri[2]].z);

    if (use_texture && tri[2] != BOTTOM_VERT_INDEX && tri[2] != TOP_VERT_INDEX) {
      glDisable(GL_TEXTURE_2D);
    }
  }
  glEnd();

  glDisable(GL_LIGHTING);
  glDisable(GL_NORMALIZE);

  glColor3d(1, 0, 0);
  glLineWidth(5);
  glBegin(GL_LINE_STRIP);
  //for(auto &e: anchor_points) {
  //	glVertex2d(e.front().x, e.front().y);
  //}
  glEnd();
  glBegin(GL_LINE_STRIP);
  //for (auto &e : anchor_points) {
  //	glVertex2d(e[1].x, e[1].y);
  //}
  glEnd();
  glLineWidth(1);
  glColor3d(1, 1, 1);
}
