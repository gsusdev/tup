#include "form_fin.h"
#include "ui_form_fin.h"

#include "utils.h"

FinForm::FinForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FinForm)
{
    ui->setupUi(this);

    initCopList(*ui->cbCop);
}

FinForm::~FinForm()
{
    delete ui;
}

PFinFrame FinForm::getFrame() const
{
    auto ptr = PFinFrame(new tup_v1_fin_t());

    ptr->j = ui->sbJ->value();
    ptr->cop = static_cast<tup_v1_cop_t>(ui->cbCop->currentData().toUInt());

    return ptr;
}

void FinForm::setFrame(PFinFrame value_p)
{
    ui->sbJ->setValue(value_p->j);
    ui->cbCop->setCurrentIndex(ui->cbCop->findData(value_p->cop));
}
