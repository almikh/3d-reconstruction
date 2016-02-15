#ifndef POINTS_MOVER_H_INCLUDED__
#define POINTS_MOVER_H_INCLUDED__

#include <memory>
#include <QVector>

#include <image.h>
#include <vec2.h>

class PointsMover {
protected:
	int growth_length_; // величина роста
	vec2d growth_dir_; // направление роста (нормир-е): после изменения на <dir> - происходит процесс изменения)
	vec2d change_dir_; // направление изменения
	vec2i offset_; // учитываемое смещение
	bool look_ahead_; // заглядывать ли вперед, или нет
	std::shared_ptr<ip::Image<double>> grad_; // поле модулей градиента
	std::shared_ptr<ip::Image<double>> grad_dir_; // поле направлений градиента
	QVector<QVector<vec2i>> prev_layers_; // все предыдущие точки (слои)

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
