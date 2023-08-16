#ifndef FORM_FIN_H
#define FORM_FIN_H

#include <QWidget>

#include "common.h"

namespace Ui {
class FinForm;
}

class FinForm : public QWidget
{
    Q_OBJECT

public:
    explicit FinForm(QWidget *parent = nullptr);
    ~FinForm();

    PFinFrame getFrame() const;

public slots:
    void setFrame(PFinFrame value_p);

private:
    Ui::FinForm *ui;
};

#endif // FORM_FIN_H
