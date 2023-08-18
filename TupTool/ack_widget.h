#ifndef FORM_ACK_H
#define FORM_ACK_H

#include <QWidget>

#include "common.h"

namespace Ui {
class AckWidget;
}

class AckWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AckWidget(QWidget *parent = nullptr);
    ~AckWidget();

    PAckFrame getFrame() const;

public slots:
    void setFrame(PAckFrame value_p);

private:
    Ui::AckWidget *ui;
};

#endif // FORM_ACK_H
