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
  struct Toolbar {
    QAction* open;

    QAction* create;
    QAction* cursor;
    QAction* hand;

    QAction* texturing;
    QAction* unite_meshes;
    QAction* enable_last_layer;
    QAction* enable_first_layer;

    void resetOtherButtons(QAction* current);
  };

private:
  rn::Viewport* viewport_;
  ModelSettingsWidget* settings_widget_;
  Toolbar toolbar_;

private:
  std::shared_ptr<rn::Session> session_;
  std::shared_ptr<rn::ModelCreator> model_creator_;
  QList<vec3i> shifts_; // сдвиги, используются для перемещения моделей мышкой
  QPoint prev_mouse_; // предыдущие координаты мыши

  QPoint convertToSceneCoord(const QPoint& pos);

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
  void slotResetOtherButtons();
};

#endif // MAINWINDOW_H_INCLUDED__
