#include <QApplication>
#include <QTcpServer>
#include <QTcpSocket>

#ifdef Q_OS_WIN
#include "ScreenshotWidget.h"
#else
#include "ScreenshotWidgetmac.h"
#endif

#define PORT 50263

QTcpServer *server = NULL;
QList<QTcpSocket *> socketList;

ScreenshotWidget *shotWidget = NULL;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QObject::connect(&a, &QApplication::aboutToQuit, [](){
        if(shotWidget)
        {
            shotWidget->deleteLater();
            shotWidget = NULL;
        }

        if(server)
        {
            server->close();
            server->deleteLater();
            server = NULL;
        }

        while(socketList.count())
        {
            QTcpSocket *socket = socketList.takeFirst();
            socket->disconnectFromHost();
            socket->close();
            socket->deleteLater();
        }
    });

    server = new QTcpServer();
    QObject::connect(server, &QTcpServer::newConnection, [](){
        QTcpSocket * socket = server->nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead, [socket](){
            QByteArray buffer = socket->readAll();
            if(buffer == "start")
            {
                if(shotWidget)
                {
                    shotWidget->close();
                    shotWidget->deleteLater();
                    shotWidget = NULL;
                }

                shotWidget = new ScreenshotWidget();
#ifdef Q_OS_WIN
                shotWidget->getReadyToShot();
                shotWidget->show();
#endif
                shotWidget->raise();
                shotWidget->activateWindow();
            }
        });
        QObject::connect(socket, &QTcpSocket::disconnected, [socket](){
            socket->close();
            socket->deleteLater();
            socketList.removeAll(socket);
        });
        socketList.append(socket);
    });
    server->listen(QHostAddress::Any, PORT);

    return a.exec();
}
