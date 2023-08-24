#include "master_handler.h"

#include <algorithm>

#include <QDebug>

MasterHandler::MasterHandler(QObject *parent) : QObject(parent)
{

}

void MasterHandler::setTup(TupWrapper* tup_p)
{
    if (_tup_p != nullptr)
    {
        disconnect(_tup_p, &TupWrapper::onReceiveData, this, &MasterHandler::onReceiveData);
    }

    _tup_p = tup_p;

    if (_tup_p != nullptr)
    {
        connect(_tup_p, &TupWrapper::onReceiveData, this, &MasterHandler::onReceiveData);
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
    }

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

void MasterHandler::onReceiveData(QByteArray data, quint8 isFinal)
{
    _inputBuf.append(data);
    if (_inputBuf.size() > APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES)
    {
        _inputBuf.clear();
        emit sigOnBadFrame();

        return;
    }

    if (!isFinal)
    {
        return;
    }

    if (_inputBuf.size() != APP_PROTOCOL_SLAVE_MESSAGE_SIZE_BYTES)
    {
        _inputBuf.clear();
        emit sigOnBadFrame();

        return;
    }

    app_protocol_slaveOutputData_t decodedData;
    if (app_protocol_decodeSlaveOutput(_inputBuf.data(), _inputBuf.size(), &decodedData))
    {
        _slaveOutput = decodedData;
        emit sigOnUpdated();
    }
    else
    {
        emit sigOnBadFrame();
    }

    _inputBuf.clear();
}
