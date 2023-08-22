#ifndef INSTANCE_WIDGET_H
#define INSTANCE_WIDGET_H

#include <QWidget>

#include "tup_instance_w.h"

namespace Ui {
class InstanceWidget;
}

class InstanceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InstanceWidget(QWidget *parent = nullptr);
    ~InstanceWidget();

    void setPort(QIODevice* port_p) { _instance.setPort(port_p); }
    void setName(const QString& value) { _instance.setName(value); }

private slots:
    void butRunClicked(bool checked);
    void butStopClicked(bool checked);
    void butConnectClicked(bool checked);
    void butAcceptClicked(bool checked);
    void butSendFinClicked(bool checked);
    void butSendDataClicked(bool checked);
    void butSetResultClicked(bool checked);
    void butClearClicked(bool checked);

    void onConnect();
    void onDisconnectRequest();
    void onSendDataProgress(quintptr sentSize_bytes, quintptr totalSize_bytes);
    void onReceiveData(QByteArray data, quint8 isFinal);
    void onFail(quint32 failCode);

private:
    Ui::InstanceWidget *ui;

    TupInstance _instance;
};

#endif // INSTANCE_WIDGET_H
