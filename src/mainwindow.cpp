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
  createMenuFile();
  createMenuView();

  setAcceptDrops(true);
  setMinimumSize(MIN_SCENE_WIDTH, MIN_SCENE_HEIGHT);
  resize(MIN_SCENE_WIDTH, MIN_SCENE_HEIGHT);

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

  connect(model_creator_.get(), &rn::ModelCreator::signalBeforeNewModelCreating, this, &MainWindow::slotBeforeNewModelCreating);

  connect(viewport_, &rn::Viewport::signalWheelEvent, this, &MainWindow::slotWheelEvent);
  connect(viewport_, &rn::Viewport::signalMouseMoveEvent, this, &MainWindow::slotMouseMoveEvent);
  connect(viewport_, &rn::Viewport::signalMousePressEvent, this, &MainWindow::slotMousePressEvent);
  connect(viewport_, &rn::Viewport::signalMouseReleaseEvent, this, &MainWindow::slotMouseReleaseEvent);

  connect(tools_->create, &QPushButton::clicked, this, &MainWindow::slotResetOtherButtons);
  connect(tools_->create, &QPushButton::clicked, this, &MainWindow::slotInterruptCreatingProcess);
  connect(tools_->create, &QPushButton::toggled, creating_toolbar_.toolbar, &QToolBar::setVisible);
  connect(tools_->create, &QPushButton::toggled, this, &MainWindow::slotChangeCreatingMode);

  connect(tools_->cursor, &QPushButton::clicked, this, &MainWindow::slotInterruptCreatingProcess);
  connect(tools_->cursor, &QPushButton::toggled, moving_toolbar_.toolbar, &QToolBar::setVisible);
  connect(tools_->cursor, &QPushButton::clicked, [=](bool checked) {
    if (!checked) {
      session_->selected_meshes.clear();
      onSelectionChange();
    }
  });

  connect(tools_->smooth, &QPushButton::clicked, [=]() {
    if (!session_) return;
    for (auto mesh : session_->selected_meshes) {
      dynamic_cast<rn::CylindricalModelCreator*>(model_creator_.get())->smoothWithAveraging(mesh);
      viewport_->updateGL();
    }
  });

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

void MainWindow::setRightPosForTools() {
  int dy = (height() - centralWidget()->height()) + main_toolbar_.toolbar->height();
  tools_->move(pos().x() + 8, pos().y() + dy);
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
  if (show_image_->isChecked() && !event->isAutoRepeat() && event->key() == Qt::Key_H) { // временно скрывать изображение при отображении в окне
    viewport_->hide_image = true;
    viewport_->updateGL();
  }
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
  if (show_image_->isChecked() && !event->isAutoRepeat() && event->key() == Qt::Key_H) {
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

void MainWindow::moveEvent(QMoveEvent* e) {
  QMainWindow::moveEvent(e);

  auto shift = e->pos() - e->oldPos();
  tools_->move(tools_->pos() + shift);
}

void MainWindow::createMainToolbar() {
  main_toolbar_.toolbar = addToolBar("Main toolbar");
  main_toolbar_.toolbar->setIconSize(QSize(32, 32));

  main_toolbar_.open = main_toolbar_.toolbar->addAction(QIcon("icons/folder.png"), ru("Открыть изображение"));
  connect(main_toolbar_.open, SIGNAL(triggered()), SLOT(slotOpenImage()));

  main_toolbar_.save = main_toolbar_.toolbar->addAction(QIcon("icons/save.png"), ru("Сохранить полученные модели"));
  connect(main_toolbar_.save, SIGNAL(triggered()), SLOT(slotSaveMeshes()));

  main_toolbar_.toolbar->addSeparator();

  main_toolbar_.screenshot = main_toolbar_.toolbar->addAction(QIcon("icons/screenshot.png"), ru("Сохранить сцену в файл"));
  connect(main_toolbar_.screenshot, SIGNAL(triggered()), SLOT(slotMakeScreenshot()));

  main_toolbar_.toolbar->addSeparator();

  main_toolbar_.undo = main_toolbar_.toolbar->addAction(QIcon("icons/undo.png"), ru("Отменить последнее действие"));
  connect(main_toolbar_.undo, SIGNAL(triggered()), SLOT(slotUndoLastAction()));
  main_toolbar_.undo->setShortcut(QKeySequence("CTRL+Z"));
}

void MainWindow::createCreatingToolbar() {
  creating_toolbar_.toolbar = addToolBar("Creating toolbar");
  creating_toolbar_.toolbar->setIconSize(QSize(32, 32));
  creating_toolbar_.toolbar->setVisible(false);

  creating_toolbar_.mode = new QComboBox(this);
  creating_toolbar_.mode->setFont(QFont("Arial", 13));
  creating_toolbar_.mode->setFixedWidth(136);
  creating_toolbar_.mode->addItems({ ru("По-умолчанию"), ru("Симметрично") });
  creating_toolbar_.toolbar->addWidget(creating_toolbar_.mode);
  connect(creating_toolbar_.mode, &QComboBox::currentTextChanged, [=](const QString&) {
    auto mode = (rn::ModelCreator::CreatingMode)creating_toolbar_.mode->currentIndex();
    model_creator_->setPointsMover(mode);
  });

  creating_toolbar_.toolbar->addSeparator();

  creating_toolbar_.mirror = creating_toolbar_.toolbar->addAction(QIcon("icons/mirror.png"), ru("Отразить по-горизонтали"));
  creating_toolbar_.mirror->setShortcut(QKeySequence("CTRL+M"));
  creating_toolbar_.mirror->setEnabled(false);
  connect(creating_toolbar_.mirror, &QAction::triggered, this, &MainWindow::slotMirrorSelectedMeshes);
  creating_toolbar_.toolbar->addSeparator();

  creating_toolbar_.copy = creating_toolbar_.toolbar->addAction(QIcon("icons/copy.png"), ru("Создать копию"));
  creating_toolbar_.copy->setShortcut(QKeySequence("CTRL+C"));
  creating_toolbar_.copy->setEnabled(false);
  connect(creating_toolbar_.copy, &QAction::triggered, [=]() {
    if (session_) {
      for (auto mesh : session_->selected_meshes) {
        session_->meshes.push_back(mesh->clone());
      }
    }
  });

  creating_toolbar_.toolbar->addSeparator();

  creating_toolbar_.unite_meshes = creating_toolbar_.toolbar->addAction(QIcon("icons/unite.png"), ru("Объединять ближайшие модели"));
  connect(creating_toolbar_.unite_meshes, &QAction::triggered, this, &MainWindow::slotUniteSelectedMeshes);
  creating_toolbar_.unite_meshes->setEnabled(false);

  creating_toolbar_.toolbar->addSeparator();

  creating_toolbar_.texturing = creating_toolbar_.toolbar->addAction(QIcon("icons/texturing.png"), ru("Использовать текстурирование"));
  creating_toolbar_.texturing->setCheckable(true);
  connect(creating_toolbar_.texturing, &QAction::triggered, [=](bool checked) {
    creating_toolbar_.texturing_mode->setEnabled(checked);
    model_creator_->using_texturing = checked;
    if (checked) {
      auto mode = creating_toolbar_.texturing_mode->currentIndex();
      model_creator_->texturing_mode = mode;
    }
  });

  creating_toolbar_.texturing_mode = new QComboBox(this);
  creating_toolbar_.texturing_mode->setFont(QFont("Arial", 13));
  creating_toolbar_.texturing_mode->setFixedWidth(128);
  creating_toolbar_.texturing_mode->addItems({ ru("Зеркально"), ru("Циклически") });
  creating_toolbar_.texturing_mode->setEnabled(false);
  creating_toolbar_.toolbar->addWidget(creating_toolbar_.texturing_mode);
  connect(creating_toolbar_.texturing_mode, &QComboBox::currentTextChanged, [=](const QString&) {
    auto mode = creating_toolbar_.texturing_mode->currentIndex();
    model_creator_->texturing_mode = mode;
  });

  creating_toolbar_.toolbar->addSeparator();

  auto step_label = new QLabel(ru(" Шаг: "), this);
  step_label->setFont(QFont("Arial", 13));
  creating_toolbar_.toolbar->addWidget(step_label);

  creating_toolbar_.step = new QComboBox(this);
  creating_toolbar_.step->setFont(QFont("Arial", 13));
  creating_toolbar_.step->addItems(generate(1, 15));
  creating_toolbar_.step->setCurrentText("5");
  creating_toolbar_.step->setFixedWidth(48);
  creating_toolbar_.toolbar->addWidget(creating_toolbar_.step);

  creating_toolbar_.toolbar->addSeparator();

  auto slices_label = new QLabel(ru(" Ломтиков: "), this);
  slices_label->setFont(QFont("Arial", 13));
  creating_toolbar_.toolbar->addWidget(slices_label);

  creating_toolbar_.slices = new QComboBox(this);
  creating_toolbar_.slices->setFont(QFont("Arial", 13));
  creating_toolbar_.slices->addItems(generate(4, 360));
  creating_toolbar_.slices->setCurrentText("45");
  creating_toolbar_.slices->setFixedWidth(64);
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
  moving_toolbar_.radius->setFixedWidth(64);
  moving_toolbar_.toolbar->addWidget(moving_toolbar_.radius);
}

void MainWindow::createMenuView() {
  auto menu = menuBar()->addMenu(ru("Вид"));

  auto show_settings = menu->addAction(ru("Инструменты"));
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

  auto show_force_field = menu->addAction(ru("Показать поле сил"));
  show_force_field->setIcon(QIcon("icons/grad.png"));
  show_force_field->setCheckable(true);

  connect(show_force_field, &QAction::triggered, [=](bool checked) {
    viewport_->show_force_field = checked;
    viewport_->updateGL();
  });

  show_image_ = menu->addAction(ru("Показать изображение"));
  show_image_->setIcon(QIcon("icons/show-image.png"));
  show_image_->setShortcut(QKeySequence("CTRL+S"));
  show_image_->setCheckable(true);
  show_image_->setChecked(true);

  connect(show_image_, &QAction::triggered, [=](bool checked) {
    viewport_->hide_image = !checked;
    viewport_->updateGL();
  });
}

void MainWindow::createMenuFile() {
  auto menu = menuBar()->addMenu(ru("Файл"));

  menu_file_.open = menu->addAction(ru("Открыть"));
  menu_file_.open->setIcon(QIcon("icons/folder.png"));
  connect(menu_file_.open, &QAction::triggered, this, &MainWindow::slotOpenImage);

  menu->addSeparator();

  menu_file_.save = menu->addAction(ru("Сохранить"));
  menu_file_.save->setIcon(QIcon("icons/save.png"));
  connect(menu_file_.save, &QAction::triggered, this, &MainWindow::slotSaveMeshes);

  menu_file_.save_each = menu->addAction(ru("Сохранить каждую модель в отдельный файл"));
  menu_file_.save_each->setIcon(QIcon("icons/save-each.png"));
  connect(menu_file_.save_each, &QAction::triggered, this, &MainWindow::slotSaveEachMeshes);

  menu->addSeparator();

  auto quit = menu->addAction(ru("Выход"));
  connect(quit, &QAction::triggered, qApp, &QApplication::quit);
}

void MainWindow::onSelectionChange() {
  if (!session_->selected_meshes.isEmpty()) {
    creating_toolbar_.copy->setEnabled(true);
    creating_toolbar_.mirror->setEnabled(true);
    creating_toolbar_.unite_meshes->setEnabled(true);
    tools_->triangle_last_layer->setEnabled(true);
    tools_->triangle_first_layer->setEnabled(true);
  }
  else {
    creating_toolbar_.copy->setEnabled(false);
    creating_toolbar_.mirror->setEnabled(false);
    creating_toolbar_.unite_meshes->setEnabled(false);
    tools_->triangle_last_layer->setEnabled(false);
    tools_->triangle_first_layer->setEnabled(false);
  }
}

void MainWindow::openImage(const QString& filename) {
  tools_->create->setChecked(true);
  viewport_->hide_image = false;
  show_image_->setChecked(true);

  viewport_->makeCurrent();
  session_.reset(new rn::Session(QImage(filename), viewport_));
  session_->slices = creating_toolbar_.slices->currentText().toInt();
  session_->step = creating_toolbar_.step->currentText().toInt();
  viewport_->setSession(session_);
  model_creator_->setSessionData(session_);

  model_creator_->texturing_mode = creating_toolbar_.texturing_mode->currentIndex();
  model_creator_->using_texturing = creating_toolbar_.texturing->isChecked();

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
  if (!session_) return;
  Mesh::HardPtr common = session_->meshes.first();
  for (int i = 1; i < session_->meshes.size(); ++i) {
    common = Mesh::merge(common, session_->meshes[i]);
  }

  QString default_dir = "/";
  auto caption = ru("Сохранить модель(и) как:");
  auto path = QFileDialog::getSaveFileName(this, caption, default_dir, ru("Формат OBJ (*.obj);;"));
  if (path.isEmpty()) return;

  if (!path.endsWith(".obj")) path += ".obj";
  common->saveAsObj(path.toLocal8Bit().data());
}

void MainWindow::slotMakeScreenshot() {
  QString default_dir = "/";
  auto caption = ru("Сохранить скриншот как:");
  auto filename = QFileDialog::getSaveFileName(this, caption, default_dir, ru("Формат PNG (*.png);;"));
  if (!filename.isEmpty()) {
    viewport_->makeScreenshot(filename);
  }
}

void MainWindow::slotSaveEachMeshes() {
  if (!session_) return;
  for (int i = 0; i < session_->meshes.size(); ++i) {
    QString default_dir = "/";
    auto caption = ru("Сохранить модель #%1 как:").arg(i);
    auto path = QFileDialog::getSaveFileName(this, caption, default_dir, ru("Формат OBJ (*.obj);;"));
    if (path.isEmpty()) continue;

    if (!path.endsWith(".obj")) path += ".obj";
    session_->meshes[i]->saveAsObj(path.toLocal8Bit().data());
  }
}

void MainWindow::slotUndoLastAction() {
  session_->rollback();
  viewport_->updateGL();
  main_toolbar_.undo->setEnabled(session_->hasBackups());
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

  onSelectionChange();
  viewport_->updateGL();
}

void MainWindow::slotMousePressEvent(QMouseEvent* event) {
  if (tools_->cursor->isChecked()) {
    auto modifiers = QApplication::keyboardModifiers();
    if (modifiers == Qt::ControlModifier && event->button() == Qt::RightButton) { // крутим модель
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
        onSelectionChange();
      }

      viewport_->updateGL();
    }
    else if (event->button() == Qt::RightButton) { // перемещаем выделенные меши
      if (!session_->selected_meshes.isEmpty()) {
        QApplication::setOverrideCursor(QCursor(Qt::SizeAllCursor));

        shifts_.clear();
        prev_mouse_ = convertToSceneCoord(event->pos());
        vec3i mouse(prev_mouse_.x(), prev_mouse_.y(), 0);
        for (auto mesh : session_->selected_meshes) {
          shifts_.push_back(mouse - mesh->center());
        }
      }
    }
  }
  else if (tools_->create->isChecked() && event->button() == Qt::LeftButton) {
    model_creator_->onMouseMove(event->x(), event->y());
    model_creator_->onMousePress(event->button());
    viewport_->updateGL();
  }
}

void MainWindow::slotMouseMoveEvent(QMouseEvent* event) {
  if (tools_->cursor->isChecked()) {
    auto modifiers = QApplication::keyboardModifiers();
    if (modifiers == Qt::ControlModifier && event->buttons() & Qt::RightButton) { // крутим модель
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
    else if (event->buttons() & Qt::RightButton) { // перемещаем выделенные меши
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
  else if (tools_->create->isChecked()) {
    model_creator_->onMouseMove(event->x(), event->y());
    viewport_->updateGL();
  }
}

void MainWindow::slotMouseReleaseEvent(QMouseEvent* event) {
  if (tools_->cursor->isChecked()) {
    auto modifiers = QApplication::keyboardModifiers();
    if (modifiers == Qt::ControlModifier && event->button() == Qt::RightButton) { // крутим модель
      viewport_->trackball->release();
      viewport_->updateGL();
    }
    else if(event->button() == Qt::LeftButton) { // выделяем отдельные меши
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

        onSelectionChange();

        viewport_->selected_area.clear();
        viewport_->updateGL();
      }
    }
    else if (event->button() == Qt::RightButton) { // перемещаем выделенные меши
      if (!session_->selected_meshes.isEmpty()) {
        QApplication::restoreOverrideCursor();
        slotBeforeNewModelCreating();

        QList<Mesh::HardPtr> changed_meshes; // нужно для механизма UNDO
        int radius = moving_toolbar_.radius->currentText().toInt();
        for (auto mesh : session_->selected_meshes) {
          auto new_mesh = mesh->clone();
          session_->meshes.removeOne(mesh);
          changed_meshes.push_back(new_mesh);
          model_creator_->place(new_mesh, radius);
          session_->meshes.push_back(new_mesh);
        }

        // теперь выделенными являются обновленные меши
        session_->selected_meshes = changed_meshes;

        shifts_.clear();
        viewport_->updateGL();
      }
    }
  }
  else if (tools_->create->isChecked() && event->button() == Qt::LeftButton) {
    model_creator_->onMouseMove(event->x(), event->y());
    model_creator_->onMouseRelease(event->button());
    viewport_->updateGL();
  }
}

void MainWindow::slotBeforeNewModelCreating() {
  session_->commit();
  main_toolbar_.undo->setEnabled(true);
}

void MainWindow::slotMirrorSelectedMeshes() {
  if (!session_) return;

  QList<Mesh::HardPtr> changed_meshes; // нужно для механизма UNDO
  int symmetry_axis = 0; // если мешей несколько, то отражать будем по средней оси
  if (session_->selected_meshes.size() > 1) {
    for (auto mesh : session_->selected_meshes) {
      symmetry_axis += mesh->center().x;
    }

    symmetry_axis /= session_->selected_meshes.size();
  }

  slotBeforeNewModelCreating();
  for (auto mesh : session_->selected_meshes) {
    auto new_mesh = mesh->clone();
    session_->meshes.removeOne(mesh);

    new_mesh->mirror(symmetry_axis);
    changed_meshes.push_back(new_mesh);
    session_->meshes.push_back(new_mesh);
  }

  // теперь выделенными являются обновленные меши
  session_->selected_meshes = changed_meshes;
  viewport_->updateGL();
}

void MainWindow::slotUniteSelectedMeshes() {
  if (!session_) return;
  if (session_->selected_meshes.size() <= 1) return;

  slotBeforeNewModelCreating();

  // сливаем все выбранные меши в один (первым и последним слоями сливаем)
  while (session_->selected_meshes.size() > 1) {
    double min_dist = Double::max();
    QPair<Mesh::HardPtr, Mesh::HardPtr> targets;
    for (auto e1 : session_->selected_meshes) {
      for (auto e2 : session_->selected_meshes) {
        if (e1 == e2) continue;

        auto dist = e1->dist(*e2);
        if (dist < min_dist) {
          min_dist = dist;
          targets = qMakePair(e1, e2);
        }
      }
    }

    session_->meshes.removeOne(targets.first);
    session_->meshes.removeOne(targets.second);
    session_->selected_meshes.removeOne(targets.first);
    session_->selected_meshes.removeOne(targets.second);

    auto new_mesh = Mesh::unite(targets.first, targets.second);
    session_->selected_meshes.push_back(new_mesh);
    session_->addMesh(new_mesh);
  }

  viewport_->updateGL();
}

void MainWindow::slotInterruptCreatingProcess() {
  if (!session_) return;

  auto sender = qobject_cast<QPushButton*>(QObject::sender());
  bool checked = sender->isChecked();

  bool need_interrupt = (sender == tools_->create) && !checked;
  need_interrupt |= (sender != tools_->create) && checked;
  if (!checked) {
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
