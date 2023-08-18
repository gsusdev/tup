#pragma once

#include <QObject>
#include <QIODevice>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include "tup_v1_body.h"
#include "tup_v1_transfer.h"

class TupTransfer : public QObject
{
    Q_OBJECT
public:
    explicit TupTransfer(QObject *parent = nullptr);
    ~TupTransfer();

    void setPort(QIODevice* port_p);

signals:
    void onCompleted(tup_transfer_result_t resultCode);
    void onSyn(uint32_t j, size_t windowSize);
    void onFin();
    void onData(QByteArray payaload, bool isFinal);
    void onFail(tup_transfer_fail_t failCode);

public slots:    
    void sendSyn(uint32_t j);
    void sendFin();
    void sendData(QByteArray payload, bool isFinal);
    void setResult(tup_transfer_result_t result);

private slots:
    void portDestroyed(QObject *obj);
    void portAboutToClose();
    void portReadyRead();
    void portBytesWritten(qint64 bytes);
    void process();

private:
    void reset();

    bool init();
    void run();

    void notify();
    bool wait(uint32_t timeout_ms);
    void transmit(const void* buf_p, size_t size_bytes);

    void doOnCompleted(tup_transfer_result_t resultCode);
    void doOnSyn(uint32_t j, size_t windowSize);
    void doOnFin();
    void doOnData(QByteArray payload, bool isFinal);
    void doOnFail(tup_transfer_fail_t failCode);

    tup_transfer_t _transfer;
    QByteArray _payload;
    QByteArray _sendingData;
    QByteArray _receivedData;
    QByteArray _workBuffer;

    friend class SignalsInvoker;
    void* _sigOnvoker_p;

    QIODevice* _port_p;
    QMutex _mutex;
    QWaitCondition _waitCond;
    QThread* _thread_p;
};
