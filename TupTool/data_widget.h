#ifndef FORM_DATA_H
#define FORM_DATA_H

#include <QWidget>

#include "common.h"

namespace Ui {
class DataWidget;
}

class DataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataWidget(QWidget *parent = nullptr);
    ~DataWidget();

    PDataFrame getFrame() const;

public slots:
    void setFrame(PDataFrame value_p);

private:
    Ui::DataWidget *ui;
};

#endif // FORM_DATA_H
