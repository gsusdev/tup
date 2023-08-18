#include "syn_widget.h"
#include "ui_syn_widget.h"

#include "utils.h"

SynWidget::SynWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SynWidget)
{
    ui->setupUi(this);

    initCopList(*ui->cbCop, tup_v1_cop_syn);
}

SynWidget::~SynWidget()
{
    delete ui;
}

PSynFrame SynWidget::getFrame() const
{
    auto ptr = PSynFrame(new tup_v1_syn_t());

    ptr->j = ui->leJ->text().toULong(nullptr, 0);
    ptr->cop = static_cast<tup_v1_cop_t>(ui->cbCop->currentData().toUInt());
    ptr->windowSize = ui->leWindowSize->text().toULong(nullptr, 0);

    return ptr;
}

void SynWidget::setFrame(PSynFrame value_p)
{
    ui->leJ->setText(QString::number(value_p->j));
    ui->cbCop->setCurrentIndex(ui->cbCop->findData(value_p->cop));
    ui->leWindowSize->setText(QString::number(value_p->windowSize));
}
