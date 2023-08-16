#include "form_data.h"
#include "ui_form_data.h"

DataForm::DataForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataForm)
{
    ui->setupUi(this);
}

DataForm::~DataForm()
{
    delete ui;
}

PDataFrame DataForm::getFrame() const
{
    auto ptr = PDataFrame(new DataFrame());

    ptr->j = ui->sbJ->value();
    ptr->cop = static_cast<tup_v1_cop_t>(ui->cbCop->currentData().toUInt());
    ptr->isFinal = ui->chkFinal->checkState() == Qt::CheckState::Checked;
    ptr->payload = ui->lePayload->text().toUtf8();

    return ptr;
}

void DataForm::setFrame(PDataFrame value_p)
{
    ui->sbJ->setValue(value_p->j);
    ui->cbCop->setCurrentIndex(ui->cbCop->findData(value_p->cop));

    const auto checkState = value_p->isFinal ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
    ui->chkFinal->setCheckState(checkState);

    ui->lePayload->setText(QString::fromUtf8(value_p->payload));
}
