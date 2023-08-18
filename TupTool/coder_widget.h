#ifndef CODER_WIDGET_H
#define CODER_WIDGET_H

#include <QWidget>
#include <QByteArray>
#include <QIODevice>

#include "tup_sender.h"
#include "tup_receiver.h"

#include "syn_widget.h"
#include "fin_widget.h"
#include "ack_widget.h"
#include "data_widget.h"

namespace Ui {
class CoderWidget;
}

class CoderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CoderWidget(QWidget *parent = nullptr);
    ~CoderWidget();

    void setPort(QIODevice* port_p);

private slots:
    void butSendSynClicked(bool checked);
    void butSendFinClicked(bool checked);
    void butSendAckClicked(bool checked);
    void butSendDataClicked(bool checked);

private:
    Ui::CoderWidget *ui;

    SynWidget* _sendSynForm_p = nullptr;
    FinWidget* _sendFinForm_p = nullptr;
    AckWidget* _sendAckForm_p = nullptr;
    DataWidget* _sendDataForm_p = nullptr;

    SynWidget* _recvSynForm_p = nullptr;
    FinWidget* _recvFinForm_p = nullptr;
    AckWidget* _recvAckForm_p = nullptr;
    DataWidget* _recvDataForm_p = nullptr;

    QByteArray _dataPayloadBuf;

    TupSender _sender;
    TupReceiver _receiver;
};

#endif // CODER_WIDGET_H
