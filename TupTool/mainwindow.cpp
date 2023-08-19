#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSerialPortInfo>
#include <QMessageBox>
#include <QMetaEnum>

#include "common.h"
#include "instance_form.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{    
    qRegisterMetaType<PAckFrame>("PAckFrame");
    qRegisterMetaType<PSynFrame>("PSynFrame");
    qRegisterMetaType<PFinFrame>("PFinFrame");
    qRegisterMetaType<PDataFrame>("PDataFrame");

    qRegisterMetaType<quintptr>("quintptr");



    ui->setupUi(this);      

    connect(ui->butNewInstance, &QPushButton::clicked, this, &MainWindow::butNewInstanceClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::logText(QString text)
{
    ui->pteLog->appendPlainText(text);    
}

void MainWindow::logTextWoReturn(QString text)
{
    ui->pteLog->moveCursor(QTextCursor::End);
    ui->pteLog->insertPlainText(text);
    ui->pteLog->moveCursor(QTextCursor::End);
}

void MainWindow::butNewInstanceClicked(bool checked)
{
    (void)checked;

    auto instance_p = new InstanceForm(this);
    instance_p->show();
}


