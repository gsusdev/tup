#include "mainwindow.h"

#include <QApplication>

static MainWindow* logWindow_p = nullptr;

void logFunc(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (logWindow_p == nullptr)
    {
        return;
    }

    QString severity;
    switch (type)
    {
    case QtDebugMsg:
        severity = "Debug";
        break;
    case QtInfoMsg:
        severity = "Info";
        break;
    case QtWarningMsg:
        severity = "Warning";
        break;
    case QtCriticalMsg:
        severity = "Critical";
        break;
    case QtFatalMsg:
        severity = "Fatal";
        abort();
    }

    (void)context;
    logWindow_p->logText(severity + ": " + msg);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    logWindow_p = &w;
    qInstallMessageHandler(logFunc);

    return a.exec();
}
