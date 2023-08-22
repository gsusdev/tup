#include "tup_instance_w.h"

#include <QDateTime>
#include <QDebug>

#include "tup_port.h"

class InstanceSignalsInvoker
{
public:
    InstanceSignalsInvoker(TupInstance& owner) :
        _owner(owner)
    {}

    void doOnConnect()
    {
        _owner.doOnConnect();
    }

    void doOnDisconnectRequest()
    {
        _owner.doOnDisconnectRequest();
    }

    void doOnSendDataProgress(quintptr sentSize_bytes, quintptr totalSize_bytes)
    {
        _owner.doOnSendDataProgress(sentSize_bytes, totalSize_bytes);
    }

    void doOnReceiveData(QByteArray data, quint8 isFinal)
    {
        _owner.doOnReceiveData(data, isFinal);
    }

    void doOnFail(tup_transfer_fail_t failCode)
    {
        _owner.doOnFail(failCode);
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
    TupInstance& _owner;
};

class TupInstance::InstanceThread : public QThread
{
public:
    using func_t = tup_error_t(*)(tup_instance_t* instance_p);

    void setFunction(func_t func_p, tup_instance_t* instance_p)
    {
        _func_p = func_p;
        _instance_p = instance_p;
    }

    void run() override
    {
        _func_p(_instance_p);
    }

private:
    func_t _func_p = nullptr;
    tup_instance_t* _instance_p = nullptr;
};

static void onConnectHandler(uintptr_t callbackValue)
{
    auto& invoker = *reinterpret_cast<InstanceSignalsInvoker*>(callbackValue);
    invoker.doOnConnect();
}

static void onDisconnectRequestHandler(uintptr_t callbackValue)
{
    auto& invoker = *reinterpret_cast<InstanceSignalsInvoker*>(callbackValue);
    invoker.doOnDisconnectRequest();
}

static void onSendDataProgressHandler(size_t sentSize_bytes, size_t totalSize_bytes, uintptr_t callbackValue)
{
    auto& invoker = *reinterpret_cast<InstanceSignalsInvoker*>(callbackValue);
    invoker.doOnSendDataProgress(sentSize_bytes, totalSize_bytes);
}

static void onReceiveDataHandler(const void volatile* buf_p, size_t size_bytes, bool isFinal, uintptr_t callbackValue)
{
    auto& invoker = *reinterpret_cast<InstanceSignalsInvoker*>(callbackValue);

    QByteArray receivedData(reinterpret_cast<const char*>(const_cast<const void*>(buf_p)), size_bytes);
    invoker.doOnReceiveData(receivedData, isFinal);
}

static void onFailHandler(tup_transfer_fail_t failCode, uintptr_t callbackValue)
{
    auto& invoker = *reinterpret_cast<InstanceSignalsInvoker*>(callbackValue);
    invoker.doOnFail(failCode);
}

static void linkTransmitHandler(const void* buf_p, size_t size_bytes, uintptr_t callbackValue)
{
    auto& invoker = *reinterpret_cast<InstanceSignalsInvoker*>(callbackValue);
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

    auto& invoker = *reinterpret_cast<InstanceSignalsInvoker*>(callbackValue);
    invoker.notify();
}

static bool signalWaitHandler(uintptr_t signal, uint32_t timeout_ms, uintptr_t callbackValue)
{
    (void)signal;

    auto& invoker = *reinterpret_cast<InstanceSignalsInvoker*>(callbackValue);
    const auto result = invoker.wait(timeout_ms);

    return result;
}

TupInstance::TupInstance(QObject *parent)
    : QObject{parent}
{
    _sigInvoker_p = new InstanceSignalsInvoker(*this);

    _initStruct.synTimeout_ms = 2000;
    _initStruct.dataTimeout_ms = 5000;
    _initStruct.tryCount = 5;
    _initStruct.retryPause_ms = 5000;
    _initStruct.flushDuration_ms = 500;
    _initStruct.isMaster = false;
    _initStruct.rxBufSize_bytes = 1024;
}

TupInstance::~TupInstance()
{
    stop();
    delete static_cast<InstanceSignalsInvoker*>(_sigInvoker_p);
}

void TupInstance::setPort(QIODevice *port_p)
{
    if (_port_p != nullptr)
    {
        stop();

        disconnect(_port_p, &QObject::destroyed, this, &TupInstance::portDestroyed);
        disconnect(_port_p, &QIODevice::readyRead, this, &TupInstance::portReadyRead);
        disconnect(_port_p, &QIODevice::aboutToClose, this, &TupInstance::portAboutToClose);
        disconnect(_port_p, &QIODevice::bytesWritten, this, &TupInstance::portBytesWritten);
    }

    _port_p = port_p;

    if (_port_p != nullptr)
    {
        connect(_port_p, &QObject::destroyed, this, &TupInstance::portDestroyed);
        connect(_port_p, &QIODevice::readyRead, this, &TupInstance::portReadyRead);
        connect(_port_p, &QIODevice::aboutToClose, this, &TupInstance::portAboutToClose);
        connect(_port_p, &QIODevice::bytesWritten, this, &TupInstance::portBytesWritten);
    }
}

void TupInstance::setName(const QString& value)
{
    _name = value.toLatin1();
    _name.append('\0');
}

bool TupInstance::run()
{
    tup_port_setLinkTransmitHandler(linkTransmitHandler);
    tup_port_setSignalFireHandler(signalFireHandler);
    tup_port_setSignalWaitHandler(signalWaitHandler);
    tup_port_setGetCurrentTimeHandler(getCurrentTimeHandler);

    _workBuf.resize(_initStruct.rxBufSize_bytes * 2);

    _initStruct.onConnect = onConnectHandler;
    _initStruct.onDisconnectRequest = onDisconnectRequestHandler;
    _initStruct.onFail = onFailHandler;
    _initStruct.onReceiveData = onReceiveDataHandler;
    _initStruct.onSendDataProgress = onSendDataProgressHandler;

    _initStruct.workBuffer_p = _workBuf.data();
    _initStruct.workBufferSize_bytes = _workBuf.size();

    const auto invoker = reinterpret_cast<uintptr_t>(_sigInvoker_p);
    _initStruct.signalFuncsCallback = invoker;
    _initStruct.userCallbackValue = invoker;
    _initStruct.txCallbackValue = invoker;
    _initStruct.signal = 1;
    _initStruct.name = _name.data();

    const auto err = tup_init(&_instance, &_initStruct);
    if (err != tup_error_ok)
    {
        return false;
    }

    _thread_p = new InstanceThread();
    _thread_p->setFunction(tup_run, &_instance);
    connect(_thread_p, &QThread::finished, _thread_p, &QThread::deleteLater);

    _thread_p->start();

    return true;
}

void TupInstance::stop()
{
    if (_thread_p == nullptr)
    {
        return;
    }

    const auto err = tup_stop(&_instance);
    if (err == tup_error_ok)
    {
        if (_thread_p->wait(3000))
        {
            _thread_p = nullptr;
        }
    }
}

bool TupInstance::tupConnect()
{
    const auto err = tup_connect(&_instance);
    return err == tup_error_ok;
}

bool TupInstance::tupAccept()
{
    const auto err = tup_accept(&_instance);
    return err == tup_error_ok;
}

bool TupInstance::sendFin()
{
    const auto err = tup_sendFin(&_instance);
    return err == tup_error_ok;
}

bool TupInstance::sendData(QByteArray data)
{
    _sendingData = data;
    const auto err = tup_sendData(&_instance, _sendingData.data(), _sendingData.size());
    return err == tup_error_ok;
}

bool TupInstance::setResult(tup_transfer_result_t result)
{
    const auto err = tup_setResult(&_instance, result);
    return err == tup_error_ok;
}

void TupInstance::portDestroyed(QObject *obj)
{
    (void)obj;

    stop();
    _port_p = nullptr;
}

void TupInstance::portAboutToClose()
{
    stop();
}

void TupInstance::portReadyRead()
{
    _receivedData = _port_p->readAll();
    tup_received(&_instance, _receivedData.data(), _receivedData.size());
}

void TupInstance::portBytesWritten(qint64 bytes)
{
    tup_transmitted(&_instance, bytes);
}

void TupInstance::slotTransmit(QByteArray data)
{
    _port_p->write(data);
}

void TupInstance::notify()
{
    _waitCond.notify_all();
}

bool TupInstance::wait(uint32_t timeout_ms)
{
    _mutex.lock();
    const auto result = _waitCond.wait(&_mutex, timeout_ms);
    _mutex.unlock();

    return result;
}

void TupInstance::transmit(const void *buf_p, size_t size_bytes)
{
    auto data = QByteArray::fromRawData(static_cast<const char*>(buf_p), size_bytes);
    QMetaObject::invokeMethod(this, "slotTransmit", Q_ARG(QByteArray, data));
}

void TupInstance::doOnConnect()
{
    emit onConnect();
}

void TupInstance::doOnDisconnectRequest()
{
    emit onDisconnectRequest();
}

void TupInstance::doOnSendDataProgress(quintptr sentSize_bytes, quintptr totalSize_bytes)
{
    emit onSendDataProgress(sentSize_bytes, totalSize_bytes);
}

void TupInstance::doOnReceiveData(QByteArray data, quint8 isFinal)
{
    emit onReceiveData(data, isFinal);
}

void TupInstance::doOnFail(quint32 failCode)
{
    emit onFail(failCode);
}


