#pragma once

#include <QObject>
#include <QIODevice>
#include <QByteArray>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>

#include <cstdint>
#include <cstdbool>

#include "tup_instance.h"

class TupWrapper : public QObject
{
    Q_OBJECT
    friend class InstanceSignalsInvoker;
public:
    explicit TupWrapper(QObject *parent = nullptr);
    ~TupWrapper();

    bool isConnected() const { return _isConnected; }

    void setPort(QIODevice* port_p);
    void setName(const QString& value);

    void setSynTimeout_ms(quint32 value) { _initStruct.synTimeout_ms = value; }
    void setDataTimeout_ms(quint32 value) { _initStruct.dataTimeout_ms = value; }
    void setTryCount(quint32 value) { _initStruct.tryCount = value; }
    void setRetryPause_ms(quint32 value) { _initStruct.retryPause_ms = value; }
    void setFlushDuration_ms(quint32 value) { _initStruct.flushDuration_ms = value; }
    void setIsMaster(bool value) { _initStruct.isMaster = value; }
    void setRxBufSize(quintptr value) { _initStruct.rxBufSize_bytes = value; }

    bool run();
    void stop();

    bool tupConnect();
    bool tupAccept();

    bool sendFin();
    bool sendData(QByteArray data);
    bool setResult(tup_transfer_result_t result);

signals:
    void onConnect();
    void onDisconnectRequest();
    void onSendDataProgress(quintptr sentSize_bytes, quintptr totalSize_bytes);
    void onResultSent();
    void onReceiveData(QByteArray data, quint8 isFinal);
    void onFail(quint32 failCode);

private slots:
    void portDestroyed(QObject *obj);
    void portAboutToClose();
    void portReadyRead();
    void portBytesWritten(qint64 bytes);

    void slotTransmit(QByteArray data);

private:
    class InstanceThread;

    void notify();
    bool wait(uint32_t timeout_ms);
    void transmit(const void* buf_p, size_t size_bytes);

    void doOnConnect();
    void doOnDisconnectRequest();
    void doOnSendDataProgress(quintptr sentSize_bytes, quintptr totalSize_bytes);
    void doOnResultSent();
    void doOnReceiveData(QByteArray data, quint8 isFinal);
    void doOnFail(quint32 failCode);

    bool _isConnected = false;

    QMutex _mutex;
    QWaitCondition _waitCond;
    InstanceThread* _thread_p = nullptr;
    void* _sigInvoker_p = nullptr;

    QByteArray _receivedData;
    QByteArray _sendingData;
    QIODevice* _port_p = nullptr;

    tup_instance_t _instance;
    tup_initStruct_t _initStruct;
    QByteArray _workBuf;

    QByteArray _name;
};
