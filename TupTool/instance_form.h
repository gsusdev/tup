#ifndef INSTANCE_FORM_H
#define INSTANCE_FORM_H

#include <QMainWindow>
#include <QComboBox>
#include <QSerialPort>

#include "coder_widget.h"

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

private:
    void initPortList(QComboBox& comboBox);
    void initBaudratesList(QComboBox& comboBox);

    Ui::InstanceForm *ui;

    CoderWidget* _coder_p = nullptr;

    QSerialPort _port;
};

#endif // INSTANCE_FORM_H
