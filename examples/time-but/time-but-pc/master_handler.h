#pragma once

#include <cstdbool>
#include <atomic>

#include <QObject>
#include <QByteArray>

#include "tup_wrapper.h"
#include "app_protocol.h"

class MasterHandler : public QObject
{
    Q_OBJECT
public:
    explicit MasterHandler(QObject *parent = nullptr);

    void setTup(TupWrapper* tup_p);
    bool isBusy() const { return _isBusy; }

    const app_protocol_slaveOutputData_t& slaveOutput() const { return _slaveOutput; }

    const app_protocol_masterOutputData_t& masterOutput() const { return _masterOutput; }
    app_protocol_masterOutputData_t& masterOutput() { return _masterOutput; }
    void setMasterOutput(const app_protocol_masterOutputData_t& values) { _masterOutput = values; }

    void sendData();
    void reset();

signals:
    void sigOnUpdated();
    void sigOnBadFrame();

private slots:
    void onReceiveData(QByteArray data, quint8 isFinal);
    void onResultSent();

private:
    TupWrapper* _tup_p = nullptr;

    app_protocol_masterOutputData_t _masterOutput;
    app_protocol_slaveOutputData_t _slaveOutput;

    bool _isDataReceived = false;
    std::atomic_bool _isBusy;

    QByteArray _inputBuf;
};
