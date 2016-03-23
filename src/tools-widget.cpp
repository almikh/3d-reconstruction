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

  create = new QPushButton(QIcon("icons/create.png"), "", this);
  create->setIconSize(QSize(38, 38));
  create->setToolTip("Create model");
  create->setFixedSize(48, 48);
  create->setCheckable(true);
  layout->addWidget(create);

  cursor = new QPushButton(QIcon("icons/cursor.png"), "", this);
  cursor->setIconSize(QSize(38, 38));
  cursor->setToolTip("Cursor mode");
  cursor->setFixedSize(48, 48);
  cursor->setCheckable(true);
  layout->addWidget(cursor);

  smooth = new QPushButton(QIcon("icons/hand-pro.png"), "", this);
  smooth->setIconSize(QSize(38, 38));
  smooth->setToolTip("Hand mode");
  smooth->setFixedSize(48, 48);
  layout->addWidget(smooth);
  smooth->setEnabled(false);

  triangle_first_layer = new QPushButton(QIcon("icons/last-layer.png"), "", this);
  triangle_first_layer->setToolTip("Triangulate last layer");
  triangle_first_layer->setIconSize(QSize(38, 38));
  triangle_first_layer->setFixedSize(48, 48);
  layout->addWidget(triangle_first_layer);

  triangle_last_layer = new QPushButton(QIcon("icons/first-layer.png"), "", this);
  triangle_last_layer->setToolTip("Triangulate first layer");
  triangle_last_layer->setIconSize(QSize(38, 38));
  triangle_last_layer->setFixedSize(48, 48);
  layout->addWidget(triangle_last_layer);

  auto widget = new QWidget();
  widget->setLayout(layout);
  setCentralWidget(widget);

  setFixedSize(56, 48 * 5 + 4 * 6);
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
