#include "transfer_widget.h"
#include "ui_transfer_widget.h"

#include <QMessageBox>

#include "utils.h"

TransferWidget::TransferWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransferWidget)
{
    ui->setupUi(this);

    connect(ui->butSendSyn, &QPushButton::clicked, this, &TransferWidget::butSendSynClicked);
    connect(ui->butSendFin, &QPushButton::clicked, this, &TransferWidget::butSendFinClicked);
    connect(ui->butSendData, &QPushButton::clicked, this, &TransferWidget::butSendDataClicked);
    connect(ui->butSetResult, &QPushButton::clicked, this, &TransferWidget::butSetResultClicked);

    connect(ui->butClearInput, &QPushButton::clicked, this, &TransferWidget::butClearInputClicked);

    connect(&_transfer, &TupTransfer::onSyn, this, &TransferWidget::onSyn);
    connect(&_transfer, &TupTransfer::onFin, this, &TransferWidget::onFin);
    connect(&_transfer, &TupTransfer::onData, this, &TransferWidget::onData);
    connect(&_transfer, &TupTransfer::onCompleted, this, &TransferWidget::onCompleted);
    connect(&_transfer, &TupTransfer::onFail, this, &TransferWidget::onFail);
    connect(&_transfer, &TupTransfer::onBadFrame, this, &TransferWidget::onBadFrame);

    initErrorList(*ui->cbError);
}

TransferWidget::~TransferWidget()
{
    delete ui;
}

void TransferWidget::setPort(QIODevice *port_p)
{
    _transfer.setPort(port_p);
}

void TransferWidget::butSendSynClicked(bool checked)
{
    (void)checked;

    _transfer.sendSyn(ui->leJ->text().toUInt(nullptr, 0));
}

void TransferWidget::butSendFinClicked(bool checked)
{
    (void)checked;

    _transfer.sendFin();
}

void TransferWidget::butSetResultClicked(bool checked)
{
    (void)checked;

    _transfer.setResult(static_cast<tup_transfer_result_t>(ui->cbError->currentData().toUInt()));
}

void TransferWidget::butSendDataClicked(bool checked)
{
    (void)checked;

    auto payload = ui->lePayload->text().toUtf8();
    _transfer.sendData(payload, ui->chkFinal->isChecked());
}

void TransferWidget::butClearInputClicked(bool checked)
{
    (void)checked;

    ui->teInput->clear();
}

void TransferWidget::onCompleted(int resultCode)
{
    ui->teInput->append(QString("COMPLETED: %1\n").arg(resultCode));
}

void TransferWidget::onSyn(quint32 j, quintptr windowSize)
{
    ui->teInput->append(QString("SYN: j=%1, windowSize=%2\n").arg(j).arg(windowSize));
}

void TransferWidget::onFin()
{
    ui->teInput->append(QString("FIN\n"));
}

void TransferWidget::onData(QByteArray payload, bool isFinal)
{
    auto text = QString::fromUtf8(payload);
    ui->teInput->append(QString("DATA: final=%1, payload=%2\n").arg(isFinal).arg(text));
}

void TransferWidget::onFail(quint32 failCode)
{
    ui->teInput->append(QString("FAIL: %1\n").arg(failCode));
}

void TransferWidget::onBadFrame(quint32 errorCode)
{
    ui->teInput->append(QString("Bad frame: %1\n").arg(errorCode));
}

