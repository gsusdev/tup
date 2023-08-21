#include "instance_widget.h"
#include "ui_instance_widget.h"

#include <QDebug>
#include <QByteArray>

#include "utils.h"

InstanceWidget::InstanceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InstanceWidget)
{
    ui->setupUi(this);
    initErrorList(*ui->cbError);

    connect(ui->butRun, &QPushButton::clicked, this, &InstanceWidget::butRunClicked);
    connect(ui->butStop, &QPushButton::clicked, this, &InstanceWidget::butStopClicked);
    connect(ui->butConnect, &QPushButton::clicked, this, &InstanceWidget::butConnectClicked);
    connect(ui->butAccept, &QPushButton::clicked, this, &InstanceWidget::butAcceptClicked);
    connect(ui->butSendFin, &QPushButton::clicked, this, &InstanceWidget::butSendFinClicked);
    connect(ui->butSendData, &QPushButton::clicked, this, &InstanceWidget::butSendDataClicked);
    connect(ui->butClear, &QPushButton::clicked, this, &InstanceWidget::butClearClicked);

    connect(&_instance, &TupInstance::onConnect, this, &InstanceWidget::onConnect);
    connect(&_instance, &TupInstance::onDisconnectRequest, this, &InstanceWidget::onDisconnectRequest);
    connect(&_instance, &TupInstance::onSendDataProgress, this, &InstanceWidget::onSendDataProgress);
    connect(&_instance, &TupInstance::onReceiveData, this, &InstanceWidget::onReceiveData);
    connect(&_instance, &TupInstance::onFail, this, &InstanceWidget::onFail);
}

InstanceWidget::~InstanceWidget()
{
    delete ui;
}

void InstanceWidget::butRunClicked(bool checked)
{
    (void)checked;

    _instance.setIsMaster(ui->rbMaster->isChecked());

    if (!_instance.run())
    {
        qDebug() << "Run failed";
    }
}

void InstanceWidget::butStopClicked(bool checked)
{
    (void)checked;

    _instance.stop();
}

void InstanceWidget::butConnectClicked(bool checked)
{
    (void)checked;

    if (!_instance.tupConnect())
    {
        qDebug() << "Connect failed";
    }
}

void InstanceWidget::butAcceptClicked(bool checked)
{
    (void)checked;

    if (!_instance.tupAccept())
    {
        qDebug() << "Accept failed";
    }
}

void InstanceWidget::butSendFinClicked(bool checked)
{
    (void)checked;

    if (!_instance.sendFin())
    {
        qDebug() << "Send FIN failed";
    }
}

void InstanceWidget::butSendDataClicked(bool checked)
{
    (void)checked;

    QByteArray data(ui->pteDataToSend->toPlainText().toUtf8());

    if (!_instance.sendData(data))
    {
        qDebug() << "Send DATA failed";
    }
}

void InstanceWidget::butSetResultClicked(bool checked)
{
    const auto errorCode = ui->cbError->currentData().toUInt();
    if (!_instance.setResult(static_cast<tup_transfer_result_t>(errorCode)))
    {
        qDebug() << "Set result failed";
    }
}

void InstanceWidget::butClearClicked(bool checked)
{
    (void)checked;
    ui->teLog->clear();
}

void InstanceWidget::onConnect()
{
    ui->teLog->append("Connected");
}

void InstanceWidget::onDisconnectRequest()
{
    ui->teLog->append("FIN received");
}

void InstanceWidget::onSendDataProgress(quintptr sentSize_bytes, quintptr totalSize_bytes)
{
    ui->teLog->append(QString("Sent %1 of %2 bytes").arg(sentSize_bytes).arg(totalSize_bytes));
}

void InstanceWidget::onReceiveData(QByteArray data, quint8 isFinal)
{
    const auto text = QString::fromUtf8(data);
    const QString final = isFinal ? "(fin)" : "";

    ui->teLog->append(QString("Received %1: %2 bytes").arg(final).arg(text));
}

void InstanceWidget::onFail(quint32 failCode)
{
    ui->teLog->append(QString("FAIL %1").arg(failCode));
}
