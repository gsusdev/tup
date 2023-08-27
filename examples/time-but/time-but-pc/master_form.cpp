#include "master_form.h"
#include "ui_master_form.h"

#include <QDebug>
#include <QTime>
#include <QtEndian>

#include "port_widget.h"

MasterForm::MasterForm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MasterForm)
{
    ui->setupUi(this);

    auto portWidget_p = new PortWidget(ui->gbPort);
    portWidget_p->setPort(&_port);

    connect(portWidget_p, &PortWidget::sigOpened, this, &MasterForm::slotPortOpened);
    connect(portWidget_p, &PortWidget::sigClosed, this, &MasterForm::slotPortClosed);

    _tup.setIsMaster(true);
    _tup.setPort(&_port);

    _master.setTup(&_tup);

    connect(ui->butConnect, &QPushButton::clicked, this, &MasterForm::slotButConnectClicked);
    connect(ui->butSend, &QPushButton::clicked, this, &MasterForm::slotButSendClicked);
    connect(ui->butClear, &QPushButton::clicked, this, &MasterForm::slotButClearClicked);
    connect(ui->chkAutoSend, &QCheckBox::clicked, this, &MasterForm::slotChkAutoClicked);

    connect(&_tup, &TupWrapper::onConnect, this, &MasterForm::slotOnConnect);
    connect(&_tup, &TupWrapper::onDisconnectRequest, this, &MasterForm::slotOnDisconnectRequest);
    connect(&_tup, &TupWrapper::onSendDataProgress, this, &MasterForm::slotOnSendDataProgress);
    connect(&_tup, &TupWrapper::onResultSent, this, &MasterForm::slotOnResultSent);
    connect(&_tup, &TupWrapper::onReceiveData, this, &MasterForm::slotOnReceiveData);
    connect(&_tup, &TupWrapper::onFail, this, &MasterForm::slotOnFail);

    connect(&_master, &MasterHandler::sigOnUpdated, this, &MasterForm::slotOnReceivedFromSlave);

    connect(&_timer, &QTimer::timeout, this, &MasterForm::slotTimerTimeout);
}

MasterForm::~MasterForm()
{
    delete ui;
}

void MasterForm::slotPortOpened()
{
    _tup.setName(_port.portName());
    _tup.run();

    ui->butConnect->setEnabled(true);
}

void MasterForm::slotPortClosed()
{
    ui->butConnect->setEnabled(false);
    ui->butSend->setEnabled(false);
    ui->chkAutoSend->setEnabled(false);
    ui->chkAutoSend->setChecked(false);

    _tup.stop();
}

void MasterForm::slotButConnectClicked(bool checked)
{
    (void)checked;

    if (!_tup.tupConnect())
    {
        qDebug() << "Master failed to connect";
    }
}

void MasterForm::slotButSendClicked(bool checked)
{
    (void)checked;

    auto& masterOutput = _master.masterOutput();    

    if (ui->rbTimeSystem->isChecked())
    {
        const auto time = QTime::currentTime();

        masterOutput.hour = time.hour();
        masterOutput.minute = time.minute();
        masterOutput.second = time.second();
    }
    else if (ui->rbTimeManual->isChecked())
    {
        masterOutput.hour = ui->sbOutHour->value();
        masterOutput.minute = ui->sbOutMinute->value();
        masterOutput.second = ui->sbOutSecond->value();
    }
    else
    {
        masterOutput.hour = 24;
        masterOutput.minute = 60;
        masterOutput.second = 60;
    }

    _master.sendData();
}

void MasterForm::slotChkAutoClicked(bool checked)
{
    if (checked)
    {
        _timer.setSingleShot(false);
        _timer.setInterval(ui->sbAutoSendInterval->value());
        _timer.start();
    }
    else
    {
        _timer.stop();
    }
}

void MasterForm::slotButClearClicked(bool checked)
{
    (void)checked;

    ui->plainTextEdit->clear();
}

void MasterForm::slotOnConnect()
{
    ui->butSend->setEnabled(true);
    ui->chkAutoSend->setEnabled(true);

    ui->plainTextEdit->appendPlainText("Connected");
}

void MasterForm::slotOnDisconnectRequest()
{
    ui->plainTextEdit->appendPlainText("FIN received");
}

void MasterForm::slotOnSendDataProgress(quintptr sentSize_bytes, quintptr totalSize_bytes)
{
    ui->plainTextEdit->appendPlainText(QString("Sent %1 of %2 bytes").arg(sentSize_bytes).arg(totalSize_bytes));
}

void MasterForm::slotOnResultSent()
{
    ui->plainTextEdit->appendPlainText("ACK sent");
}

void MasterForm::slotOnReceiveData(QByteArray data, quint8 isFinal)
{
    QString final = isFinal ? "final" : "";
    ui->plainTextEdit->appendPlainText(QString("Received %1 %2 bytes").arg(data.size()).arg(final));
}

void MasterForm::slotOnFail(quint32 failCode)
{
    ui->plainTextEdit->appendPlainText(QString("Fail %1").arg(failCode));
}

void MasterForm::slotOnReceivedFromSlave()
{
    const auto& slaveOutput = _master.slaveOutput();

    ui->chkIsBigEndian->setChecked(slaveOutput.isBigEndian);
    ui->chkIsButDown->setChecked(slaveOutput.isButtonDown);
    ui->sbInHour->setValue(slaveOutput.hour);
    ui->sbInMinute->setValue(slaveOutput.minute);
    ui->sbInSecond->setValue(slaveOutput.second);
    ui->sbClickCount->setValue(slaveOutput.butClickCount);

    if (slaveOutput.mcuIdSize == sizeof(quint32))
    {
        quint32 val = *reinterpret_cast<const quint32*>(slaveOutput.mcuId);
        if (slaveOutput.isBigEndian)
        {
            val = qFromBigEndian<quint32>(val);
        }
        else
        {
            val = qFromLittleEndian<quint32>(val);
        }

        ui->leMcuId->setText(QString::number(val, 10));
    }
    else
    {
        QByteArray ba(reinterpret_cast<const char*>(slaveOutput.mcuId), slaveOutput.mcuIdSize);
        const auto s = ba.toHex(' ');
        ui->leMcuId->setText(s);
    }
}

void MasterForm::slotTimerTimeout()
{
    ui->butSend->click();
}
