#include <mainwindow.h>
#include <QFileDialog>
#include <QInputDialog>
#include <QGraphicsScene>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QSettings>
#include <QPixmap>
#include <QDebug>
#include <QAction>
#include <QTimer>
#include <QToolBar>
#include <QAction>

#include <viewport.h>
#include <model-settings-widget.h>
#include <cylindical-model-creator.h>

/* MainWindow::Toolbar */
void MainWindow::Toolbar::resetOtherButtons(QAction* current) {
  if (create->isCheckable() && create != current) create->setChecked(false);
  if (cursor->isCheckable() && cursor != current) cursor->setChecked(false);
  if (hand->isCheckable() && hand != current) hand->setChecked(false);
}

/* MainWindow */
MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  viewport_(new rn::Viewport(this)),
  settings_widget_(new ModelSettingsWidget(this)),
  model_creator_(new rn::CylindricalModelCreator())
{
  setWindowTitle("3d-reconstruction");
  setCentralWidget(viewport_);

  viewport_->model_creator = model_creator_;
  //settings_widget_->show();

  createMainToolbar();
  createMenuView();

  setAcceptDrops(true);
  resize(800, 600);

  connect(settings_widget_->slices, &QSlider::valueChanged, [=](int value) {
    if (session_) {
      session_->slices = value;
    }
  });
  connect(settings_widget_->step, &QSlider::valueChanged, [=](int value) {
    if (session_) {
      session_->step = value;
    }
  });

  connect(model_creator_.get(), &rn::ModelCreator::signalModelCreated, this, &MainWindow::slotModelCreated);

  connect(viewport_, &rn::Viewport::signalWheelEvent, this, &MainWindow::slotWheelEvent);
  connect(viewport_, &rn::Viewport::signalMouseMoveEvent, this, &MainWindow::slotMouseMoveEvent);
  connect(viewport_, &rn::Viewport::signalMousePressEvent, this, &MainWindow::slotMousePressEvent);
  connect(viewport_, &rn::Viewport::signalMouseReleaseEvent, this, &MainWindow::slotMouseReleaseEvent);
}

MainWindow::~MainWindow() {

}

void MainWindow::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);

}

void MainWindow::keyPressEvent(QKeyEvent* event) {
  Q_UNUSED(event);

}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
  if (e->mimeData()->hasUrls()) {
    e->acceptProposedAction();
  }
}

void MainWindow::dropEvent(QDropEvent *e) {
  const QSet<QString> extensions = {
    "png",
    "jpeg",
    "jpg",
    "bmp"
  };

  for (auto& url : e->mimeData()->urls()) {
    QString filename = url.toLocalFile();
    QFileInfo info(filename);

    if (extensions.contains(info.suffix())) {
      openImage(filename);
    }
  }
}

void MainWindow::createMainToolbar() {
  auto toolbar = new QToolBar("Main toolbar");
  addToolBar(Qt::LeftToolBarArea, toolbar);
  toolbar->setIconSize(QSize(32, 32));

  toolbar_.open = toolbar->addAction(QIcon("icons/folder.png"), "Open image");
  connect(toolbar_.open, SIGNAL(triggered()), SLOT(slotOpenImage()));

  toolbar->addSeparator();

  toolbar_.create = toolbar->addAction(QIcon("icons/create.png"), "Create model");
  toolbar_.create->setCheckable(true);
  connect(toolbar_.create, SIGNAL(triggered()), SLOT(slotResetOtherButtons()));
  connect(toolbar_.create, SIGNAL(triggered()), SLOT(slotInterruptCreatingProcess()));
  connect(toolbar_.create, &QAction::triggered, [=](bool checked) {
    if (!session_) return;
    if (checked) session_->selected_meshes.clear();
  });

  toolbar_.cursor = toolbar->addAction(QIcon("icons/cursor.png"), "Cursor mode");
  toolbar_.cursor->setCheckable(true);
  connect(toolbar_.cursor, SIGNAL(triggered()), SLOT(slotResetOtherButtons()));
  connect(toolbar_.cursor, SIGNAL(triggered()), SLOT(slotInterruptCreatingProcess()));

  toolbar_.hand = toolbar->addAction(QIcon("icons/hand.png"), "Hand mode");
  toolbar_.hand->setCheckable(true);
  connect(toolbar_.hand, SIGNAL(triggered()), SLOT(slotResetOtherButtons()));
  connect(toolbar_.hand, SIGNAL(triggered()), SLOT(slotInterruptCreatingProcess()));

  toolbar->addSeparator();

  toolbar_.enable_first_layer = toolbar->addAction(QIcon("icons/show-first-layer.png"), "Triangulate first layer");
  toolbar_.enable_first_layer->setCheckable(true);
  connect(toolbar_.enable_first_layer, &QAction::triggered, [=](bool checked) {
    model_creator_->triangulate_first_layer = checked;
  });

  toolbar_.enable_last_layer = toolbar->addAction(QIcon("icons/show-last-layer.png"), "Triangulate last layer");
  toolbar_.enable_last_layer->setCheckable(true);
  connect(toolbar_.enable_last_layer, &QAction::triggered, [=](bool checked) {
    model_creator_->triangulate_last_layer = checked;
  });

  toolbar_.unite_meshes = toolbar->addAction(QIcon("icons/unite.png"), "Unite nearest meshes");
  toolbar_.unite_meshes->setCheckable(true);
  connect(toolbar_.unite_meshes, &QAction::triggered, [=](bool checked) {
    model_creator_->merge_models = checked;
  });

  toolbar_.texturing = toolbar->addAction(QIcon("icons/tex.png"), "Use texturing");
  toolbar_.texturing->setCheckable(true);
  connect(toolbar_.texturing, &QAction::triggered, [=](bool checked) {
    model_creator_->using_texturing = checked;
  });

  toolbar->addSeparator();

  // TODO
  //connect(toolbar_.select_mode, &QAction::triggered, [=](bool checked) {
  //  if (!session_) return;
  //  if (!checked) {
  //    session_->selected_meshes.clear();
  //  }

  //  viewport_->updateGL();
  //});
}

void MainWindow::createMenuView() {
  auto menu = menuBar()->addMenu("View");

  auto show_settings = menu->addAction("Show settings window");
  show_settings->setCheckable(true);
  show_settings->setChecked(!settings_widget_->isHidden());

  connect(settings_widget_, &ModelSettingsWidget::signalHide, [=]() {
    show_settings->setChecked(false);
  });

  connect(show_settings, &QAction::triggered, [=](bool checked) {
    if (checked) settings_widget_->show();
    else settings_widget_->hide();
  });
}

void MainWindow::openImage(const QString& filename) {
  viewport_->makeCurrent();
  session_.reset(new rn::Session(QImage(filename), viewport_));
  session_->slices = settings_widget_->slices->value();
  session_->step = settings_widget_->step->value();
  viewport_->setSession(session_);
  model_creator_->setSessionData(session_);

  model_creator_->merge_models = toolbar_.unite_meshes->isChecked();
  model_creator_->using_texturing = toolbar_.texturing->isChecked();
  model_creator_->triangulate_last_layer = toolbar_.enable_last_layer->isChecked();
  model_creator_->triangulate_first_layer = toolbar_.enable_first_layer->isChecked();

  viewport_->updateGL();
}

void MainWindow::slotOpenImage() {
  QString default_path = "";
  QSettings settings("settings.ini", QSettings::IniFormat);
  if (settings.contains("last-path")) default_path = settings.value("last-path").toString();
  QString filter = QString::fromLocal8Bit("Изображения (*.png *.jpg *.jpeg);;");
  QString filename = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("Выберите изображение:"), default_path, filter);
  if (!filename.isEmpty()) {
    settings.setValue("last-path", filename.left(filename.lastIndexOf('/')) + "/");
    openImage(filename);
  }
}

void MainWindow::slotWheelEvent(QWheelEvent* event) {
  if (toolbar_.create->isChecked()) return;

  double delta = event->delta();
  double sf = 1.0 + delta / 600;
  glScaled(sf, sf, sf);

  viewport_->updateGL();
}

QPoint MainWindow::convertToSceneCoord(const QPoint& pos) {
  Q_ASSERT(session_);

  QPoint scene_coord;
  scene_coord.setX(pos.x() - session_->screen_size.x / 2);
  scene_coord.setY(pos.y() - session_->screen_size.y / 2);

  return scene_coord;
}

void MainWindow::slotMousePressEvent(QMouseEvent* event) {
  if (toolbar_.create->isChecked() && event->button() == Qt::LeftButton) {
    model_creator_->onMouseMove(event->x(), event->y());
    model_creator_->onMousePress(event->button());
    viewport_->updateGL();
  }
  else if (toolbar_.cursor->isChecked()) {
    if (event->button() == Qt::RightButton) { // крутим модель
      static double model_view_matr[16];
      glGetDoublev(GL_MODELVIEW_MATRIX, model_view_matr);

      viewport_->trackball->setLastMatrix(model_view_matr);
      viewport_->trackball->click(event->x(), event->y());
      viewport_->updateGL();
    }
    else if (event->button() == Qt::LeftButton) { // выделяем отдельные меши
      auto pos = convertToSceneCoord(event->pos());
      viewport_->selected_area.push_back(pos);
      viewport_->selected_area.push_back(pos);
      viewport_->updateGL();
    }
  }
  else if (toolbar_.hand->isChecked() && event->button() == Qt::LeftButton) {
    // перемещаем выделенные меши
    if (!session_->selected_meshes.isEmpty()) {
      shifts_.clear();
      prev_mouse_ = convertToSceneCoord(event->pos());
      vec3i mouse(prev_mouse_.x(), prev_mouse_.y(), 0);
      for (auto mesh : session_->selected_meshes) {
        shifts_.push_back(mouse - mesh->center());
      }
    }
  }
}

void MainWindow::slotMouseMoveEvent(QMouseEvent* event) {
  if (toolbar_.create->isChecked()) {
    model_creator_->onMouseMove(event->x(), event->y());
    viewport_->updateGL();
  }
  else if (toolbar_.cursor->isChecked()) {
    if (event->buttons() & Qt::RightButton) { // крутим модель
      if (viewport_->trackball->isClicked()) {
        viewport_->trackball->rotate(event->x(), event->y());

        glLoadIdentity();
        glMultMatrixf(viewport_->trackball->getMat());

        viewport_->updateGL();
      }
    }
    else if (event->buttons() & Qt::LeftButton) { // выделяем отдельные меши
      if (!viewport_->selected_area.isEmpty()) {
        viewport_->selected_area.pop_back();
        viewport_->selected_area.push_back(convertToSceneCoord(event->pos()));
        viewport_->updateGL();
      }
    }
  }
  else if (toolbar_.hand->isChecked() && (event->buttons() & Qt::LeftButton)) {
    // перемещаем выделенные меши
    if (!session_->selected_meshes.isEmpty()) {
      Q_ASSERT(!shifts_.isEmpty());
      auto current_pos = convertToSceneCoord(event->pos());
      if (prev_mouse_ != current_pos) {
        for (auto mesh : session_->selected_meshes) {
          auto diff = current_pos - prev_mouse_;
          mesh->move(vec3i(diff.x(), diff.y(), 0));
        }

        prev_mouse_ = current_pos;
        viewport_->updateGL();
      }
    }
  }
}

void MainWindow::slotMouseReleaseEvent(QMouseEvent* event) {
  if (toolbar_.create->isChecked() && event->button() == Qt::LeftButton) {
    model_creator_->onMouseMove(event->x(), event->y());
    model_creator_->onMouseRelease(event->button());
    viewport_->updateGL();
  }
  else if (toolbar_.cursor->isChecked()) {
    if (event->button() == Qt::RightButton) { // крутим модель
      viewport_->trackball->release();
      viewport_->updateGL();
    }
    else if (event->button() == Qt::LeftButton) { // выделяем отдельные меши
      if (viewport_->selected_area.size() > 1) {
        viewport_->selected_area.pop_back();
        viewport_->selected_area.push_back(convertToSceneCoord(event->pos()));
        viewport_->updateGL();

        // определим меши, попавшие в область выделения
        session_->selected_meshes.clear();
        QRect region(viewport_->selected_area[0], viewport_->selected_area[1]);
        for (auto mesh : session_->meshes) {
          if (mesh->fallsInto(region)) {
            session_->selected_meshes.push_back(mesh);
          }
        }

        viewport_->selected_area.clear();
        viewport_->updateGL();
      }
    }
  }
  else if (toolbar_.hand->isChecked() && event->button() == Qt::LeftButton) {
    // перемещаем выделенные меши
    for (auto mesh : session_->selected_meshes) {
      model_creator_->place(mesh, 16); // TODO сделать спец. toolbar
    }

    shifts_.clear();
    viewport_->updateGL();
  }
}

void MainWindow::slotModelCreated(Mesh::HardPtr mesh) {
  toolbar_.create->setChecked(false);
}

void MainWindow::slotInterruptCreatingProcess() {
  if (!session_) return;

  auto sender = qobject_cast<QAction*>(QObject::sender());
  bool checked = sender->isChecked();

  bool need_interrupt = !checked && sender == toolbar_.create;
  need_interrupt |= checked && sender != toolbar_.create;
  if (need_interrupt) {
    model_creator_->OnInterruptRequest();
    viewport_->updateGL();
  }
}

void MainWindow::slotResetOtherButtons() {
  auto sender = qobject_cast<QAction*>(QObject::sender());
  bool checked = sender->isChecked();
  if (checked) {
    toolbar_.resetOtherButtons(sender);
    viewport_->updateGL();
  }
}
