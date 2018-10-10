#include "mytcpclient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MyTcpClient w;
    w.show();

    return a.exec();
}
