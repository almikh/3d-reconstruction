#include <model-settings-widget.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QLabel>
#include <QDebug>

ModelSettingsWidget::ModelSettingsWidget(QWidget* parent) :
QMainWindow(parent)
{
	setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
	setWindowTitle("Settings");

	auto layout = new QVBoxLayout();
	auto widget = new QWidget();

	auto step_layout = new QHBoxLayout();
	auto step_label = new QLabel("Step:", this);
	step_label->setFont(QFont("Arial", 13));
	step_layout->addWidget(step_label);
	step = new QSlider(Qt::Horizontal, this);
	step->setFixedHeight(24);
	step->setMaximum(15);
	step->setValue(5);
	step->setMinimum(1);
	step_layout->addWidget(step);
	layout->addLayout(step_layout);

	auto slices_layout = new QHBoxLayout();
	auto slices_label = new QLabel("Slices:", this);
	slices_label->setFont(QFont("Arial", 13));
	slices_layout->addWidget(slices_label);
	slices = new QSlider(Qt::Horizontal, this);
	slices->setFixedHeight(24);
	slices->setMaximum(360);
	slices->setValue(45);
	slices->setMinimum(4);
	slices_layout->addWidget(slices);
	layout->addLayout(slices_layout);

	widget->setLayout(layout);
	setCentralWidget(widget);

	resize(250, 80);
}

ModelSettingsWidget::~ModelSettingsWidget() {

}

void ModelSettingsWidget::hideEvent(QHideEvent*) {
	emit signalHide();
}
