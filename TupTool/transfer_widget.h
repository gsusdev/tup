#ifndef TRANSFER_WIDGET_H
#define TRANSFER_WIDGET_H

#include <QWidget>
#include <QIODevice>

#include "tup_v1_types.h"
#include "tup_transfer.h"

namespace Ui {
class TransferWidget;
}

class TransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransferWidget(QWidget *parent = nullptr);
    ~TransferWidget();

    void setName(const QString& value) { _transfer.setName(value); }
    void setPort(QIODevice* port_p);

private slots:
    void butSendSynClicked(bool checked);
    void butSendFinClicked(bool checked);
    void butSetResultClicked(bool checked);
    void butSendDataClicked(bool checked);
    void butClearInputClicked(bool checked);

    void onCompleted(int resultCode);
    void onSyn(quint32 j, quintptr windowSize);
    void onFin();
    void onData(QByteArray payload, bool isFinal);
    void onFail(quint32 failCode);
    void onBadFrame(quint32 errorCode);

private:
    Ui::TransferWidget *ui;

    TupTransfer _transfer;
};

#endif // TRANSFER_WIDGET_H
