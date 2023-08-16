#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSerialPortInfo>
#include <QMessageBox>
#include <QMetaEnum>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{    
    qRegisterMetaType<PAckFrame>("PAckFrame");
    qRegisterMetaType<PSynFrame>("PSynFrame");
    qRegisterMetaType<PFinFrame>("PFinFrame");
    qRegisterMetaType<PDataFrame>("PDataFrame");

    ui->setupUi(this);    

    _sendSynForm_p = new SynForm(ui->frSendSyn);
    _sendFinForm_p = new FinForm(ui->frSendFin);
    _sendAckForm_p = new AckForm(ui->frSendAck);
    _sendDataForm_p = new DataForm(ui->frSendData);

    _recvSynForm_p = new SynForm(ui->frRecvSyn);
    _recvFinForm_p = new FinForm(ui->frRecvFin);
    _recvAckForm_p = new AckForm(ui->frRecvAck);
    _recvDataForm_p = new DataForm(ui->frRecvData);

    connect(ui->butRefresh, &QPushButton::clicked, this, &MainWindow::butRefreshClicked);
    connect(ui->butOpenClose, &QPushButton::clicked, this, &MainWindow::butOpenCloseClicked);

    connect(ui->butSendSyn, &QPushButton::clicked, this, &MainWindow::butSendSynClicked);
    connect(ui->butSendFin, &QPushButton::clicked, this, &MainWindow::butSendFinClicked);
    connect(ui->butSendAck, &QPushButton::clicked, this, &MainWindow::butSendAckClicked);
    connect(ui->butSendData, &QPushButton::clicked, this, &MainWindow::butSendDataClicked);

    connect(&_receiver, &TupReceiver::synReceived, _recvSynForm_p, &SynForm::setFrame);
    connect(&_receiver, &TupReceiver::finReceived, _recvFinForm_p, &FinForm::setFrame);
    connect(&_receiver, &TupReceiver::ackReceived, _recvAckForm_p, &AckForm::setFrame);
    connect(&_receiver, &TupReceiver::dataReceived, _recvDataForm_p, &DataForm::setFrame);

    initPortList(*ui->cbPortName);
    initBaudratesList(*ui->cbBaudrate);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::butRefreshClicked(bool checked)
{
    (void)checked;

    initPortList(*ui->cbPortName);
}

void MainWindow::butOpenCloseClicked(bool checked)
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
            ui->butOpenClose->setText("Open");
        }
        else
        {
            const auto errText = QMetaEnum::fromType<QSerialPort::SerialPortError>().valueToKey(_port.error());
            QMessageBox::critical(this, "Open port", errText);
        }
    }
}

void MainWindow::butSendSynClicked(bool checked)
{
    (void)checked;

    auto frame_p = _sendSynForm_p->getFrame();
    _sender.sendSyn(frame_p.get());
}

void MainWindow::butSendFinClicked(bool checked)
{
    (void)checked;

    auto frame_p = _sendFinForm_p->getFrame();
    _sender.sendFin(frame_p.get());
}

void MainWindow::butSendAckClicked(bool checked)
{
    (void)checked;

    auto frame_p = _sendAckForm_p->getFrame();
    _sender.sendAck(frame_p.get());
}

void MainWindow::butSendDataClicked(bool checked)
{
    (void)checked;

    auto frame_p = _sendDataForm_p->getFrame();

    _dataPayloadBuf = frame_p->payload;

    tup_v1_data_t data;

    data.j = frame_p->j;
    data.cop = frame_p->cop;
    data.end = frame_p->isFinal;
    data.payloadSize_bytes = _dataPayloadBuf.size();
    data.payload_p = _dataPayloadBuf.data();

    _sender.sendData(&data);
}

void MainWindow::initPortList(QComboBox& comboBox)
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

void MainWindow::initBaudratesList(QComboBox& comboBox)
{
    comboBox.clear();

    for (const auto br : QSerialPortInfo::standardBaudRates())
    {
        comboBox.addItem(QString::number(br), br);
    }

    comboBox.setCurrentIndex(0);
}



