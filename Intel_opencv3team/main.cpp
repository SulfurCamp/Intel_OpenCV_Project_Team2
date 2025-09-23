#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);
    QTextStream ts(stderr);
    switch (type) {
    case QtDebugMsg:
        ts << "Debug: " << msg << Qt::endl;
        break;
    case QtWarningMsg:
        ts << "Warning: " << msg << Qt::endl;
        break;
    case QtCriticalMsg:
        ts << "Critical: " << msg << Qt::endl;
        break;
    case QtFatalMsg:
        ts << "Fatal: " << msg << Qt::endl;
        abort();
    case QtInfoMsg:
        ts << "Info: " << msg << Qt::endl;
        break;
    }
}

int main(int argc, char *argv[]) {
    qInstallMessageHandler(customMessageHandler);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
