#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <cstdint>
#include <cstdbool>

#include <QMainWindow>
#include <QSerialPort>
#include <QComboBox>

#include "tup_sender.h"
#include "tup_receiver.h"

#include "form_syn.h"
#include "form_fin.h"
#include "form_ack.h"
#include "form_data.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void butRefreshClicked(bool checked);
    void butOpenCloseClicked(bool checked);

    void butSendSynClicked(bool checked);
    void butSendFinClicked(bool checked);
    void butSendAckClicked(bool checked);
    void butSendDataClicked(bool checked);

private:
    void initPortList(QComboBox& comboBox);
    void initBaudratesList(QComboBox& comboBox);

    Ui::MainWindow *ui;

    SynForm* _sendSynForm_p = nullptr;
    FinForm* _sendFinForm_p = nullptr;
    AckForm* _sendAckForm_p = nullptr;
    DataForm* _sendDataForm_p = nullptr;

    SynForm* _recvSynForm_p = nullptr;
    FinForm* _recvFinForm_p = nullptr;
    AckForm* _recvAckForm_p = nullptr;
    DataForm* _recvDataForm_p = nullptr;

    QByteArray _dataPayloadBuf;

    QSerialPort _port;

    TupSender _sender;
    TupReceiver _receiver;
};
#endif // MAINWINDOW_H
