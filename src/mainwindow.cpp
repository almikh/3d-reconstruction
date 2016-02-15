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

/* MainWindow */
MainWindow::MainWindow(QWidget* parent) :
QMainWindow(parent),
viewport_(new rn::Viewport(this)),
settings_widget_(new ModelSettingsWidget(this))
{
	setWindowTitle("3d-reconstruction");
	setCentralWidget(viewport_);

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
	auto toolbar = addToolBar("Main toolbar");
	toolbar->setIconSize(QSize(32, 32));
	
	toolbar_.open = toolbar->addAction(QIcon("icons/folder.png"), "Open image");
	connect(toolbar_.open, SIGNAL(triggered()), SLOT(slotOpenImage()));

	toolbar->addSeparator();

	toolbar_.enable_first_layer = toolbar->addAction(QIcon("icons/show-first-layer.png"), "Triangulate first layer");
	toolbar_.enable_first_layer->setCheckable(true);
	connect(toolbar_.enable_first_layer, &QAction::triggered, [=](bool enable) {
		auto creator = viewport_->model_creator;
		creator->triangulate_first_layer = enable;
	});

	toolbar_.enable_last_layer = toolbar->addAction(QIcon("icons/show-last-layer.png"), "Triangulate last layer");
	toolbar_.enable_last_layer->setCheckable(true);
	connect(toolbar_.enable_last_layer, &QAction::triggered, [=](bool enable) {
		auto creator = viewport_->model_creator;
		creator->triangulate_last_layer = enable;
	});

	toolbar_.unite_meshes = toolbar->addAction(QIcon("icons/unite.png"), "Unite nearest meshes");
	toolbar_.unite_meshes->setCheckable(true);
	connect(toolbar_.unite_meshes, &QAction::triggered, [=](bool enable) {
		auto creator = viewport_->model_creator;
		creator->merge_models = enable;
	});

	toolbar_.texturing = toolbar->addAction(QIcon("icons/tex.png"), "Use texturing");
	toolbar_.texturing->setCheckable(true);
	connect(toolbar_.texturing, &QAction::triggered, [=](bool enable) {
		auto creator = viewport_->model_creator;
		creator->using_texturing = enable;
	});
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
	viewport_->model_creator->setSessionData(session_);

	viewport_->model_creator->merge_models = toolbar_.unite_meshes->isChecked();
	viewport_->model_creator->using_texturing = toolbar_.texturing->isChecked();
	viewport_->model_creator->triangulate_last_layer = toolbar_.enable_last_layer->isChecked();
	viewport_->model_creator->triangulate_first_layer = toolbar_.enable_first_layer->isChecked();

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
