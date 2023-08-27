#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "master_form.h"
#include "slave_form.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void butMasterFormClicked(bool checked);
    void butSlaveFormClicked(bool checked);

private:
    Ui::MainWindow *ui;

    MasterForm* _masterForm_p = nullptr;
    SlaveForm* _slaveForm_p = nullptr;
};
#endif // MAINWINDOW_H
