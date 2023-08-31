#ifndef MASTER_FORM_H
#define MASTER_FORM_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>

#include "tup_wrapper.h"
#include "master_handler.h"

namespace Ui {
class MasterForm;
}

class MasterForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit MasterForm(QWidget *parent = nullptr);
    ~MasterForm();

private slots:
    void slotPortOpened();
    void slotPortClosed();

    void slotButConnectClicked(bool checked);
    void slotButSendClicked(bool checked);
    void slotChkAutoClicked(bool checked);
    void slotButClearClicked(bool checked);

    void slotOnConnect();
    void slotOnDisconnectRequest();
    void slotOnSendDataProgress(quintptr sentSize_bytes, quintptr totalSize_bytes);
    void slotOnRetryProgress(quint32 attemptNumber, quint32 maxAttemptCount, quint32 remainingTime_ms);
    void slotOnResultSent();
    void slotOnReceiveData(QByteArray data, quint8 isFinal);
    void slotOnFail(quint32 failCode);

    void slotOnReceivedFromSlave();

    void slotTimerTimeout();

private:
    Ui::MasterForm *ui;

    QSerialPort _port;
    TupWrapper _tup;
    MasterHandler _master;

    QTimer _timer;
};

#endif // MASTER_FORM_H
