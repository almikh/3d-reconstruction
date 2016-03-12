#include <tools-widget.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QDebug>
#include <QIcon>

ToolsWidget::ToolsWidget(QWidget* parent) :
  QMainWindow(parent)
{
  setWindowFlags(Qt::Tool);
  setWindowTitle(" ");

  auto layout = new QVBoxLayout();
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setAlignment(Qt::AlignCenter);
  layout->setSpacing(4);

  create = new QPushButton(QIcon("icons/create.png"), "");
  create->setFixedSize(48, 48);
  create->setIconSize(QSize(38, 38));
  create->setToolTip("Create model");
  create->setCheckable(true);
  layout->addWidget(create);

  cursor = new QPushButton(QIcon("icons/cursor.png"), "");
  cursor->setFixedSize(48, 48);
  cursor->setIconSize(QSize(38, 38));
  cursor->setToolTip("Cursor mode");
  cursor->setCheckable(true);
  layout->addWidget(cursor);

  //hand = new QPushButton(QIcon("icons/hand.png"), "");
  //hand->setFixedSize(48, 48);
  //hand->setIconSize(QSize(38, 38));
  //hand->setToolTip("Hand mode");
  //hand->setCheckable(true);
  //layout->addWidget(hand);

  triangle_first_layer = new QPushButton(QIcon("icons/last-layer.png"), "");
  triangle_first_layer->setFixedSize(48, 48);
  triangle_first_layer->setIconSize(QSize(38, 38));
  triangle_first_layer->setToolTip("Triangulate last layer");
  layout->addWidget(triangle_first_layer);

  triangle_last_layer = new QPushButton(QIcon("icons/first-layer.png"), "");
  triangle_last_layer->setFixedSize(48, 48);
  triangle_last_layer->setIconSize(QSize(38, 38));
  triangle_last_layer->setToolTip("Triangulate first layer");
  layout->addWidget(triangle_last_layer);

  auto widget = new QWidget();
  widget->setLayout(layout);
  setCentralWidget(widget);

  setFixedSize(56, 48 * 4 + 4 * 5);
}

ToolsWidget::~ToolsWidget() {

}

void ToolsWidget::resetOtherButtons(QPushButton* current) {
  if (create != current) create->setChecked(false);
  if (cursor != current) cursor->setChecked(false);
}

void ToolsWidget::hideEvent(QHideEvent*) {
  emit signalHide();
}
