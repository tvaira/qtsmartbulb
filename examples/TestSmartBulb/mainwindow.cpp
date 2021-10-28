#include "mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    smartBulb = new SmartBulb(QString::fromUtf8("ff:ff:00:00:2b:b1"), QString::fromUtf8("Bulb2BB1"), QString::fromUtf8("password"), this);
    connect(smartBulb, SIGNAL(connecteUpdated()), this, SLOT(connecte()));
    connect(smartBulb, SIGNAL(rechercheUpdated()), this, SLOT(recherche()));
    connect(smartBulb, SIGNAL(etatOnlineUpdated()), this, SLOT(commande()));
    connect(smartBulb, SIGNAL(erreurUpdated()), this, SLOT(erreur()));
    connect(smartBulb, SIGNAL(detecteUpdated()), this, SLOT(detecte()));
    connect(smartBulb, SIGNAL(smartBulbUpdated()), this, SLOT(smartBulbUpdated()));

    //smartBulb->rechercher(); // cf. smartBulbUpdated()
    // ou :
    smartBulb->connecter();
}

MainWindow::~MainWindow()
{
    if(smartBulb->estConnecte())
        smartBulb->eteindre();
    qDebug() << Q_FUNC_INFO;
}

void MainWindow::connecte()
{
    qDebug() << Q_FUNC_INFO << smartBulb->etatConnexion() << smartBulb->estConnecte();
}

void MainWindow::recherche()
{
    qDebug() << Q_FUNC_INFO << smartBulb->etatRecherche();
}

void MainWindow::commande()
{
    qDebug() << Q_FUNC_INFO << smartBulb->estConnecte();
    if(smartBulb->estConnecte())
    {
        // Exemples :
        smartBulb->commanderRGB(50, 127, 0); // en couleur
        //smartBulb->commander(10); // en blanc
        smartBulb->allumer();
    }
}

void MainWindow::erreur()
{
    if(smartBulb->connexionErreur())
        qDebug() << Q_FUNC_INFO;
}

void MainWindow::detecte()
{
    qDebug() << Q_FUNC_INFO << smartBulb->smartBulbDetecte();
}

void MainWindow::smartBulbUpdated()
{
    qDebug() << Q_FUNC_INFO << smartBulb->estDetecte();
    if(smartBulb->estDetecte() && !smartBulb->estConnecte())
    {
        smartBulb->connecter();
    }
}
