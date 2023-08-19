#pragma once

#include <QObject>
#include <QIODevice>
#include <QWaitCondition>
#include <QMutex>

#include "tup_v1_transfer.h"

class TupTransfer : public QObject
{
    Q_OBJECT
public:
    explicit TupTransfer(QObject *parent = nullptr);
    ~TupTransfer();

    void setName(const QString& value);
    void setPort(QIODevice* port_p);

    void sendSyn(uint32_t j);
    void sendFin();
    void sendData(QByteArray payload, bool isFinal);
    void setResult(tup_transfer_result_t result);

signals:
    void onCompleted(int resultCode);
    void onSyn(quint32 j, quintptr windowSize);
    void onFin();
    void onData(QByteArray payload, bool isFinal);
    void onFail(quint32 failCode);
    void onBadFrame(quint32 errorCode);

    void sigTransmit(QByteArray data);

private slots:
    void portDestroyed(QObject *obj);
    void portAboutToClose();
    void portReadyRead();
    void portBytesWritten(qint64 bytes);
    void process();

    void slotTransmit(QByteArray data);

private:
    void reset();

    bool init();
    void run();

    void notify();
    bool wait(uint32_t timeout_ms);
    void transmit(const void* buf_p, size_t size_bytes);

    void doOnCompleted(int resultCode);
    void doOnSyn(quint32 j, quintptr windowSize);
    void doOnFin();
    void doOnData(QByteArray payload, bool isFinal);
    void doOnFail(quint32 failCode);
    void doOnBadFrame(quint32 errorCode);

    tup_transfer_t _transfer;
    QByteArray _payload;
    QByteArray _sendingData;
    QByteArray _receivedData;
    QByteArray _workBuffer;

    friend class SignalsInvoker;
    void* _sigOnvoker_p = nullptr;

    QIODevice* _port_p = nullptr;
    QMutex _mutex;
    QWaitCondition _waitCond;

    QByteArray _name;
};
