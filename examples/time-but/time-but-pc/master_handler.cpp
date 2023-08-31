#include "master_handler.h"

#include <algorithm>

#include <QDebug>

MasterHandler::MasterHandler(QObject *parent) : QObject(parent)
{
    _isBusy = false;
}

void MasterHandler::setTup(TupWrapper* tup_p)
{
    if (_tup_p != nullptr)
    {
        disconnect(_tup_p, &TupWrapper::onReceiveData, this, &MasterHandler::onReceiveData);
        disconnect(_tup_p, &TupWrapper::onResultSent, this, &MasterHandler::onResultSent);
    }

    _tup_p = tup_p;

    if (_tup_p != nullptr)
    {
        connect(_tup_p, &TupWrapper::onReceiveData, this, &MasterHandler::onReceiveData);
        connect(_tup_p, &TupWrapper::onResultSent, this, &MasterHandler::onResultSent);
    }
}

void MasterHandler::sendData()
{
    if (_tup_p == nullptr)
    {
        return;
    }

    if (!_tup_p->isConnected())
    {
        qDebug() << "Master not connected";
        return;
    }

    bool val = false;
    if (!_isBusy.compare_exchange_strong(val, true))
    {
        qDebug() << "Master is busy";
        return;
    }

    _isDataReceived = false;

    auto sz = app_protocol_encodeMasterOutput(&_masterOutput, nullptr, 0);
    if (sz > 0)
    {
        QByteArray buf(sz, 0);
        sz = app_protocol_encodeMasterOutput(&_masterOutput, buf.data(), buf.size());
        if (sz > 0)
        {
            if (!_tup_p->sendData(buf))
            {
                qDebug() << "Master failed to send the app data";
            }
        }
        else
        {
            qDebug() << "Master failed to encode the app data";
        }
    }
    else
    {
        qDebug() << "Master failed to get the app data size";
    }
}

void MasterHandler::reset()
{
    _isBusy = false;
    _isDataReceived = false;
}

void MasterHandler::onReceiveData(QByteArray data, quint8 isFinal)
{
    if (_tup_p == nullptr)
    {
        return;
    }

    _inputBuf.append(data);
    if (_inputBuf.size() > APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES)
    {
        _inputBuf.clear();
        emit sigOnBadFrame();

        _tup_p->setResult(TUP_ERROR_NOMEMORY);
        return;
    }

    if (!isFinal)
    {
        return;
    }

    _isDataReceived = true;

    if (_inputBuf.size() != APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES)
    {
        _inputBuf.clear();
        emit sigOnBadFrame();

        _tup_p->setResult(TUP_ERROR_LEN);
        return;
    }

    app_protocol_slaveOutputData_t decodedData;
    if (app_protocol_decodeSlaveOutput(_inputBuf.data(), _inputBuf.size(), &decodedData))
    {
        _slaveOutput = decodedData;
        emit sigOnUpdated();

        _tup_p->setResult(TUP_OK);
    }
    else
    {
        emit sigOnBadFrame();
        _tup_p->setResult(TUP_ERROR_UNKNOWN);
    }

    _inputBuf.clear();
}

void MasterHandler::onResultSent()
{
    if (_isDataReceived)
    {
        _isBusy = false;
        _isDataReceived = false;
    }
}
