#ifndef TOOLS_WIDGET_H_INCLUDED__
#define TOOLS_WIDGET_H_INCLUDED__

#include <memory>
#include <QMainWindow>

class QSlider;
class QPushButton;

class ToolsWidget : public QMainWindow {
  Q_OBJECT

public:    
  QPushButton* create;
  QPushButton* cursor;
  QPushButton* smooth;

  QPushButton* triangle_last_layer;
  QPushButton* triangle_first_layer;

public:
  explicit ToolsWidget(QWidget* parent = nullptr);
  ~ToolsWidget();

  void resetOtherButtons(QPushButton* current);

  void hideEvent(QHideEvent*) override;

signals:
  void signalHide();
  void signalChangeStep(int step);
};

#endif // TOOLS_WIDGET_H_INCLUDED__
