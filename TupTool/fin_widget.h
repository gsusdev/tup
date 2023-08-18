#ifndef FORM_FIN_H
#define FORM_FIN_H

#include <QWidget>

#include "common.h"

namespace Ui {
class FinWidget;
}

class FinWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FinWidget(QWidget *parent = nullptr);
    ~FinWidget();

    PFinFrame getFrame() const;

public slots:
    void setFrame(PFinFrame value_p);

private:
    Ui::FinWidget *ui;
};

#endif // FORM_FIN_H
