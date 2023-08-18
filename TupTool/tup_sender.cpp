#include "tup_sender.h"

#include <QDebug>

#include "tup_v1_body.h"

TupSender::TupSender(QObject *parent) : QObject(parent)
{
    _buf.resize(1024);
    reset();
}

void TupSender::setPort(QIODevice* port_p)
{
    if (_port_p != nullptr)
    {
        disconnect(_port_p, &QObject::destroyed, this, &TupSender::portDestroyed);
        disconnect(_port_p, &QIODevice::bytesWritten, this, &TupSender::portBytesWritten);
        disconnect(_port_p, &QIODevice::aboutToClose, this, &TupSender::portAboutToClose);
    }

    _port_p = port_p;

    if (_port_p != nullptr)
    {
        connect(_port_p, &QObject::destroyed, this, &TupSender::portDestroyed);
        connect(_port_p, &QIODevice::bytesWritten, this, &TupSender::portBytesWritten);
        connect(_port_p, &QIODevice::aboutToClose, this, &TupSender::portAboutToClose);
    }
}

bool TupSender::sendSyn(const tup_v1_syn_t* data_p)
{
    const auto result = genericSend<tup_v1_syn_t>(data_p, tup_v1_syn_encode);
    return result;
}

bool TupSender::sendAck(const tup_v1_ack_t* data_p)
{
    const auto result = genericSend<tup_v1_ack_t>(data_p, tup_v1_ack_encode);
    return result;
}

bool TupSender::sendFin(const tup_v1_fin_t* data_p)
{
    const auto result = genericSend<tup_v1_fin_t>(data_p, tup_v1_fin_encode);
    return result;
}

bool TupSender::sendData(const tup_v1_data_t* data_p)
{
    const auto result = genericSend<tup_v1_data_t>(data_p, tup_v1_data_encode);
    return result;
}

void TupSender::portDestroyed(QObject* obj)
{
    (void)obj;
    _port_p = nullptr;
    reset();
}

void TupSender::portAboutToClose()
{
    reset();
}

void TupSender::portBytesWritten(qint64 bytes)
{
    bool isFinished;
    auto res = tup_frameSender_txCompleted(&frameSender, bytes, &isFinished);
    if (res != tup_frameSender_error_ok)
    {
        return;
    }

    if (!isFinished)
    {
        fetchAndSend();
    }
}

bool TupSender::fetchAndSend()
{
    const void* dataToSend_p;
    size_t sizeToSend;
    const auto getDataResult = tup_frameSender_getDataToSend(&frameSender, &dataToSend_p, &sizeToSend);
    if (getDataResult != tup_frameSender_error_ok)
    {
        qDebug() << "frameSender_getDataToSend error " << getDataResult;
        return false;
    }

    const auto writtenSize = _port_p->write(static_cast<const char*>(dataToSend_p), sizeToSend);
    if (writtenSize == -1)
    {
        qDebug() << "Port error " << _port_p->errorString();
        return false;
    }

    return true;
}

void TupSender::reset()
{
    tup_frameSender_initStruct_t init;
    tup_frameSender_init(&frameSender, &init);
}
