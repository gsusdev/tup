#include "fin_widget.h"
#include "ui_fin_widget.h"

#include "utils.h"

FinWidget::FinWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FinWidget)
{
    ui->setupUi(this);

    initCopList(*ui->cbCop, tup_v1_cop_fin);
}

FinWidget::~FinWidget()
{
    delete ui;
}

PFinFrame FinWidget::getFrame() const
{
    auto ptr = PFinFrame(new tup_v1_fin_t());

    ptr->j = ui->leJ->text().toULong(nullptr, 0);
    ptr->cop = static_cast<tup_v1_cop_t>(ui->cbCop->currentData().toUInt());

    return ptr;
}

void FinWidget::setFrame(PFinFrame value_p)
{
    ui->leJ->setText(QString::number(value_p->j));
    ui->cbCop->setCurrentIndex(ui->cbCop->findData(value_p->cop));
}
