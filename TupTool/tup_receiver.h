#pragma once

#include <cstdint>
#include <cstdbool>

#include <QObject>
#include <QIODevice>
#include <QByteArray>
#include <QSharedPointer>

#include "tup_frame_receiver.h"
#include "tup_v1_body.h"

#include "common.h"


class TupReceiver : public QObject
{
    Q_OBJECT
public:
    explicit TupReceiver(QObject *parent = nullptr);
    void setPort(QIODevice* port_p);

signals:
    void badFrameReceived(tup_frameReceiver_status_t error);
    void synReceived(PSynFrame frame_p);
    void finReceived(PFinFrame frame_p);
    void ackReceived(PAckFrame frame_p);
    void dataReceived(PDataFrame frame_p);

private slots:
    void portDestroyed(QObject *obj);
    void portAboutToClose();
    void portReadyRead();
    void receiverHandlingNeeded();

private:
    void reset();

    void frameReceived();

    void decodeSyn(const void volatile* buf_p, size_t size);
    void decodeFin(const void volatile* buf_p, size_t size);
    void decodeAck(const void volatile* buf_p, size_t size);
    void decodeData(const void volatile* buf_p, size_t size);

    QIODevice* _port_p = nullptr;
    tup_frameReceiver_t frameReceiver;
};
