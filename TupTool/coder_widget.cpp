#include "coder_widget.h"
#include "ui_coder_widget.h"

CoderWidget::CoderWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CoderWidget)
{
    ui->setupUi(this);

    _sendSynForm_p = new SynWidget(ui->frSendSyn);
    _sendFinForm_p = new FinWidget(ui->frSendFin);
    _sendAckForm_p = new AckWidget(ui->frSendAck);
    _sendDataForm_p = new DataWidget(ui->frSendData);

    _recvSynForm_p = new SynWidget(ui->frRecvSyn);
    _recvFinForm_p = new FinWidget(ui->frRecvFin);
    _recvAckForm_p = new AckWidget(ui->frRecvAck);
    _recvDataForm_p = new DataWidget(ui->frRecvData);

    connect(ui->butSendSyn, &QPushButton::clicked, this, &CoderWidget::butSendSynClicked);
    connect(ui->butSendFin, &QPushButton::clicked, this, &CoderWidget::butSendFinClicked);
    connect(ui->butSendAck, &QPushButton::clicked, this, &CoderWidget::butSendAckClicked);
    connect(ui->butSendData, &QPushButton::clicked, this, &CoderWidget::butSendDataClicked);

    connect(&_receiver, &TupReceiver::synReceived, _recvSynForm_p, &SynWidget::setFrame);
    connect(&_receiver, &TupReceiver::finReceived, _recvFinForm_p, &FinWidget::setFrame);
    connect(&_receiver, &TupReceiver::ackReceived, _recvAckForm_p, &AckWidget::setFrame);
    connect(&_receiver, &TupReceiver::dataReceived, _recvDataForm_p, &DataWidget::setFrame);
}

CoderWidget::~CoderWidget()
{
    delete ui;
}

void CoderWidget::setPort(QIODevice* port_p)
{
    _sender.setPort(port_p);
    _receiver.setPort(port_p);
}

void CoderWidget::butSendSynClicked(bool checked)
{
    (void)checked;

    auto frame_p = _sendSynForm_p->getFrame();
    _sender.sendSyn(frame_p.get());
}

void CoderWidget::butSendFinClicked(bool checked)
{
    (void)checked;

    auto frame_p = _sendFinForm_p->getFrame();
    _sender.sendFin(frame_p.get());
}

void CoderWidget::butSendAckClicked(bool checked)
{
    (void)checked;

    auto frame_p = _sendAckForm_p->getFrame();
    _sender.sendAck(frame_p.get());
}

void CoderWidget::butSendDataClicked(bool checked)
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
