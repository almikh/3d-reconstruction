#ifndef MESH_H_INCLUDED__
#define MESH_H_INCLUDED__
#include <algebra.h>
#include <triangle.h>
#include <iostream>
#include <QVector>
#include <QRect>
#include <memory>
#include <vector>
#include <array>
#include <set>

typedef unsigned int GLuint;
using layer_t = std::pair<int, int>;
using triangles_t = std::vector<Trid>;

class Mesh {
  std::vector<vec2d> tex_coord_;
  std::vector<layer_t> layers_;

  // нижняя и верхняя крышки - отдельно, для удобства слияния нескольких мешей
  struct Cover {
    vec3i vertex;
    triangles_t triangles;
    bool need_triangulate = false;
  } bottom_cover_, top_cover_; // нижняя и верхняя крышки" модели

private:
  vec3i& vert(int index);
  const vec3i& vert(int index) const;

  static std::pair<int, int> findNearestLayers(const Mesh& first, const Mesh& second);

public:
  typedef std::shared_ptr<Mesh> HardPtr;
  typedef std::vector<vec3i> vertices_t;
  typedef QVector<QVector<vec2i>> triangles_t;

public:
  std::vector<vec3i> vertices;
  std::vector<Trid> triangles;
  QVector<QVector<vec2i>> anchor_points;
  GLuint texture_id = 0; // идентификатор текстуры для наложения

  Mesh() = default;

  static Mesh unite(const Mesh& first, const Mesh& second);

  void saveAsObj(const char* file);

  void clear();
  Mesh& swap(Mesh* mesh);
  Mesh& move(const vec3i& diff);

  vec3i center() const;
  double dist(const Mesh& other) const;
  bool fallsInto(const QRect& rect) const;

  Mesh copy() const;

  vec3i& operator[](int index);
  const vec3i& operator[](int index) const;

  void updateNormals();

  void triangulateLastLayer();
  void triangulateFirstLayer();
  void removeLayer(int index);
  void removeLastLayer();
  void triangulateLayer(int index);
  void addTexCoords(const std::vector<vec2d>& coords);
  void addLayer(const std::vector<vec3i>& layer);
  void newLayer(const std::vector<vec3i>& vertices);
  void triangleLayers(const layer_t& first, const layer_t& second);

  int getLayer(int vert_index) const;
  std::pair<vertices_t::iterator, vertices_t::iterator> getLayerPoints(int layer);

  size_t addTriangle(size_t ind1, size_t ind2, size_t ind3); // индексы вершин

  virtual void render(const vec3b& color = vec3b(70, 130, 180), bool texturing = true) const;
};

#endif // MESH_H_INCLUDED__
