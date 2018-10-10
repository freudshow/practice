#include "mytcpserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyTcpServer w;
    w.show();

    return a.exec();
}
