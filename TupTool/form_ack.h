#ifndef FORM_ACK_H
#define FORM_ACK_H

#include <QWidget>

#include "common.h"

namespace Ui {
class AckForm;
}

class AckForm : public QWidget
{
    Q_OBJECT

public:
    explicit AckForm(QWidget *parent = nullptr);
    ~AckForm();

    PAckFrame getFrame() const;

public slots:
    void setFrame(PAckFrame value_p);

private:
    Ui::AckForm *ui;
};

#endif // FORM_ACK_H
