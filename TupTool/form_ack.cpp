#include "form_ack.h"
#include "ui_form_ack.h"

#include "utils.h"

AckForm::AckForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AckForm)
{
    ui->setupUi(this);

    initCopList(*ui->cbCop, tup_v1_cop_ack);
    initErrorList(*ui->cbError);
}

AckForm::~AckForm()
{
    delete ui;
}

PAckFrame AckForm::getFrame() const
{
    auto ptr = PAckFrame(new tup_v1_ack_t());

    ptr->j = ui->leJ->text().toULong(nullptr, 0);
    ptr->cop = static_cast<tup_v1_cop_t>(ui->cbCop->currentData().toUInt());
    ptr->error = static_cast<tup_transfer_result_t>(ui->cbError->currentData().toUInt());

    return ptr;
}

void AckForm::setFrame(PAckFrame value_p)
{
    ui->leJ->setText(QString::number(value_p->j));
    ui->cbCop->setCurrentIndex(ui->cbCop->findData(value_p->cop));
    ui->cbError->setCurrentIndex(ui->cbError->findData(value_p->error));
}
