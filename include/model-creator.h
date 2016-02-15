#ifndef MODEL_CREATOR_H_INCLUDED__
#define MODEL_CREATOR_H_INCLUDED__
#include <memory>
#include <QVector>
#include <session.h>
#include <vec2.h>

namespace rn {
	using layer_t = QVector<vec2i>;

	class ModelCreator {
	protected:
		vec2i mouse_;
		QMap<Qt::MouseButton, bool> buttons_;
		rn::Session::HardPtr data_; // данные текущей сессии

	public:
		bool using_texturing; // текстурировать ли создаваемую модель
		bool triangulate_first_layer; // создавать ли "нижнюю крышку" модели
		bool triangulate_last_layer; // создавать ли "верхнюю крышку" модели
		bool merge_models; // "склеивать" близкие модели

	public:
		ModelCreator();
		virtual ~ModelCreator();

		std::shared_ptr<rn::Session> data() const;
		void setSessionData(std::shared_ptr<rn::Session> data);

		virtual void onMouseMove(int x, int y);
		virtual void onMousePress(Qt::MouseButton button);
		virtual void onMouseRelease(Qt::MouseButton button);

		virtual void render() {};
	};
}

#endif // MODEL_CREATOR_H_INCLUDED__