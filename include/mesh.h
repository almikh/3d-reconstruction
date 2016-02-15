#ifndef MESH_H_INCLUDED__
#define MESH_H_INCLUDED__
#include <algebra.h>
#include <triangle.h>
#include <iostream>
#include <QVector>
#include <memory>
#include <vector>
#include <array>
#include <set>

typedef unsigned int GLuint;
using layer_t = std::pair<int, int>;
using triangles_t = std::vector<Trid>;

class Mesh {
	std::vector<Trid> triangles_;
	std::vector<vec3i> vertices_;
	std::vector<vec2d> tex_coord_;
	std::vector<layer_t> layers_;

	// нижн€€ и верхн€€ крышки - отдельно, дл€ удобства сли€ни€ нескольких мешей
	struct Cover {
		vec3i vertex;
		triangles_t triangles;
		bool need_triangulate = false;
	} bottom_cover_, top_cover_; // нижн€€ и верхн€€ крышки" модели

private:
	vec3i& vert(int index);
	const vec3i& vert(int index) const;

	static std::pair<int, int> findNearestLayers(const Mesh& first, const Mesh& second);

public:
	GLuint texture_id = 0; // идентификатор текстуры дл€ наложени€
	QVector<QVector<vec2i>> auxiliary_points;

	typedef std::shared_ptr<Mesh> HardPtr;

	template<class T, class V = typename T::iterator>
	class iterator {
		V cur_;

		iterator(const V& iter) : cur_(iter) {}
	public:
		iterator() = default;
		iterator(const iterator<T, V>& iter) : cur_(iter.cur_) {}

		typedef typename T::value_type value_type;
		typedef typename V::reference reference;
		typedef typename V::pointer pointer;

		reference operator*() {
			return *cur_;
		}

		pointer operator->() {
			return &(*cur_);
		}

		iterator<T, V>& operator++() {
			++cur_;
			return *this;
		}

		iterator<T, V> operator++(int) {
			iterator<T, V> temp(*this);
			operator++();
			return temp;
		}

		iterator<T, V> operator + (int val) {
			return iterator<T, V>(cur_ + val);
		}

		bool operator!=(const iterator<T, V>& rhs) {
			return (!(*this == rhs));
		}

		bool operator==(const iterator<T, V>& rhs) {
			return cur_ == rhs.cur_;
		}

		friend class Mesh;
	};

	using vertex_iterator = iterator<std::vector<vec3i>, std::vector<vec3i>::iterator>;
	using face_iterator = iterator<std::vector<Trid>, std::vector<Trid>::iterator>;
	using const_vertex_iterator = iterator<std::vector<vec3i>, std::vector<vec3i>::const_iterator>;
	using const_face_iterator = iterator<std::vector<Trid>, std::vector<Trid>::const_iterator>;

	Mesh() = default;

	static Mesh unite(const Mesh& first, const Mesh& second);

	void saveAsObj(const char* file);

	void clear();
	Mesh& swap(Mesh* mesh);
	Mesh& move(const vec3i& diff);

	vec3i center() const;
	double dist(const Mesh& other) const;

	Mesh copy() const;

	vec3i& operator[](int index);
	const vec3i& operator[](int index) const;

	vertex_iterator beginVertices();
	vertex_iterator endVertices();
	face_iterator beginFaces();
	face_iterator endFaces();
	const_vertex_iterator beginVertices() const;
	const_vertex_iterator endVertices() const;
	const_face_iterator beginFaces() const;
	const_face_iterator endFaces() const;

	void updateNormals();

	void triangulateLastLayer();
	void triangulateFirstLayer();
	void removeLayer(int index);
	void removeLastLayer();
	void triangulateLayer(int index);
	void addTexCoords(const std::vector<vec2d>& coords);
	void addLayer(const std::vector<vec3i>& layer);
	void newLayer(const std::vector<vec3i>& vertices, bool add_front = false);
	void triangleLayers(const layer_t& first, const layer_t& second);
	int getLayer(int vert_index, const_vertex_iterator* begin = nullptr, const_vertex_iterator* end = nullptr) const;

	size_t addTriangle(size_t ind1, size_t ind2, size_t ind3); // индексы вершин

	virtual void render(bool texturing = true) const;
};

#endif // MESH_H_INCLUDED__
