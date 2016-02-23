#ifndef MAINWINDOW_H_INCLUDED__
#define MAINWINDOW_H_INCLUDED__

#include <memory>
#include <QMainWindow>

#include <session.h>
#include <cylindical-model-creator.h>

class QAction;
class ModelSettingsWidget;

namespace rn {
	class Viewport;
}

class MainWindow : public QMainWindow {
	Q_OBJECT

private:
	rn::Viewport* viewport_;
	ModelSettingsWidget* settings_widget_;
	struct {
		QAction* open;
		QAction* create;

		QAction* texturing;
		QAction* unite_meshes;
		QAction* enable_last_layer;
		QAction* enable_first_layer;
		QAction* select_mode;
 	} toolbar_;
	
private:
	std::shared_ptr<rn::Session> session_;
	std::shared_ptr<rn::ModelCreator> model_creator_;
	QList<vec3i> shifts_; // сдвиги, используются для перемещения моделей мышкой
	QPoint prev_mouse_; // предыдущие координаты мыши

	void openImage(const QString& filename);
	void createMainToolbar();
	void createMenuView();

public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow();

	void showEvent(QShowEvent*);
	void keyPressEvent(QKeyEvent* event) override;

	void dragEnterEvent(QDragEnterEvent* e) override;
	void dropEvent(QDropEvent* e) override;

private slots:
	void slotOpenImage();	

	void slotWheelEvent(QWheelEvent* event);
	void slotMouseMoveEvent(QMouseEvent* event);
	void slotMousePressEvent(QMouseEvent* event);
	void slotMouseReleaseEvent(QMouseEvent* event);
	void slotModelCreated(Mesh::HardPtr mesh);
	void slotInterruptCreatingProcess();
};

#endif // MAINWINDOW_H_INCLUDED__
