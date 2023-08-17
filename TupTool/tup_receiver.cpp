#include "tup_receiver.h"

#include <QMetaObject>
#include <QDebug>

#include "tup_v1_body.h"

TupReceiver::TupReceiver(QObject* parent)
{
    (void)parent;

    _buf.resize(1024);

    reset();
}

void TupReceiver::setPort(QIODevice* port_p)
{
    if (_port_p != nullptr)
    {
        disconnect(_port_p, &QObject::destroyed, this, &TupReceiver::portDestroyed);
        disconnect(_port_p, &QIODevice::readyRead, this, &TupReceiver::portReadyRead);
        disconnect(_port_p, &QIODevice::aboutToClose, this, &TupReceiver::portAboutToClose);
    }

    _port_p = port_p;

    connect(_port_p, &QObject::destroyed, this, &TupReceiver::portDestroyed);
    connect(_port_p, &QIODevice::readyRead, this, &TupReceiver::portReadyRead);
    connect(_port_p, &QIODevice::aboutToClose, this, &TupReceiver::portAboutToClose);
}

void TupReceiver::portDestroyed(QObject* obj)
{
    (void)obj;
    _port_p = nullptr;
    reset();
}

void TupReceiver::portAboutToClose()
{
    reset();
}

void TupReceiver::portReadyRead()
{
    const auto receivedData = _port_p->readAll();

    auto err = tup_frameReceiver_received(&frameReceiver, receivedData.data(), receivedData.size());
    if (err != tup_frameReceiver_error_ok)
    {
        qDebug() << "frameReceiver_received error " << err;
        return;
    }

    bool isHandlingNeeded;
    err = tup_frameReceiver_isHandlingNeeded(&frameReceiver, &isHandlingNeeded);
    if (err != tup_frameReceiver_error_ok)
    {
        qDebug() << "frameReceiver_isHandlingNeeded error " << err;
        return;
    }

    if (isHandlingNeeded)
    {
        QMetaObject::invokeMethod(this, &TupReceiver::receiverHandlingNeeded);
    }
}

void TupReceiver::receiverHandlingNeeded()
{
    auto err = tup_frameReceiver_handle(&frameReceiver);
    if (err != tup_frameReceiver_error_ok)
    {
        qDebug() << "frameReceiver_handle error " << err;
        return;
    }

    tup_frameReceiver_status_t status;
    err = tup_frameReceiver_getStatus(&frameReceiver, &status);
    if (err != tup_frameReceiver_error_ok)
    {
        qDebug() << "frameReceiver_getStatus error " << err;
        return;
    }

    switch (status)
    {
        case tup_frameReceiver_status_received:
            frameReceived();
            tup_frameReceiver_reset(&frameReceiver);
            tup_frameReceiver_listen(&frameReceiver);
            break;

        case tup_frameReceiver_status_receiving:
        case tup_frameReceiver_status_idle:
            break;

        default:
            emit badFrameReceived(status);
            tup_frameReceiver_reset(&frameReceiver);
            tup_frameReceiver_listen(&frameReceiver);
    }
}

void TupReceiver::reset()
{
    tup_frameReceiver_initStruct_t init;

    init.inputBuffer_p = _buf.data();
    init.bufferSize_bytes = _buf.size();

    auto err = tup_frameReceiver_init(&frameReceiver, &init);
    if (err != tup_frameReceiver_error_ok)
    {
        qDebug() << "frameReceiver_init error " << err;
        return;
    }

    err = tup_frameReceiver_listen(&frameReceiver);
    if (err != tup_frameReceiver_error_ok)
    {
        qDebug() << "frameReceiver_listen error " << err;
    }
}


void TupReceiver::frameReceived()
{
    size_t size;
    tup_version_t version;
    volatile const void* body_p;

    auto errRecv = tup_frameReceiver_getReceivedBody(&frameReceiver, &body_p, &size, &version);
    if (errRecv != tup_frameReceiver_error_ok)
    {
        qDebug() << "frameReceiver_getReceivedBody error " << errRecv;
        return;
    }

    tup_v1_cop_t type;
    auto errBody = tup_v1_body_getType(body_p, size, &type);
    if (errBody != tup_body_error_ok)
    {
        qDebug() << "body_getType error " << errBody;
        return;
    }

    switch (type)
    {
        case tup_v1_cop_syn:
            decodeSyn(body_p, size);
            break;

        case tup_v1_cop_ack:
            decodeAck(body_p, size);
            break;

        case tup_v1_cop_data:
            decodeData(body_p, size);
            break;

        case tup_v1_cop_fin:
            decodeFin(body_p, size);
            break;
    }
}

void TupReceiver::decodeSyn(const volatile void* buf_p, size_t size)
{
    auto frame_p = PSynFrame(new tup_v1_syn_t());

    const auto err = tup_v1_syn_decode(buf_p, size, frame_p.data());
    if (err != tup_body_error_ok)
    {
        qDebug() << "syn_decode error " << err;
        return;
    }

    emit synReceived(frame_p);
}

void TupReceiver::decodeFin(const volatile void* buf_p, size_t size)
{
    auto frame_p = PFinFrame(new tup_v1_fin_t());

    const auto err = tup_v1_fin_decode(buf_p, size, frame_p.data());
    if (err != tup_body_error_ok)
    {
        qDebug() << "fin_decode error " << err;
        return;
    }

    emit finReceived(frame_p);
}

void TupReceiver::decodeAck(const volatile void* buf_p, size_t size)
{
    auto frame_p = PAckFrame(new tup_v1_ack_t());

    const auto err = tup_v1_ack_decode(buf_p, size, frame_p.data());
    if (err != tup_body_error_ok)
    {
        qDebug() << "ack_decode error " << err;
        return;
    }

    emit ackReceived(frame_p);
}

void TupReceiver::decodeData(const volatile void* buf_p, size_t size)
{
    tup_v1_data_t receivedDataFrame;
    const auto err = tup_v1_data_decode(buf_p, size, &receivedDataFrame);
    if (err != tup_body_error_ok)
    {
        qDebug() << "data_decode error " << err;
        return;
    }

    auto frame_p = PDataFrame(new DataFrame(receivedDataFrame));

    emit dataReceived(frame_p);
}

DataFrame::DataFrame(const tup_v1_data_t& src)
{
    j = src.j;
    cop = src.cop;
    isFinal = src.end;
    payload.resize(src.payloadSize_bytes);

    const char volatile* src_p = static_cast<const char volatile*>(src.payload_p);
    auto* dst_p = payload.data();

    for (size_t i = 0; i < src.payloadSize_bytes; ++i)
    {
        *dst_p++ = *src_p++;
    }
}
