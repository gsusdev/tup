#ifndef PORT_WIDGET_H
#define PORT_WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QComboBox>

namespace Ui {
class PortWidget;
}

class PortWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PortWidget(QWidget *parent = nullptr);
    ~PortWidget();

    void setPort(QSerialPort* port_p) { _port_p = port_p; }

signals:
    void sigOpened();
    void sigClosed();

private slots:
    void butOpenCloseClicked(bool checked);

private:
    void initBaudratesList(QComboBox& comboBox);
    void initPortList(QComboBox& comboBox);

    Ui::PortWidget *ui;

    QSerialPort* _port_p = nullptr;
};

#endif // PORT_WIDGET_H
