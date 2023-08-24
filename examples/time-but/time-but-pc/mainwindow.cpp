#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "master_form.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->butMasterForm, &QPushButton::clicked, this, &MainWindow::butMasterFormClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::butMasterFormClicked(bool checked)
{
    (void)checked;
    (new MasterForm(this))->show();
}

