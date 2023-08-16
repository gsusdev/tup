#include "tup_receiver.h"

#include <QMetaObject>

#include "tup_v1_body.h"

TupReceiver::TupReceiver(QObject* parent)
{
    (void)parent;

    reset();
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
        return;
    }

    bool isHandlingNeeded;
    err = tup_frameReceiver_isHandlingNeeded(&frameReceiver, &isHandlingNeeded);
    if (err != tup_frameReceiver_error_ok)
    {
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
        return;
    }

    tup_frameReceiver_status_t status;
    err = tup_frameReceiver_getStatus(&frameReceiver, &status);

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
    tup_frameReceiver_init(&frameReceiver, &init);
}

void TupReceiver::frameReceived()
{
    size_t size;
    tup_version_t version;
    volatile const void* body_p;

    auto errRecv = tup_frameReceiver_getReceivedBody(&frameReceiver, &body_p, &size, &version);
    if (errRecv != tup_frameReceiver_error_ok)
    {
        return;
    }

    tup_v1_cop_t type;
    auto errBody = tup_v1_body_getType(body_p, size, &type);
    if (errBody != tup_body_error_ok)
    {
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
        *dst_p = *src_p;
    }
}
