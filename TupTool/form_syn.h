#ifndef FORM_SYN_H
#define FORM_SYN_H

#include <QWidget>

#include "common.h"

namespace Ui {
class SynForm;
}

class SynForm : public QWidget
{
    Q_OBJECT

public:
    explicit SynForm(QWidget *parent = nullptr);
    ~SynForm();

    PSynFrame getFrame() const;

public slots:
    void setFrame(PSynFrame value_p);

private:
    Ui::SynForm *ui;
};

#endif // FORM_SYN_H
