#include "slave_form.h"
#include "ui_slave_form.h"

#include <cstddef>
#include <cstdint>

#include <QDebug>

#include "port_widget.h"

SlaveForm::SlaveForm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SlaveForm)
{
    ui->setupUi(this);

    auto portWidget_p = new PortWidget(ui->gbPort);
    portWidget_p->setPort(&_port);

    connect(portWidget_p, &PortWidget::sigOpened, this, &SlaveForm::slotPortOpened);
    connect(portWidget_p, &PortWidget::sigClosed, this, &SlaveForm::slotPortClosed);

    _tup.setIsMaster(false);
    _tup.setPort(&_port);

    _slave.setTup(&_tup);

    connect(ui->butAccept, &QPushButton::clicked, this, &SlaveForm::slotButAcceptClicked);
    connect(ui->butClear, &QPushButton::clicked, this, &SlaveForm::slotButClearClicked);
    connect(ui->butBlueButton, &QPushButton::clicked, this, &SlaveForm::slotButBlueButtonClicked);

    connect(&_tup, &TupWrapper::onConnect, this, &SlaveForm::slotOnConnect);
    connect(&_tup, &TupWrapper::onDisconnectRequest, this, &SlaveForm::slotOnDisconnectRequest);
    connect(&_tup, &TupWrapper::onSendDataProgress, this, &SlaveForm::slotOnSendDataProgress);
    connect(&_tup, &TupWrapper::onResultSent, this, &SlaveForm::slotOnResultSent);
    connect(&_tup, &TupWrapper::onReceiveData, this, &SlaveForm::slotOnReceiveData);
    connect(&_tup, &TupWrapper::onFail, this, &SlaveForm::slotOnFail);

    connect(&_slave, &SlaveHandler::sigOnUpdated, this, &SlaveForm::slotOnReceivedFromMaster);

    connect(&_timer, &QTimer::timeout, this, &SlaveForm::slotTimerTimeout);

    _timer.setInterval(1000);
    _timer.start();
}

SlaveForm::~SlaveForm()
{
    _timer.stop();
    delete ui;
}

void SlaveForm::slotPortOpened()
{
    _tup.setName(_port.portName());
    _tup.run();

    ui->butAccept->setEnabled(true);
}

void SlaveForm::slotPortClosed()
{
    ui->butAccept->setEnabled(false);

    _tup.stop();
}

void SlaveForm::slotButAcceptClicked(bool checked)
{
    (void)checked;

    if (!_tup.tupAccept())
    {
        qDebug() << "Slave failed to accept";
    }
    else
    {
        ui->butAccept->setEnabled(false);
    }
}

void SlaveForm::slotButBlueButtonClicked(bool checked)
{
    if (checked)
    {
        ui->sbClickCount->setValue(ui->sbClickCount->value() + 1);
    }
}

void SlaveForm::slotButClearClicked(bool checked)
{
    (void)checked;

    ui->plainTextEdit->clear();
}

void SlaveForm::slotOnConnect()
{
    ui->plainTextEdit->appendPlainText("Connected");
}

void SlaveForm::slotOnDisconnectRequest()
{
    ui->plainTextEdit->appendPlainText("FIN received");
}

void SlaveForm::slotOnSendDataProgress(quintptr sentSize_bytes, quintptr totalSize_bytes)
{
    ui->plainTextEdit->appendPlainText(QString("Sent %1 of %2 bytes").arg(sentSize_bytes).arg(totalSize_bytes));
}

void SlaveForm::slotOnResultSent()
{
    ui->plainTextEdit->appendPlainText("ACK sent");
}

void SlaveForm::slotOnReceiveData(QByteArray data, quint8 isFinal)
{
    QString final = isFinal ? "final" : "";
    ui->plainTextEdit->appendPlainText(QString("Received %1 %2 bytes").arg(data.size()).arg(final));
}

void SlaveForm::slotOnFail(quint32 failCode)
{
    ui->plainTextEdit->appendPlainText(QString("Fail %1").arg(failCode));
    ui->butAccept->setEnabled(true);
}

void SlaveForm::slotOnReceivedFromMaster()
{
    const auto& masterOutput = _slave.masterOutput();

    ui->sbInHour->setValue(masterOutput.hour);
    ui->sbInMinute->setValue(masterOutput.minute);
    ui->sbInSecond->setValue(masterOutput.second);

    auto isValid = true;

    isValid &= masterOutput.hour < 24;
    isValid &= masterOutput.minute < 60;
    isValid &= masterOutput.second < 60;

    if (ui->chkCopyToSend->isChecked() || (ui->chkSetIfValid->isChecked() && isValid))
    {
        ui->sbOutHour->setValue(masterOutput.hour);
        ui->sbOutMinute->setValue(masterOutput.minute);
        ui->sbOutSecond->setValue(masterOutput.second);
    }

    auto& slaveOutput = _slave.slaveOutput();

    slaveOutput.hour = ui->sbOutHour->value();
    slaveOutput.minute = ui->sbOutMinute->value();
    slaveOutput.second = ui->sbOutSecond->value();
    slaveOutput.isBigEndian = ui->chkIsBigEndian->isChecked();
    slaveOutput.isButtonDown = ui->butBlueButton->isChecked();
    slaveOutput.butClickCount = ui->sbClickCount->value();

    bool ok;
    uint32_t mcuId = ui->leMcuId->text().toUInt(&ok, 0);
    if (!ok)
    {
        mcuId = 0;
    }

    slaveOutput.mcuIdSize = sizeof(mcuId);

    *reinterpret_cast<uint32_t*>(slaveOutput.mcuId) = mcuId;
}

void SlaveForm::slotTimerTimeout()
{
    if (!ui->chkTimeGo->isChecked())
    {
        return;
    }

    uint8_t hour = ui->sbOutHour->value();
    uint8_t minute = ui->sbOutMinute->value();
    uint8_t second = ui->sbOutSecond->value();

    ++second;

    if (second == 60)
    {
        second = 0;
        ++minute;
    }

    if (minute == 60)
    {
        minute = 0;
        ++hour;
    }

    if (hour == 24)
    {
        hour = 0;
    }

    ui->sbOutHour->setValue(hour);
    ui->sbOutMinute->setValue(minute);
    ui->sbOutSecond->setValue(second);
}
