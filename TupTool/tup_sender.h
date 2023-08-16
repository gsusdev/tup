#pragma once

#include <cstdint>
#include <cstdbool>

#include <QObject>
#include <QIODevice>
#include <QByteArray>

#include "tup_frame_sender.h"
#include "tup_v1_body.h"

class TupSender : public QObject
{
    Q_OBJECT
public:
    explicit TupSender(QObject *parent = nullptr);
    void setPort(QIODevice* port_p);

    bool sendSyn(const tup_v1_syn_t* data_p);
    bool sendAck(const tup_v1_ack_t* data_p);
    bool sendFin(const tup_v1_fin_t* data_p);
    bool sendData(const tup_v1_data_t* data_p);

private slots:
    void portDestroyed(QObject *obj);
    void portAboutToClose();
    void portBytesWritten(qint64 bytes);

private:
    template <class T>
    using EncoderFunc = tup_body_error_t (*)(const T* in_p, void* buf_out_p, size_t bufSize_bytes, size_t* actualSize_bytes_out_p);

    bool fetchAndSend();
    void reset();

    template <class T>
    bool genericSend(const T* inputFrame_p, EncoderFunc<T> encoder)
    {
        if (_port_p == NULL)
        {
            return false;
        }

        if (!_port_p->isOpen() || !_port_p->isWritable())
        {
            return false;
        }

        size_t size;
        const auto encodeResult = encoder(inputFrame_p, _buf.data(), _buf.size(), &size);
        if (encodeResult != tup_body_error_ok)
        {
            return false;
        }

        const auto sendResult = tup_frameSender_send(&frameSender, TUP_VERSION_1, _buf.data(), size);
        if (sendResult != tup_frameSender_error_ok)
        {
            return false;
        }

        if (!fetchAndSend())
        {
            return false;
        }

        return true;
    }

    QIODevice* _port_p = nullptr;
    tup_frameSender_t frameSender;
    QByteArray _buf;
};
