#ifndef MAINWINDOW_H_INCLUDED__
#define MAINWINDOW_H_INCLUDED__

#include <memory>
#include <QMainWindow>

#include <session.h>

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
		QAction* texturing;
		QAction* unite_meshes;
		QAction* enable_last_layer;
		QAction* enable_first_layer;
 	} toolbar_;
	
private:
	std::shared_ptr<rn::Session> session_;

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
};

#endif // MAINWINDOW_H_INCLUDED__
