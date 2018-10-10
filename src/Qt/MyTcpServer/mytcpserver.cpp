#include "mytcpserver.h"
#include "ui_mytcpserver.h"

MyTcpServer::MyTcpServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MyTcpServer)
{
    ui->setupUi(this);

    tcpServer = new QTcpServer(this);
    ui->edtIP->setText(QNetworkInterface().allAddresses().at(1).toString());   //获取本地IP
    ui->btnConnect->setEnabled(true);
    ui->btnSend->setEnabled(false);

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(NewConnectionSlot()));
}

MyTcpServer::~MyTcpServer()
{
    delete ui;
}
// newConnection -> newConnectionSlot 新连接建立的槽函数
void MyTcpServer::NewConnectionSlot()
{
    currentTcpClient = tcpServer->nextPendingConnection();
    tcpClientList.append(currentTcpClient);
    ui->cbxConnection->addItem(tr("%1:%2").arg(currentTcpClient->peerAddress().toString().split("::ffff:")[1])\
                                          .arg(currentTcpClient->peerPort()));
    connect(currentTcpClient, SIGNAL(readyRead()), this, SLOT(ReadData()));
    connect(currentTcpClient, SIGNAL(disconnected()), this, SLOT(disconnectedSlot()));
}

// 客户端数据可读信号，对应的读数据槽函数
void MyTcpServer::ReadData()
{
    // 由于readyRead信号并未提供SocketDecriptor，所以需要遍历所有客户端
    for(int i=0; i<tcpClientList.length(); i++)
    {
        QByteArray buffer = tcpClientList[i]->readAll();
        if(buffer.isEmpty())    continue;

        static QString IP_Port, IP_Port_Pre;
        IP_Port = tr("[%1:%2]:").arg(tcpClientList[i]->peerAddress().toString().split("::ffff:")[1])\
                                     .arg(tcpClientList[i]->peerPort());

        // 若此次消息的地址与上次不同，则需显示此次消息的客户端地址
        if(IP_Port != IP_Port_Pre)
            ui->edtRecv->append(IP_Port);

        ui->edtRecv->append(buffer);

        //更新ip_port
        IP_Port_Pre = IP_Port;
    }
}
// disconnected -> disconnectedSlot 客户端断开连接的槽函数
void MyTcpServer::disconnectedSlot()
{
    //由于disconnected信号并未提供SocketDescriptor，所以需要遍历寻找
    for(int i=0; i<tcpClientList.length(); i++)
    {
        if(tcpClientList[i]->state() == QAbstractSocket::UnconnectedState)
        {
            // 删除存储在combox中的客户端信息
            ui->cbxConnection->removeItem(ui->cbxConnection->findText(tr("%1:%2")\
                                  .arg(tcpClientList[i]->peerAddress().toString().split("::ffff:")[1])\
                                  .arg(tcpClientList[i]->peerPort())));
            // 删除存储在tcpClient列表中的客户端信息
             tcpClientList[i]->destroyed();
             tcpClientList.removeAt(i);
        }
    }
}
// 监听--断开
void MyTcpServer::on_btnConnect_clicked()
{
    if(ui->btnConnect->text()=="监听")
    {
        bool ok = tcpServer->listen(QHostAddress::Any, ui->edtPort->text().toInt());
        if(ok)
        {
            ui->btnConnect->setText("断开");
            ui->btnSend->setEnabled(true);
        }
    }
    else
    {
        for(int i=0; i<tcpClientList.length(); i++)//断开所有连接
        {
            tcpClientList[i]->disconnectFromHost();
            bool ok = tcpClientList[i]->waitForDisconnected(1000);
            if(!ok)
            {
                // 处理异常
            }
            tcpClientList.removeAt(i);  //从保存的客户端列表中取去除
        }
        tcpServer->close();     //不再监听端口
        ui->btnConnect->setText("监听");
        ui->btnSend->setEnabled(false);
    }
}
// 发送数据
void MyTcpServer::on_btnSend_clicked()
{
    QString data = ui->edtSend->toPlainText();
    if(data == "")  return;    // 文本输入框为空时
    //全部连接
    if(ui->cbxConnection->currentIndex() == 0)
    {
        for(int i=0; i<tcpClientList.length(); i++)
            tcpClientList[i]->write(data.toLatin1()); //qt5除去了.toAscii()
    }
    //指定连接
    else
    {
        QString clientIP = ui->cbxConnection->currentText().split(":")[0];
        int clientPort = ui->cbxConnection->currentText().split(":")[1].toInt();
//        qDebug() << clientIP;
//        qDebug() << clientPort;
        for(int i=0; i<tcpClientList.length(); i++)
        {
            if(tcpClientList[i]->peerAddress().toString().split("::ffff:")[1]==clientIP\
                            && tcpClientList[i]->peerPort()==clientPort)
            {
                tcpClientList[i]->write(data.toLatin1());
                return; //ip:port唯一，无需继续检索
            }
        }
    }
}

void MyTcpServer::on_btnClear_clicked()
{
    ui->edtRecv->clear();
}

