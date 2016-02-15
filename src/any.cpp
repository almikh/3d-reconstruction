#include "any.h"

any::any(void) : content_(nullptr) {}
any::any(const any& other) : content_(other.content_ ? other.content_->clone() : nullptr) {}

any::~any(void) {
	delete content_;
}

any& any::swap(any& other) {
	std::swap(content_, other.content_);
	return *this;
}

any& any::operator=(const any& rhs) {
	if (&rhs == this) return *this;

	any(rhs).swap(*this);
	return *this;
}

void any::clear() {
	if (content_) {
		delete content_;
		content_ = nullptr;
	}
}

const std::type_info& any::type() const {
	return (content_ ? content_->type() : typeid(void));
}

bool any::empty() const {
	return content_ == nullptr;
}
