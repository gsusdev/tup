#include "form_syn.h"
#include "ui_form_syn.h"

#include "utils.h"

SynForm::SynForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SynForm)
{
    ui->setupUi(this);

    initCopList(*ui->cbCop, tup_v1_cop_syn);
}

SynForm::~SynForm()
{
    delete ui;
}

PSynFrame SynForm::getFrame() const
{
    auto ptr = PSynFrame(new tup_v1_syn_t());

    ptr->j = ui->leJ->text().toULong(nullptr, 0);
    ptr->cop = static_cast<tup_v1_cop_t>(ui->cbCop->currentData().toUInt());
    ptr->windowSize = ui->leWindowSize->text().toULong(nullptr, 0);

    return ptr;
}

void SynForm::setFrame(PSynFrame value_p)
{
    ui->leJ->setText(QString::number(value_p->j));
    ui->cbCop->setCurrentIndex(ui->cbCop->findData(value_p->cop));
    ui->leWindowSize->setText(QString::number(value_p->windowSize));
}
