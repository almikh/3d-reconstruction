#include <mainwindow.h>
#include <QApplication>
#include <image.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

	//ip::Image<double> image(QImage("3.png"));
	//image = image.gvf(0.07, 40, [](double v1, double v2) -> double {
	//	return sqrt(v1*v1 + v2*v2);
	//}).scale(0, 255);
	//image.save("test.png");

    return a.exec();
}
