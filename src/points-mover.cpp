#include <points-mover.h>

PointsMover::~PointsMover() {

}

void PointsMover::setGrowthDir(const vec2d& dir) {
	growth_dir_ = dir;
}

void PointsMover::setGrowthLength(int length) {
	growth_length_ = length;
}

void PointsMover::setChangeDir(const vec2d& dir) {
	change_dir_ = dir;
}

void PointsMover::setOffset(const vec2i& offset) {
	offset_ = offset;
}

void PointsMover::setLookAhead(bool look) {
	look_ahead_ = look;
}

void PointsMover::setGradient(std::shared_ptr<ip::Image<double>> grad) {
	grad_ = grad;
}

void PointsMover::setGradientDir(std::shared_ptr<ip::Image<double>> dir) {
	grad_dir_ = dir;
}

void PointsMover::setPrevLayers(const QVector<QVector<vec2i>>& layers) {
	prev_layers_ = layers;
}
