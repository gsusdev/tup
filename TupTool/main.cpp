#include "mainwindow.h"

#include "tup_port.h"

#include <QApplication>
#include <QMetaObject>

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

static MainWindow* mainWin_p = nullptr;

static void tupLogHandler(const char* text)
{
    QString str(text);
    QMetaObject::invokeMethod(mainWin_p, "logTextWoReturn", Q_ARG(QString, str));
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

    mainWin_p = &w;

    logWindow_p = &w;
    qInstallMessageHandler(logFunc);

    tup_port_setLogHandler(tupLogHandler);

    return a.exec();
}
