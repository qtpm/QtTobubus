#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qttobubus/qtobubushost.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QTobubusHost* _host;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_OpenButton_clicked();
    void on_SendButton_clicked();
    void on_methodResult(QString path, QString method, QVariantList result, Call::Status status);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
