#ifndef FORM_DATA_H
#define FORM_DATA_H

#include <QWidget>

#include "common.h"

namespace Ui {
class DataForm;
}

class DataForm : public QWidget
{
    Q_OBJECT

public:
    explicit DataForm(QWidget *parent = nullptr);
    ~DataForm();

    PDataFrame getFrame() const;

public slots:
    void setFrame(PDataFrame value_p);

private:
    Ui::DataForm *ui;
};

#endif // FORM_DATA_H
