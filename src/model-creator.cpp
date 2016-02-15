#include <model-creator.h>

namespace rn {
	ModelCreator::ModelCreator():
		using_texturing(false),
		triangulate_first_layer(false),
		triangulate_last_layer(false),
		merge_models(false)
	{
	
	}

	ModelCreator::~ModelCreator() {
	
	}

	std::shared_ptr<rn::Session> ModelCreator::data() const {
		return data_;
	}

	void ModelCreator::setSessionData(std::shared_ptr<rn::Session> data) {
		data_ = data;
	}

	void ModelCreator::onMouseMove(int x, int y) {
		mouse_.x = x;
		mouse_.y = y;
	}

	void ModelCreator::onMousePress(Qt::MouseButton button) {
		buttons_[button] = true;
	}

	void ModelCreator::onMouseRelease(Qt::MouseButton button) {
		buttons_[button] = false;
	}
}
