#include "mytcpclient.h"
#include "ui_mytcpclient.h"

MyTcpClient::MyTcpClient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MyTcpClient)
{
    ui->setupUi(this);

    //初始化TCP客户端
    tcpClient = new QTcpSocket(this);   //实例化tcpClient
    tcpClient->abort();                 //取消原有连接
    ui->btnConnect->setEnabled(true);
    ui->btnSend->setEnabled(false);

    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(ReadData()));
    connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), \
            this, SLOT(ReadError(QAbstractSocket::SocketError)));
}

MyTcpClient::~MyTcpClient()
{
    delete ui;
}

void MyTcpClient::ReadData()
{
    QByteArray buffer = tcpClient->readAll();
    if(!buffer.isEmpty())
    {
        ui->edtRecv->append(buffer);
    }
}

void MyTcpClient::ReadError(QAbstractSocket::SocketError)
{
    tcpClient->disconnectFromHost();
    ui->btnConnect->setText(tr("连接"));
    QMessageBox msgBox;
    msgBox.setText(tr("failed to connect server because %1").arg(tcpClient->errorString()));
    msgBox.exec();
}

void MyTcpClient::on_btnConnect_clicked()
{
    if(ui->btnConnect->text()=="连接")
    {
        tcpClient->connectToHost(ui->edtIP->text(), ui->edtPort->text().toInt());
        if (tcpClient->waitForConnected(1000))  // 连接成功则进入if{}
        {
            ui->btnConnect->setText("断开");
            ui->btnSend->setEnabled(true);
        }
    }
    else
    {
        tcpClient->disconnectFromHost();
        if (tcpClient->state() == QAbstractSocket::UnconnectedState \
                || tcpClient->waitForDisconnected(1000))  //已断开连接则进入if{}
        {
            ui->btnConnect->setText("连接");
            ui->btnSend->setEnabled(false);
        }
    }
}

void MyTcpClient::on_btnSend_clicked()
{
    QString data = ui->edtSend->toPlainText();
    if(data != "")
    {
        tcpClient->write(data.toLatin1()); //qt5出去了.toAscii()
    }
}

void MyTcpClient::on_pushButton_clicked()
{
    ui->edtRecv->clear();
}
