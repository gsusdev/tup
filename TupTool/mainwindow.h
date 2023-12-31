#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <cstdint>
#include <cstdbool>

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void logText(QString text);
    void logTextWoReturn(QString text);

private slots:
    void butNewInstanceClicked(bool checked);


private:   
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
