#include "port_widget.h"
#include "ui_port_widget.h"

#include <QSerialPortInfo>
#include <QMessageBox>

PortWidget::PortWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PortWidget)
{
    ui->setupUi(this);

    connect(ui->butOpenClose, &QPushButton::clicked, this, &PortWidget::butOpenCloseClicked);

    initPortList(*ui->cbPort);
    initBaudratesList(*ui->cbBaudrate);
}

PortWidget::~PortWidget()
{
    delete ui;
}

void PortWidget::butOpenCloseClicked(bool checked)
{
    (void)checked;

    if (_port_p == nullptr)
    {
        return;
    }

    if (_port_p->isOpen())
    {
        _port_p->close();
        emit sigClosed();
        ui->butOpenClose->setText("Open");
    }
    else
    {
        const auto& portName = ui->cbPort->currentText();

        _port_p->setPortName(portName);
        _port_p->setParity(QSerialPort::Parity::NoParity);
        _port_p->setDataBits(QSerialPort::DataBits::Data8);
        _port_p->setStopBits(QSerialPort::StopBits::OneStop);
        _port_p->setBaudRate(ui->cbBaudrate->currentData().toInt());

        if (_port_p->open(QSerialPort::ReadWrite))
        {
            emit sigOpened();
            ui->butOpenClose->setText("Close");
        }
        else
        {
            const auto errText = _port_p->errorString();
            QMessageBox::critical(this, "Open port", errText);
        }
    }
}

void PortWidget::initPortList(QComboBox& comboBox)
{
    const auto oldPort = comboBox.currentText();
    comboBox.clear();

    auto oldPortPresent = false;

    for (const auto& port : QSerialPortInfo::availablePorts())
    {
        if (port.isNull())
        {
            continue;
        }

        const auto name = port.portName();
        comboBox.addItem(name);

        if (name == oldPort)
        {
            oldPortPresent = true;
        }
    }

    if (oldPortPresent)
    {
        comboBox.setCurrentText(oldPort);
    }
}

void PortWidget::initBaudratesList(QComboBox& comboBox)
{
    comboBox.clear();

    for (const auto br : QSerialPortInfo::standardBaudRates())
    {
        comboBox.addItem(QString::number(br), br);
    }

    comboBox.setCurrentIndex(0);
}
