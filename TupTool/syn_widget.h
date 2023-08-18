#ifndef FORM_SYN_H
#define FORM_SYN_H

#include <QWidget>

#include "common.h"

namespace Ui {
class SynWidget;
}

class SynWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SynWidget(QWidget *parent = nullptr);
    ~SynWidget();

    PSynFrame getFrame() const;

public slots:
    void setFrame(PSynFrame value_p);

private:
    Ui::SynWidget *ui;
};

#endif // FORM_SYN_H
