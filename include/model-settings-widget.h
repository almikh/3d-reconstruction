#ifndef MODEL_SETTINGS_WIDGET_H_INCLUDED__
#define MODEL_SETTINGS_WIDGET_H_INCLUDED__

#include <memory>
#include <QMainWindow>

class QSlider;

class ModelSettingsWidget : public QMainWindow {
	Q_OBJECT

public:
	QSlider* step;
	QSlider* slices;

public:
	explicit ModelSettingsWidget(QWidget* parent = nullptr);
	~ModelSettingsWidget();

	void hideEvent(QHideEvent*);

signals:
	void signalHide();
	void signalChangeStep(int step);
};

#endif // MODEL_SETTINGS_WIDGET_H_INCLUDED__
