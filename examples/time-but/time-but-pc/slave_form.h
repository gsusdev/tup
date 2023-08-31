#ifndef SLAVE_FORM_H
#define SLAVE_FORM_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>

#include "tup_wrapper.h"
#include "slave_handler.h"

namespace Ui {
class SlaveForm;
}

class SlaveForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit SlaveForm(QWidget *parent = nullptr);
    ~SlaveForm();

private slots:
    void slotPortOpened();
    void slotPortClosed();

    void slotButAcceptClicked(bool checked);
    void slotButBlueButtonClicked(bool checked);
    void slotButClearClicked(bool checked);

    void slotOnConnect();
    void slotOnDisconnectRequest();
    void slotOnSendDataProgress(quintptr sentSize_bytes, quintptr totalSize_bytes);
    void slotOnResultSent();
    void slotOnReceiveData(QByteArray data, quint8 isFinal);
    void slotOnFail(quint32 failCode);

    void slotOnReceivedFromMaster();

    void slotTimerTimeout();

private:
    Ui::SlaveForm *ui;

    QSerialPort _port;
    TupWrapper _tup;
    SlaveHandler _slave;

    QTimer _timer;
};

#endif // SLAVE_FORM_H
