#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QSerialPort>

#include "coder_widget.h"
#include "transfer_widget.h"
#include "instance_widget.h"

namespace Ui {
class InstanceForm;
}

class InstanceForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit InstanceForm(QWidget *parent = nullptr);
    ~InstanceForm();

private slots:
    void butRefreshClicked(bool checked);
    void butOpenCloseClicked(bool checked);
    void tabWidgetCurrentChanged(int index);

private:
    void initPortList(QComboBox& comboBox);
    void initBaudratesList(QComboBox& comboBox);

    Ui::InstanceForm *ui;

    CoderWidget* _coder_p = nullptr;
    TransferWidget* _transfer_p = nullptr;
    InstanceWidget* _instance_p = nullptr;

    QSerialPort _port;
};
