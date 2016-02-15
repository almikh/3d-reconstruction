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
		rn::Session::HardPtr data_; // ������ ������� ������

	public:
		bool using_texturing; // �������������� �� ����������� ������
		bool triangulate_first_layer; // ��������� �� "������ ������" ������
		bool triangulate_last_layer; // ��������� �� "������� ������" ������
		bool merge_models; // "���������" ������� ������

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