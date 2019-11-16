#include "mainwindow.h"
#include <QApplication>
#define STB_IMAGE_IMPLEMENTATION

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
