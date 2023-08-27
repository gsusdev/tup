#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->butMasterForm, &QPushButton::clicked, this, &MainWindow::butMasterFormClicked);
    connect(ui->butSlaveForm, &QPushButton::clicked, this, &MainWindow::butSlaveFormClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::butMasterFormClicked(bool checked)
{
    (void)checked;

    if (_masterForm_p == nullptr)
    {
        _masterForm_p = new MasterForm(this);
    }

    _masterForm_p->show();
    if (_masterForm_p->windowState() == Qt::WindowMinimized)
    {
        _masterForm_p->showNormal();
    }
}

void MainWindow::butSlaveFormClicked(bool checked)
{
    (void)checked;

    if (_slaveForm_p == nullptr)
    {
        _slaveForm_p = new SlaveForm(this);
    }

    _slaveForm_p->show();
}

