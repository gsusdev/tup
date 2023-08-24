#pragma once

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

    const app_protocol_slaveOutputData_t& getSlaveOutput() const { return _slaveOutput; }

    const app_protocol_masterOutputData_t& getMasterOutput() const { return _masterOutput; }
    app_protocol_masterOutputData_t& getMasterOutput() { return _masterOutput; }
    void setMasterOutput(const app_protocol_masterOutputData_t& values) { _masterOutput = values; }

    void sendData();

signals:
    void sigOnUpdated();
    void sigOnBadFrame();

private slots:
    void onReceiveData(QByteArray data, quint8 isFinal);

private:
    TupWrapper* _tup_p = nullptr;

    app_protocol_masterOutputData_t _masterOutput;
    app_protocol_slaveOutputData_t _slaveOutput;

    QByteArray _inputBuf;
};
