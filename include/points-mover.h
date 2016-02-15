#ifndef POINTS_MOVER_H_INCLUDED__
#define POINTS_MOVER_H_INCLUDED__

#include <memory>
#include <QVector>

#include <image.h>
#include <vec2.h>

class PointsMover {
protected:
	int growth_length_; // �������� �����
	vec2d growth_dir_; // ����������� ����� (������-�): ����� ��������� �� <dir> - ���������� ������� ���������)
	vec2d change_dir_; // ����������� ���������
	vec2i offset_; // ����������� ��������
	bool look_ahead_; // ����������� �� ������, ��� ���
	std::shared_ptr<ip::Image<double>> grad_; // ���� ������� ���������
	std::shared_ptr<ip::Image<double>> grad_dir_; // ���� ����������� ���������
	QVector<QVector<vec2i>> prev_layers_; // ��� ���������� ����� (����)

public:
	virtual ~PointsMover();

	void setGrowthDir(const vec2d& dir);
	void setGrowthLength(int length);
	void setChangeDir(const vec2d& dir);
	void setOffset(const vec2i& offset);
	void setLookAhead(bool look);
	void setGradient(std::shared_ptr<ip::Image<double>> grad);
	void setGradientDir(std::shared_ptr<ip::Image<double>> dir);
	void setPrevLayers(const QVector<QVector<vec2i>>& layers);

	virtual void move(QVector<vec2i>& points) = 0;
};

#endif // POINTS_MOVER_H_INCLUDED__
