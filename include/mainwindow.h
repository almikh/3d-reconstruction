#ifndef MAINWINDOW_H_INCLUDED__
#define MAINWINDOW_H_INCLUDED__

#include <memory>
#include <QMainWindow>

#include <session.h>
#include <cylindical-model-creator.h>

class QAction;
class QSlider;
class QComboBox;
class ToolsWidget;

namespace rn {
  class Viewport;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

private:
  rn::Viewport* viewport_;
  ToolsWidget* tools_;

private:
  std::shared_ptr<rn::Session> session_;
  std::shared_ptr<rn::ModelCreator> model_creator_;
  QList<vec3i> shifts_; // сдвиги, используются для перемещения моделей мышкой
  QPoint prev_mouse_; // предыдущие координаты мыши

  struct {
    QToolBar* toolbar;
    QAction* open;
    QAction* save;
    QAction* undo;

  } main_toolbar_;

  struct {
    QToolBar* toolbar;

    QComboBox* mode;
    QComboBox* step;
    QComboBox* slices;
    QAction* texturing;
    QComboBox* texturing_mode;
    QAction* unite_meshes;
    QAction* mirror;
    QAction* copy;
  } creating_toolbar_;

  struct {
    QToolBar* toolbar;

    QComboBox* radius;
  } moving_toolbar_;

  QPoint convertToSceneCoord(const QPoint& pos);

  void openImage(const QString& filename);
  void createMainToolbar();
  void createCreatingToolbar();
  void createMovingToolbar();
  void createMenuView();

  void changeStateOfTriangleButtons();

  void askAboutSaving();

protected:
  bool eventFilter(QObject* o, QEvent* e) override;

public:
  explicit MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

  void setRightPosForTools(); // установит правильную позицию виджету инструментов

  void showEvent(QShowEvent* e) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;

  void dragEnterEvent(QDragEnterEvent* e) override;
  void dropEvent(QDropEvent* e) override;

  void moveEvent(QMoveEvent* e) override;

private slots:
  void slotOpenImage();
  void slotSaveMeshes();
  void slotUndoLastAction();

  void slotChangeCreatingMode(bool checked);

  void slotWheelEvent(QWheelEvent* event);
  void slotMouseMoveEvent(QMouseEvent* event);
  void slotMousePressEvent(QMouseEvent* event);
  void slotMouseReleaseEvent(QMouseEvent* event);
  void slotBeforeNewModelCreating();
  void slotMirrorSelectedMeshes();

  void slotInterruptCreatingProcess();
  void slotResetOtherButtons();
};

#endif // MAINWINDOW_H_INCLUDED__
