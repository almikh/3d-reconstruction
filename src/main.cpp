#include <mainwindow.h>
#include <QApplication>
#include <image.h>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  w.setRightPosForTools(); // нужно вызывато после show()

  return a.exec();
}
