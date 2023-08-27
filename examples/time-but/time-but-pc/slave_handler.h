#ifndef SLAVEHANDLER_H
#define SLAVEHANDLER_H

#include <QObject>

#include "tup_wrapper.h"
#include "app_protocol.h"

class SlaveHandler : public QObject
{
    Q_OBJECT
public:
    explicit SlaveHandler(QObject *parent = nullptr);

    void setTup(TupWrapper* tup_p);

    const app_protocol_masterOutputData_t& masterOutput() const { return _masterOutput; }

    const app_protocol_slaveOutputData_t& slaveOutput() const { return _slaveOutput; }
    app_protocol_slaveOutputData_t& slaveOutput() { return _slaveOutput; }
    void setSlaveOutput(const app_protocol_slaveOutputData_t& values) { _slaveOutput = values; }

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

    QByteArray _inputBuf;
};

#endif // SLAVEHANDLER_H
