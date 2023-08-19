#include "instance_form.h"
#include "ui_instance_form.h"

#include <QSerialPortInfo>
#include <QMessageBox>
#include <QTabWidget>

InstanceForm::InstanceForm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InstanceForm)
{
    ui->setupUi(this);

    initPortList(*ui->cbPortName);
    initBaudratesList(*ui->cbBaudrate);

    connect(ui->butRefresh, &QPushButton::clicked, this, &InstanceForm::butRefreshClicked);
    connect(ui->butOpenClose, &QPushButton::clicked, this, &InstanceForm::butOpenCloseClicked);

    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &InstanceForm::tabWidgetCurrentChanged);

    _coder_p = new CoderWidget(ui->tabCoder);
    _transfer_p = new TransferWidget(ui->tabTransfer);

    _coder_p->setPort(&_port);
}

InstanceForm::~InstanceForm()
{
    delete ui;
}

void InstanceForm::butRefreshClicked(bool checked)
{
    (void)checked;

    initPortList(*ui->cbPortName);
}

void InstanceForm::butOpenCloseClicked(bool checked)
{
    (void)checked;

    if (_port.isOpen())
    {
        _port.close();
        ui->butOpenClose->setText("Open");
    }
    else
    {
        _port.setPortName(ui->cbPortName->currentText());
        _port.setParity(QSerialPort::Parity::NoParity);
        _port.setDataBits(QSerialPort::DataBits::Data8);
        _port.setStopBits(QSerialPort::StopBits::OneStop);
        _port.setBaudRate(ui->cbBaudrate->currentData().toInt());

        if (_port.open(QSerialPort::ReadWrite))
        {
            ui->butOpenClose->setText("Close");
        }
        else
        {
            const auto errText = _port.errorString();// QMetaEnum::fromType<QSerialPort::SerialPortError>().valueToKey(_port.error());
            QMessageBox::critical(this, "Open port", errText);
        }
    }
}

void InstanceForm::tabWidgetCurrentChanged(int index)
{
    auto widget_p = ui->tabWidget->widget(index);

    if (widget_p == ui->tabCoder)
    {
        _transfer_p->setPort(nullptr);
        _coder_p->setPort(&_port);
    }
    else if (widget_p == ui->tabTransfer)
    {
        _coder_p->setPort(nullptr);
        _transfer_p->setName(ui->cbPortName->currentText());
        _transfer_p->setPort(&_port);
    }
}

void InstanceForm::initPortList(QComboBox& comboBox)
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

void InstanceForm::initBaudratesList(QComboBox& comboBox)
{
    comboBox.clear();

    for (const auto br : QSerialPortInfo::standardBaudRates())
    {
        comboBox.addItem(QString::number(br), br);
    }

    comboBox.setCurrentIndex(0);
}
