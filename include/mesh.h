#ifndef MESH_H_INCLUDED__
#define MESH_H_INCLUDED__

#include <memory>
#include <vector>
#include <array>
#include <QPair>
#include <QRect>
#include <QVector>
#include <triangle.h>
#include <algebra.h>

typedef unsigned int GLuint;
using layer_t = QPair<int, int>;
using triangles_t = QVector<Trid>;

class Mesh {
  QVector<layer_t> layers_;

  // нижняя и верхняя крышки - отдельно, для удобства слияния нескольких мешей
  struct Cover {
    vec3i vertex;
    triangles_t triangles;
    bool need_triangulate = false;
  };

private:
  vec3i& vert(int index);
  const vec3i& vert(int index) const;
  vec2d& tex(int index);
  const vec2d& tex(int index) const;

  vec3d layerCenter(const layer_t& layer) const; // возвращает точку - центр слоя
  static QPair<int, int> findNearestLayers(const Mesh& first, const Mesh& second);

public:
  typedef std::shared_ptr<Mesh> HardPtr;
  typedef QVector<vec3i> vertices_t;
  typedef QVector<QVector<vec2i>> triangles_t;

public:
  QVector<vec3i> vertices;
  QVector<Trid> triangles;
  QVector<vec2d> tex_coord;
  QVector<QVector<vec2i>> anchor_points;
  Cover bottom_cover, top_cover; // нижняя и верхняя крышки" модели
  GLuint texture_id = 0; // идентификатор текстуры для наложения

  Mesh() = default;

  // пытается склеить модели по верхней или нижней крышкам
  static Mesh::HardPtr unite(const Mesh::HardPtr& first, const Mesh::HardPtr& second);

  // просто сливает две модели в одну
  static Mesh::HardPtr merge(const Mesh::HardPtr& first, const Mesh::HardPtr& second);

  void saveAsObj(const char* file);

  void clear();
  Mesh& mirror(int anchor = 0); // если anchor = 0 - используется центральная ось модели
  Mesh& swap(Mesh* mesh);
  Mesh& move(const vec3i& diff);

  vec3i center() const;
  double dist(const Mesh& other, bool by_covers = false) const; // by_covers = true: расстояние между первым и последним слоями мешей, иначе по всем точкам
  bool fallsInto(const QRect& rect) const; // область модели пересекается с указанным прямоугольником
  bool contains(const QPoint& point) const; // точка лежит в границах модели

  Mesh::HardPtr clone() const;

  vec3i& operator[](int index);
  const vec3i& operator[](int index) const;

  void updateNormals();

  QPair<int, int> outLayers() const; // первый и последний слои (по расположению, а не нумерации)

  void triangulateLastLayer();
  void triangulateFirstLayer();
  void removeLayer(int index);
  void removeLastLayer();
  void triangulateLayer(int index);
  void addTexCoords(const QVector<vec2d>& coords);
  void addLayer(const QVector<vec3i>& layer);
  void newLayer(const QVector<vec3i>& vertices);
  void triangleLayers(const layer_t& first, const layer_t& second);

  int getLayer(int vert_index) const;
  QPair<vertices_t::iterator, vertices_t::iterator> getLayerPoints(int layer);

  size_t addTriangle(size_t ind1, size_t ind2, size_t ind3); // индексы вершин

  virtual void render(const vec3b& color = vec3b(70, 130, 180), bool texturing = true, bool selected = false) const;
};

#endif // MESH_H_INCLUDED__
