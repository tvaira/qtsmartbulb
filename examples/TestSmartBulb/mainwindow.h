#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSmartBulb/QSmartBulb>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    SmartBulb *smartBulb;

public slots:
    void connecte();
    void recherche();
    void commande();
    void erreur();
    void detecte();
    void smartBulbUpdated();
};

#endif // MAINWINDOW_H
