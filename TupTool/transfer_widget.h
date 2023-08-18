#ifndef TRANSFER_WIDGET_H
#define TRANSFER_WIDGET_H

#include <QWidget>

namespace Ui {
class TransferWidget;
}

class TransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransferWidget(QWidget *parent = nullptr);
    ~TransferWidget();

private:
    Ui::TransferWidget *ui;
};

#endif // TRANSFER_WIDGET_H
