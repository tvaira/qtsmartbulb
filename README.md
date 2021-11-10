# Module Qt5 pour Smart Bulb (Telink mesh)

Ce module Qt5 fournit la classe `SmartBulb` pour gérer des ampoules Bluetooth LE avec le protocole [Telink mesh](http://wiki.telink-semi.cn/wiki/protocols/Telink-Mesh/). C'est un (rapide et partiel) portage pour Qt des classes C++ [telinkpp](https://github.com/vpaeder/telinkpp/).

## Installation

Le module `QtSmartBulb` dépend l’API Qt Bluetooth Low Energy. Elle a été introduite dans Qt 5.4. Depuis Qt 5.5, cette partie de l’API est définitive et une garantie de compatibilité est donnée pour les versions futures. Depuis Qt 5.7, une API supplémentaire prenant en charge le rôle de périphérique a été ajoutée, avec le backend implémenté pour Linux/BlueZ, iOS et macOS.

Il faut donc au moins Qt 5.7 :

```sh
$ sudo apt-get install libqt5bluetooth5 libbluetooth-dev qtconnectivity5-dev qtconnectivity5-examples
```

Télécharger le code source du module Qt5 :

```sh
$ wget -c https://github.com/tvaira/qtsmartbulb/archive/refs/heads/main.zip
```

Ou cloner le projet :

```sh
$ git clone https://github.com/tvaira/qtsmartbulb.git
```

Puis :

```sh
$ cd qtsmartbulb/

$ qmake

$ make

$ sudo make install
```

_Remarques :_

- vous pouvez modifier la version de Qt dans `.qmake.conf`
- vous pouvez préciser le chemin de `qmake` si besoin
- il faut `openssl` et ses bibliothèques `-lssl -ldl -lcrypto` (`sudo apt install libssl-dev libboost-all-dev`)

## Utilisation

Voir : [Exemple](https://github.com/tvaira/qtsmartbulb/tree/main/examples/)

Pour utiliser le module Qt `SmartBulb`, il faudra commencer par l'ajouter dans le fichier de projet `.pro` :

```
QT += smartbulb
```

L'ampoule se pilote à partir d'un objet de la classe `SmartBulb` :

```cpp
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
```

Quelques exemples :

```cpp
#include "mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    smartBulb = new SmartBulb(QString::fromUtf8("ff:ff:00:00:2b:b1"), QString::fromUtf8("Bulb2BB1"), QString::fromUtf8("password"), this);
    connect(smartBulb, SIGNAL(connecteUpdated()), this, SLOT(connecte()));
    connect(smartBulb, SIGNAL(rechercheUpdated()), this, SLOT(recherche()));
    connect(smartBulb, SIGNAL(etatOnlineUpdated()), this, SLOT(commande()));
    connect(smartBulb, SIGNAL(erreurUpdated()), this, SLOT(erreur()));
    connect(smartBulb, SIGNAL(detecteUpdated()), this, SLOT(detecte()));
    connect(smartBulb, SIGNAL(smartBulbUpdated()), this, SLOT(smartBulbUpdated()));

    //smartBulb->rechercher(); // cf. smartBulbUpdated()
    // ou directement :
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
```

_Remarques :_

Les paramètres d'usine proposés par Telink sont `telink_mesh1` et `123`, mais ils auront probablement été modifiés par le fabricant de votre appareil. Par exemple pour Zengge, les paramètres sont `ZenggeMesh` et `ZenggeTechnology`. On retrouve ces paramètres après un `reset()`.

## Voir aussi

- [Mise en oeuvre du Bluetooth BLE sous Qt](http://tvaira.free.fr/bts-sn/activites/activite-ble/activite-ble-qt.html#bluetooth-le-et-qt5)
- [C++ classes with Telink mesh protocol](https://github.com/vpaeder/telinkpp/)
- [ZNEGGE SDK](https://github.com/ZNEGGE-SDK)
- [Module Qt5 pour Magic Blue LED](https://github.com/tvaira/qtmagicblueled)

---
2021 Thierry Vaira : **[tvaira(at)free(dot)fr](tvaira@free.fr)**
