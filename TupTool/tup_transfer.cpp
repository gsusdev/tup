#include "tup_transfer.h"
#include "tup_port.h"

#include <QDateTime>
#include <QThread>
#include <QDebug>

class SignalsInvoker
{
public:
    SignalsInvoker(TupTransfer& owner) :
        _owner(owner)
    {}

    void doOnCompleted(tup_transfer_result_t resultCode)
    {
        _owner.doOnCompleted(resultCode);
    }

    void doOnSyn(uint32_t j, size_t windowSize)
    {
        _owner.doOnSyn(j, windowSize);
    }

    void doOnFin()
    {
        _owner.doOnFin();
    }

    void doOnData(QByteArray payload, bool isFinal)
    {
        _owner.doOnData(payload, isFinal);
    }

    void doOnFail(tup_transfer_fail_t failCode)
    {
        _owner.doOnFail(failCode);
    }

    void doOnBadFrame(tup_frameError_t errorCode)
    {
        _owner.doOnBadFrame(errorCode);
    }

    void notify()
    {
        _owner.notify();
    }

    bool wait(uint32_t timeout_ms)
    {
        return _owner.wait(timeout_ms);
    }

    void transmit(const void* buf_p, size_t size_bytes)
    {
        _owner.transmit(buf_p, size_bytes);
    }

private:
    TupTransfer& _owner;
};

class TransferThread : public QThread
{
public:
    using func_t = tup_transfer_error_t(*)(tup_transfer_t* transfer_p);

    void setFunction(func_t func_p, tup_transfer_t* transfer_p)
    {
        _func_p = func_p;
        _transfer_p = transfer_p;
    }

    void run() override
    {
        _func_p(_transfer_p);
    }

private:
    func_t _func_p = nullptr;
    tup_transfer_t* _transfer_p = nullptr;
};

static void onCompletedHandler(tup_transfer_result_t resultCode, uintptr_t tag)
{
    auto& invoker = *reinterpret_cast<SignalsInvoker*>(tag);
    invoker.doOnCompleted(resultCode);
}

static void onSynHandler(uint32_t j, size_t windowSize, uintptr_t tag)
{
    auto& invoker = *reinterpret_cast<SignalsInvoker*>(tag);
    invoker.doOnSyn(j, windowSize);
}

static void onFinHandler(uintptr_t tag)
{
    auto& invoker = *reinterpret_cast<SignalsInvoker*>(tag);
    invoker.doOnFin();
}

static void onDataHandler(const void volatile* payload_p, size_t payloadSize_bytes, bool isFinal, uintptr_t tag)
{
    auto& invoker = *reinterpret_cast<SignalsInvoker*>(tag);

    QByteArray payload(reinterpret_cast<const char*>(const_cast<const void*>(payload_p)), payloadSize_bytes);
    invoker.doOnData(payload, isFinal);
}

static void onFailHandler(tup_transfer_fail_t failCode, uintptr_t tag)
{
    auto& invoker = *reinterpret_cast<SignalsInvoker*>(tag);
    invoker.doOnFail(failCode);
}

static void onBadFrameHandler(tup_frameError_t errorCode, uintptr_t tag)
{
    auto& invoker = *reinterpret_cast<SignalsInvoker*>(tag);
    invoker.doOnBadFrame(errorCode);
}

static void linkTransmitHandler(const void* buf_p, size_t size_bytes, uintptr_t callbackValue)
{
    auto& invoker = *reinterpret_cast<SignalsInvoker*>(callbackValue);
    invoker.transmit(buf_p, size_bytes);
}

static uint32_t getCurrentTimeHandler()
{
    const auto timestamp = QDateTime::currentMSecsSinceEpoch();
    return static_cast<uint32_t>(timestamp & 0xFFFFFFFF);
}

static void signalFireHandler(uintptr_t signal, uintptr_t callbackValue)
{
    (void)signal;

    auto& invoker = *reinterpret_cast<SignalsInvoker*>(callbackValue);
    invoker.notify();
}

static bool signalWait_Handler(uintptr_t signal, uint32_t timeout_ms, uintptr_t callbackValue)
{
    (void)signal;

    auto& invoker = *reinterpret_cast<SignalsInvoker*>(callbackValue);
    const auto result = invoker.wait(timeout_ms);

    return result;
}

TupTransfer::TupTransfer(QObject *parent) : QObject(parent)
{
    _sigOnvoker_p = new SignalsInvoker(*this);

    connect(this, SIGNAL(sigTransmit(QByteArray)), this, SLOT(slotTransmit(QByteArray)), Qt::QueuedConnection);
    _workBuffer.resize(2048);
}

TupTransfer::~TupTransfer()
{
    delete static_cast<SignalsInvoker*>(_sigOnvoker_p);
    reset();
}

void TupTransfer::setName(const QString &value)
{
    _name = value.toLatin1();
    _name.append('\0');
}

void TupTransfer::setPort(QIODevice* port_p)
{
    if (_port_p != nullptr)
    {
        disconnect(_port_p, &QObject::destroyed, this, &TupTransfer::portDestroyed);
        disconnect(_port_p, &QIODevice::readyRead, this, &TupTransfer::portReadyRead);
        disconnect(_port_p, &QIODevice::aboutToClose, this, &TupTransfer::portAboutToClose);
        disconnect(_port_p, &QIODevice::bytesWritten, this, &TupTransfer::portBytesWritten);

        reset();
    }

    _port_p = port_p;

    if (_port_p != nullptr)
    {
        connect(_port_p, &QObject::destroyed, this, &TupTransfer::portDestroyed);
        connect(_port_p, &QIODevice::readyRead, this, &TupTransfer::portReadyRead);
        connect(_port_p, &QIODevice::aboutToClose, this, &TupTransfer::portAboutToClose);
        connect(_port_p, &QIODevice::bytesWritten, this, &TupTransfer::portBytesWritten);

        init();
        run();
    }
}

void TupTransfer::sendSyn(uint32_t j)
{
    tup_transfer_sendSyn(&_transfer, j);
}

void TupTransfer::sendFin()
{
    tup_transfer_sendFin(&_transfer);
}

void TupTransfer::sendData(QByteArray payload, bool isFinal)
{
    _payload = payload;
    tup_transfer_sendData(&_transfer, payload.data(), payload.size(), isFinal);
}

void TupTransfer::setResult(tup_transfer_result_t result)
{
    tup_transfer_setResult(&_transfer, result);
}

void TupTransfer::portDestroyed(QObject* obj)
{
    (void)obj;
    reset();
    _port_p = nullptr;
}

void TupTransfer::portAboutToClose()
{
    reset();
}

void TupTransfer::portReadyRead()
{
    _receivedData = _port_p->readAll();
    tup_transfer_received(&_transfer, _receivedData.data(), _receivedData.size());
}

void TupTransfer::portBytesWritten(qint64 bytes)
{
    tup_transfer_transmitted(&_transfer, bytes);
}

void TupTransfer::slotTransmit(QByteArray data)
{
    const auto sz = _port_p->write(data);
    if (sz != data.size())
    {
        qDebug() << "Error writing to the port";
    }
}

void TupTransfer::reset()
{
    tup_transfer_stop(&_transfer);
}

bool TupTransfer::init()
{
    tup_transfer_initStruct_t initStruct;

    initStruct.onFin = onFinHandler;
    initStruct.onSyn = onSynHandler;
    initStruct.onData = onDataHandler;
    initStruct.onFail = onFailHandler;
    initStruct.onCompleted = onCompletedHandler;
    initStruct.onInvalidFrame = onBadFrameHandler;
    initStruct.tryCount = 2;
    initStruct.workBuffer_p = _workBuffer.data();
    initStruct.workBufferSize_bytes = _workBuffer.size();
    initStruct.rxBufSize_bytes = _workBuffer.size() / 2;
    initStruct.retryPause_ms = 4000;
    initStruct.synTimeout_ms = 2000;
    initStruct.dataTimeout_ms = 2000;
    initStruct.txCallbackValue = reinterpret_cast<uintptr_t>(_sigOnvoker_p);
    initStruct.flushDuration_ms = 1000;
    initStruct.userCallbackValue = reinterpret_cast<uintptr_t>(_sigOnvoker_p);
    initStruct.signalFuncsCallback = reinterpret_cast<uintptr_t>(_sigOnvoker_p);
    initStruct.name = _name.data();

    auto result = tup_transfer_init(&_transfer, &initStruct) == tup_transfer_error_ok;
    result &= tup_transfer_listen(&_transfer) == tup_transfer_error_ok;

    return result;
}

void TupTransfer::run()
{
    tup_port_setLinkTransmitHandler(linkTransmitHandler);
    tup_port_setSignalFireHandler(signalFireHandler);
    tup_port_setSignalWaitHandler(signalWait_Handler);
    tup_port_setGetCurrentTimeHandler(getCurrentTimeHandler);

    auto thread_p = new TransferThread();

    thread_p->setFunction(tup_transfer_run, &_transfer);
    connect(thread_p, &QThread::finished, thread_p, &QThread::deleteLater);

    thread_p->start();
}

void TupTransfer::notify()
{
    _waitCond.notify_all();
}

bool TupTransfer::wait(uint32_t timeout_ms)
{
    _mutex.lock();
    const auto result = _waitCond.wait(&_mutex, timeout_ms);
    _mutex.unlock();

    return result;
}

void TupTransfer::transmit(const void* buf_p, size_t size_bytes)
{
    _sendingData = QByteArray::fromRawData(static_cast<const char*>(buf_p), size_bytes);
    emit sigTransmit(_sendingData);
}

void TupTransfer::doOnCompleted(int resultCode)
{
    emit onCompleted(resultCode);
}

void TupTransfer::doOnSyn(quint32 j, quintptr windowSize)
{
    emit onSyn(j, windowSize);
}

void TupTransfer::doOnFin()
{
    emit onFin();
}

void TupTransfer::doOnData(QByteArray payload, bool isFinal)
{
    emit onData(payload, isFinal);
}

void TupTransfer::doOnFail(quint32 failCode)
{
    emit onFail(failCode);
}

void TupTransfer::doOnBadFrame(quint32 errorCode)
{
    emit onBadFrame(errorCode);
}
