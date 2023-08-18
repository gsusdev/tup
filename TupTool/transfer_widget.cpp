#include "transfer_widget.h"
#include "ui_transfer_widget.h"

TransferWidget::TransferWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransferWidget)
{
    ui->setupUi(this);
}

TransferWidget::~TransferWidget()
{
    delete ui;
}
