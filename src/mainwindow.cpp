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
#include <QComboBox>
#include <QAction>

#include <viewport.h>
#include <tools-widget.h>
#include <cylindical-model-creator.h>

#define ru(text) QString::fromLocal8Bit(text)

QStringList generate(int min, int max) {
  QStringList seq;
  for (int i = min; i < max; ++i) {
    seq << QString::number(i);
  }

  return seq;
}

/* MainWindow */
MainWindow::MainWindow(QWidget* parent) :
  QMainWindow(parent),
  viewport_(new rn::Viewport(this)),
  tools_(new ToolsWidget(this)),
  model_creator_(new rn::CylindricalModelCreator())
{
  setWindowTitle("3d-reconstruction");
  setCentralWidget(viewport_);

  viewport_->model_creator = model_creator_;
  tools_->show();

  createMainToolbar();
  createCreatingToolbar();
  createMovingToolbar();
  createMenuView();

  setAcceptDrops(true);
  resize(800, 600);

  connect(creating_toolbar_.slices, &QComboBox::currentTextChanged, [=](const QString& value) {
    if (session_) {
      session_->slices = value.toInt();
    }
  });
  connect(creating_toolbar_.step, &QComboBox::currentTextChanged, [=](const QString& value) {
    if (session_) {
      session_->step = value.toInt();
    }
  });

  connect(model_creator_.get(), &rn::ModelCreator::signalModelCreated, this, &MainWindow::slotModelCreated);

  connect(viewport_, &rn::Viewport::signalWheelEvent, this, &MainWindow::slotWheelEvent);
  connect(viewport_, &rn::Viewport::signalMouseMoveEvent, this, &MainWindow::slotMouseMoveEvent);
  connect(viewport_, &rn::Viewport::signalMousePressEvent, this, &MainWindow::slotMousePressEvent);
  connect(viewport_, &rn::Viewport::signalMouseReleaseEvent, this, &MainWindow::slotMouseReleaseEvent);

  connect(tools_->create, &QPushButton::clicked, this, &MainWindow::slotResetOtherButtons);
  connect(tools_->create, &QPushButton::clicked, this, &MainWindow::slotInterruptCreatingProcess);
  connect(tools_->create, &QPushButton::toggled, creating_toolbar_.toolbar, &QToolBar::setVisible);
  connect(tools_->create, &QPushButton::toggled, this, &MainWindow::slotChangeCreatingMode);

  connect(tools_->cursor, &QPushButton::clicked, this, &MainWindow::slotResetOtherButtons);
  connect(tools_->cursor, &QPushButton::clicked, this, &MainWindow::slotInterruptCreatingProcess);

  connect(tools_->hand, &QPushButton::clicked, this, &MainWindow::slotResetOtherButtons);
  connect(tools_->hand, &QPushButton::clicked, this, &MainWindow::slotInterruptCreatingProcess);
  connect(tools_->hand, &QPushButton::toggled, moving_toolbar_.toolbar, &QToolBar::setVisible);

  connect(tools_->triangle_first_layer, &QPushButton::clicked, [=](bool) {
    if (!session_) return;
    for (auto mesh : session_->selected_meshes) {
      mesh->triangulateFirstLayer();
      viewport_->updateGL();
    }
  });

  connect(tools_->triangle_last_layer, &QPushButton::clicked, [=](bool) {
    if (!session_) return;
    for (auto mesh: session_->selected_meshes) {
      mesh->triangulateLastLayer();
      viewport_->updateGL();
    }
  });

  QCoreApplication::instance()->installEventFilter(this);
}

MainWindow::~MainWindow() {

}

bool MainWindow::eventFilter(QObject* o, QEvent* e) {
  int type = e->type();
  if (o->isWidgetType() && (type == QEvent::KeyPress || type == QEvent::KeyRelease)) {
    QSet<int> keys;
    keys.insert(Qt::Key_H);
    QKeyEvent* ke = static_cast<QKeyEvent*>(e);
    QWidget* w = static_cast<QWidget*>(o);
    if (w != this && isAncestorOf(w)) {
      if (keys.contains(ke->key())) {
        QCoreApplication::instance()->sendEvent(this, ke);
        return true;
      }
    }
  }

  return false;
}

void MainWindow::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);

}

void MainWindow::keyPressEvent(QKeyEvent* event) {
  if (!event->isAutoRepeat() && event->key() == Qt::Key_H) { // временно скрывать изображение при отображении в окне
    viewport_->hide_image = true;
    viewport_->updateGL();
  }
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
  if (!event->isAutoRepeat() && event->key() == Qt::Key_H) {
    viewport_->hide_image = false;
    viewport_->updateGL();
  }
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
  main_toolbar_.toolbar = addToolBar("Main toolbar");
  main_toolbar_.toolbar->setIconSize(QSize(32, 32));

  main_toolbar_.open = main_toolbar_.toolbar->addAction(QIcon("icons/folder.png"), ru("Открыть изображение"));
  connect(main_toolbar_.open, SIGNAL(triggered()), SLOT(slotOpenImage()));

  main_toolbar_.save = main_toolbar_.toolbar->addAction(QIcon("icons/save.png"), ru("Сохранить полученные модели"));
  connect(main_toolbar_.save, SIGNAL(triggered()), SLOT(slotSaveMeshes()));
}

void MainWindow::createCreatingToolbar() {
  creating_toolbar_.toolbar = addToolBar("Creating toolbar");
  creating_toolbar_.toolbar->setIconSize(QSize(32, 32));
  creating_toolbar_.toolbar->setVisible(false);

  creating_toolbar_.mode = new QComboBox(this);
  creating_toolbar_.mode->setFont(QFont("Arial", 13));
  creating_toolbar_.mode->setFixedWidth(128);
  creating_toolbar_.mode->setFixedHeight(32);
  creating_toolbar_.mode->addItems({ "Normal", "Symmetrically" });
  creating_toolbar_.toolbar->addWidget(creating_toolbar_.mode);
  connect(creating_toolbar_.mode, &QComboBox::currentTextChanged, [=](const QString& type) {
    model_creator_->setPointsMover(type);
  });

  creating_toolbar_.toolbar->addSeparator();

  creating_toolbar_.unite_meshes = creating_toolbar_.toolbar->addAction(QIcon("icons/unite.png"), ru("Объединять ближайшие модели"));
  creating_toolbar_.unite_meshes->setCheckable(true);
  connect(creating_toolbar_.unite_meshes, &QAction::triggered, [=](bool checked) {
    model_creator_->merge_models = checked;
  });

  creating_toolbar_.toolbar->addSeparator();

  creating_toolbar_.texturing = creating_toolbar_.toolbar->addAction(QIcon("icons/texturing.png"), ru("Использовать текстурирование"));
  creating_toolbar_.texturing->setCheckable(true);
  connect(creating_toolbar_.texturing, &QAction::triggered, [=](bool checked) {
    creating_toolbar_.texturing_mode->setEnabled(checked);
    if (checked) {
      model_creator_->using_texturing = creating_toolbar_.texturing_mode->currentText();
    }
    else {
      model_creator_->using_texturing = "";
    }
  });

  creating_toolbar_.texturing_mode = new QComboBox(this);
  creating_toolbar_.texturing_mode->setFont(QFont("Arial", 13));
  creating_toolbar_.texturing_mode->setFixedWidth(100);
  creating_toolbar_.texturing_mode->setFixedHeight(32);
  creating_toolbar_.texturing_mode->addItems({ "Mirror", "Cyclically" });
  creating_toolbar_.texturing_mode->setEnabled(false);
  creating_toolbar_.toolbar->addWidget(creating_toolbar_.texturing_mode);
  connect(creating_toolbar_.texturing_mode, &QComboBox::currentTextChanged, [=](const QString& text) {
    model_creator_->using_texturing = text;
  });

  creating_toolbar_.toolbar->addSeparator();

  auto step_label = new QLabel(ru(" Шаг: "), this);
  step_label->setFont(QFont("Arial", 13));
  creating_toolbar_.toolbar->addWidget(step_label);

  creating_toolbar_.step = new QComboBox(this);
  creating_toolbar_.step->setFont(QFont("Arial", 13));
  creating_toolbar_.step->addItems(generate(1, 15));
  creating_toolbar_.step->setCurrentText("5");
  creating_toolbar_.step->setFixedWidth(100);
  creating_toolbar_.toolbar->addWidget(creating_toolbar_.step);

  creating_toolbar_.toolbar->addSeparator();

  auto slices_label = new QLabel(ru(" Ломтиков: "), this);
  slices_label->setFont(QFont("Arial", 13));
  creating_toolbar_.toolbar->addWidget(slices_label);

  creating_toolbar_.slices = new QComboBox(this);
  creating_toolbar_.slices->setFont(QFont("Arial", 13));
  creating_toolbar_.slices->addItems(generate(4, 360));
  creating_toolbar_.slices->setCurrentText("45");
  creating_toolbar_.slices->setFixedWidth(100);
  creating_toolbar_.toolbar->addWidget(creating_toolbar_.slices);
}

void MainWindow::createMovingToolbar() {
  moving_toolbar_.toolbar = addToolBar("Creating toolbar");
  moving_toolbar_.toolbar->setIconSize(QSize(32, 32));
  moving_toolbar_.toolbar->setVisible(false);

  auto radius_label = new QLabel(ru(" Радиус: "), this);
  radius_label->setFont(QFont("Arial", 13));
  moving_toolbar_.toolbar->addWidget(radius_label);

  moving_toolbar_.radius = new QComboBox(this);
  moving_toolbar_.radius->setFont(QFont("Arial", 13));
  moving_toolbar_.radius->addItems(generate(0, 16));
  moving_toolbar_.radius->setCurrentText("4");
  moving_toolbar_.radius->setFixedWidth(80);
  moving_toolbar_.toolbar->addWidget(moving_toolbar_.radius);
}

void MainWindow::createMenuView() {
  auto menu = menuBar()->addMenu("View");

  auto show_settings = menu->addAction(ru("Главная панель инструментов"));
  show_settings->setIcon(QIcon("icons/main-toolbar.png"));
  show_settings->setCheckable(true);
  show_settings->setChecked(!tools_->isHidden());

  connect(tools_, &ToolsWidget::signalHide, [=]() {
    show_settings->setChecked(false);
  });

  connect(show_settings, &QAction::triggered, [=](bool checked) {
    if (checked) tools_->show();
    else tools_->hide();
  });
}

void MainWindow::changeStateOfTriangleButtons() {
  if (!session_->selected_meshes.isEmpty()) {
    tools_->triangle_last_layer->setEnabled(true);
    tools_->triangle_first_layer->setEnabled(true);
  }
  else {
    tools_->triangle_last_layer->setEnabled(false);
    tools_->triangle_first_layer->setEnabled(false);
  }
}

void MainWindow::openImage(const QString& filename) {
  tools_->create->setChecked(true);

  viewport_->makeCurrent();
  session_.reset(new rn::Session(QImage(filename), viewport_));
  session_->slices = creating_toolbar_.slices->currentText().toInt();
  session_->step = creating_toolbar_.step->currentText().toInt();
  viewport_->setSession(session_);
  model_creator_->setSessionData(session_);

  model_creator_->merge_models = creating_toolbar_.unite_meshes->isChecked();
  if (creating_toolbar_.texturing->isChecked()) {
    model_creator_->using_texturing = creating_toolbar_.texturing_mode->currentText();
  }
  else {
    model_creator_->using_texturing = "";
  }

  viewport_->updateGL();
}

void MainWindow::askAboutSaving() {
  if (tools_->create->isChecked() && session_ && !session_->meshes.isEmpty()) {
    auto title = ru("Сохранение");
    auto message = ru(
          "Вы хотите сохранить уже созданные модели в файл? "
          "После открытия нового файла или нового сеанса создания они будет удалены.");
    auto answer = QMessageBox::question(this, title, message);
    if (answer == QMessageBox::Yes) {
      slotSaveMeshes();
    }
  }
}

QPoint MainWindow::convertToSceneCoord(const QPoint& pos) {
  Q_ASSERT(session_);

  QPoint scene_coord;
  scene_coord.setX(pos.x() - session_->screen_size.x / 2);
  scene_coord.setY(pos.y() - session_->screen_size.y / 2);

  return scene_coord;
}

void MainWindow::slotSaveMeshes() {

}

void MainWindow::slotOpenImage() {
  askAboutSaving();
  session_.reset();
  slotResetOtherButtons();

  QString default_path = "";
  QSettings settings("settings.ini", QSettings::IniFormat);
  if (settings.contains("last-path")) default_path = settings.value("last-path").toString();
  QString filter = ru("Изображения (*.png *.jpg *.jpeg);;");
  QString filename = QFileDialog::getOpenFileName(this, ru("Выберите изображение:"), default_path, filter);
  if (!filename.isEmpty()) {
    settings.setValue("last-path", filename.left(filename.lastIndexOf('/')) + "/");
    openImage(filename);
  }
}

void MainWindow::slotWheelEvent(QWheelEvent* event) {
  if (tools_->create->isChecked()) return;

  double delta = event->delta();
  double sf = 1.0 + delta / 600;
  glScaled(sf, sf, sf);

  viewport_->updateGL();
}

void MainWindow::slotChangeCreatingMode(bool checked) {
  if (!session_) return;

  if (checked) {
    session_->meshes.clear();
    session_->selected_meshes.clear();
  }
  else {
    askAboutSaving();
  }

  changeStateOfTriangleButtons();
  viewport_->updateGL();
}

void MainWindow::slotMousePressEvent(QMouseEvent* event) {
  if (tools_->create->isChecked() && event->button() == Qt::LeftButton) {
    model_creator_->onMouseMove(event->x(), event->y());
    model_creator_->onMousePress(event->button());
    viewport_->updateGL();
  }
  else if (tools_->cursor->isChecked()) {
    if (event->button() == Qt::RightButton) { // крутим модель
      static double model_view_matr[16];
      glGetDoublev(GL_MODELVIEW_MATRIX, model_view_matr);

      viewport_->trackball->setLastMatrix(model_view_matr);
      viewport_->trackball->click(event->x(), event->y());
      viewport_->updateGL();
    }
    else if (event->button() == Qt::LeftButton) { // выделяем отдельные меши
      auto pos = convertToSceneCoord(event->pos());

      session_->selected_meshes.clear();
      for (auto mesh : session_->meshes) {
        if (mesh->contains(pos)) {
          session_->selected_meshes.push_back(mesh);
        }
      }

      if (session_->selected_meshes.isEmpty()) {
        viewport_->selected_area.push_back(pos);
        viewport_->selected_area.push_back(pos);
      }
      else {
        changeStateOfTriangleButtons();
      }

      viewport_->updateGL();
    }
  }
  else if (tools_->hand->isChecked() && event->button() == Qt::LeftButton) {
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
  if (tools_->create->isChecked()) {
    model_creator_->onMouseMove(event->x(), event->y());
    viewport_->updateGL();
  }
  else if (tools_->cursor->isChecked()) {
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
  else if (tools_->hand->isChecked() && (event->buttons() & Qt::LeftButton)) {
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
  if (tools_->create->isChecked() && event->button() == Qt::LeftButton) {
    model_creator_->onMouseMove(event->x(), event->y());
    model_creator_->onMouseRelease(event->button());
    viewport_->updateGL();
  }
  else if (tools_->cursor->isChecked()) {
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

        changeStateOfTriangleButtons();

        viewport_->selected_area.clear();
        viewport_->updateGL();
      }
    }
  }
  else if (tools_->hand->isChecked() && event->button() == Qt::LeftButton) {
    // перемещаем выделенные меши
    int radius = moving_toolbar_.radius->currentText().toInt();
    for (auto mesh : session_->selected_meshes) {
      model_creator_->place(mesh, radius);
    }

    shifts_.clear();
    viewport_->updateGL();
  }
}

void MainWindow::slotModelCreated(Mesh::HardPtr mesh) {
  //tools_->create->setChecked(false);
}

void MainWindow::slotInterruptCreatingProcess() {
  if (!session_) return;

  auto sender = qobject_cast<QPushButton*>(QObject::sender());
  bool checked = sender->isChecked();

  bool need_interrupt = !checked && sender == tools_->create;
  need_interrupt |= checked && sender != tools_->create;
  if (need_interrupt) {
    model_creator_->OnInterruptRequest();
    viewport_->updateGL();
  }
}

void MainWindow::slotResetOtherButtons() {
  auto sender = qobject_cast<QPushButton*>(QObject::sender());
  if (!sender || !sender->isCheckable() || sender->isChecked()) {
    tools_->resetOtherButtons(sender);
    viewport_->updateGL();
  }
}
