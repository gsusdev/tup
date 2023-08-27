#include "slave_handler.h"

#include <QDebug>

SlaveHandler::SlaveHandler(QObject *parent) : QObject(parent)
{

}

void SlaveHandler::setTup(TupWrapper* tup_p)
{
    if (_tup_p != nullptr)
    {
        disconnect(_tup_p, &TupWrapper::onReceiveData, this, &SlaveHandler::onReceiveData);
        disconnect(_tup_p, &TupWrapper::onResultSent, this, &SlaveHandler::onResultSent);
    }

    _tup_p = tup_p;

    if (_tup_p != nullptr)
    {
        connect(_tup_p, &TupWrapper::onReceiveData, this, &SlaveHandler::onReceiveData);
        connect(_tup_p, &TupWrapper::onResultSent, this, &SlaveHandler::onResultSent);
    }
}

void SlaveHandler::onReceiveData(QByteArray data, quint8 isFinal)
{
    if (_tup_p == nullptr)
    {
        return;
    }

    _inputBuf.append(data);
    if (_inputBuf.size() > APP_PROTOCOL_MASTER_MESSAGE_SIZE_BYTES)
    {
        _inputBuf.clear();
        emit sigOnBadFrame();

        if (!_tup_p->setResult(TUP_ERROR_NOMEMORY))
        {
            qDebug() << "Slave failed to set result";
        }

        return;
    }

    if (!isFinal)
    {
        return;
    }

    if (_inputBuf.size() != APP_PROTOCOL_MASTER_MESSAGE_SIZE_BYTES)
    {
        _inputBuf.clear();
        emit sigOnBadFrame();

        if (!_tup_p->setResult(TUP_ERROR_LEN))
        {
            qDebug() << "Slave failed to set result";
        }

        return;
    }

    app_protocol_masterOutputData_t decodedData;
    if (app_protocol_decodeMasterOutput(_inputBuf.data(), _inputBuf.size(), &decodedData))
    {
        _masterOutput = decodedData;
        emit sigOnUpdated();

        if (!_tup_p->setResult(TUP_OK))
        {
            qDebug() << "Slave failed to set result";
        }
    }
    else
    {
        emit sigOnBadFrame();
        if (!_tup_p->setResult(TUP_ERROR_UNKNOWN))
        {
            qDebug() << "Slave failed to set result";
        }
    }

    _inputBuf.clear();
}

void SlaveHandler::onResultSent()
{
    if (_tup_p == nullptr)
    {
        return;
    }

    auto sz = app_protocol_encodeSlaveOutput(&_slaveOutput, nullptr, 0);
    if (sz > 0)
    {
        QByteArray buf(sz, 0);
        sz = app_protocol_encodeSlaveOutput(&_slaveOutput, buf.data(), buf.size());
        if (sz > 0)
        {
            if (!_tup_p->sendData(buf))
            {
                qDebug() << "Slave failed to send the app data";
            }
        }
        else
        {
            qDebug() << "Slave failed to encode the app data";
        }
    }
    else
    {
        qDebug() << "Slave failed to get the app data size";
    }
}
