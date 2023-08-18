#include "data_widget.h"
#include "ui_data_widget.h"

#include "utils.h"

DataWidget::DataWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataWidget)
{
    ui->setupUi(this);

    initCopList(*ui->cbCop, tup_v1_cop_data);
}

DataWidget::~DataWidget()
{
    delete ui;
}

PDataFrame DataWidget::getFrame() const
{
    auto ptr = PDataFrame(new DataFrame());

    ptr->j = ui->leJ->text().toULong(nullptr, 0);
    ptr->cop = static_cast<tup_v1_cop_t>(ui->cbCop->currentData().toUInt());
    ptr->isFinal = ui->chkFinal->checkState() == Qt::CheckState::Checked;
    ptr->payload = ui->lePayload->text().toUtf8();

    return ptr;
}

void DataWidget::setFrame(PDataFrame value_p)
{
    ui->leJ->setText(QString::number(value_p->j));
    ui->cbCop->setCurrentIndex(ui->cbCop->findData(value_p->cop));

    const auto checkState = value_p->isFinal ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    ui->chkFinal->setCheckState(checkState);

    ui->lePayload->setText(QString::fromUtf8(value_p->payload));
}
