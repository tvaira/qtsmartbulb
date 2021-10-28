#ifndef SMARTBULB_H
#define SMARTBULB_H

// cf. https://github.com/vpaeder/telinkpp/

#include <QtSmartBulb/qsmartbulbglobal.h>

#include <QtCore/QIODevice>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QSharedPointer>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QLowEnergyController>

//#define DEBUG

#define Q_SMARTBULB_SERVICE_UUID                  QString::fromUtf8("{00010203-0405-0607-0809-0a0b0c0d1910}")
#define Q_SMARTBULB_CHARACTERISTIC_UUID           QString::fromUtf8("{00010203-0405-0607-0809-0a0b0c0d1912}")
#define Q_SMARTBULB_CHARACTERISTIC_CONFIGURATION  QString::fromUtf8("{00010203-0405-0607-0809-0a0b0c0d1911}")
#define Q_SMARTBULB_CHARACTERISTIC_PAIR           QString::fromUtf8("{00010203-0405-0607-0809-0a0b0c0d1914}")

// Command codes
#define COMMAND_OTA_UPDATE            0xC6
#define COMMAND_QUERY_OTA_STATE       0xC7
#define COMMAND_OTA_STATUS_REPORT     0xC8
#define COMMAND_LIGHT_ON_OFF          0xD0
#define COMMAND_GROUP_ID_QUERY        0xDD
#define COMMAND_GROUP_ID_REPORT       0xD4
#define COMMAND_GROUP_EDIT            0xD7
#define COMMAND_ONLINE_STATUS_REPORT  0xDC
#define COMMAND_ADDRESS_EDIT          0xE0
#define COMMAND_ADDRESS_REPORT        0xE1
#define COMMAND_LIGHT_ATTRIBUTES_SET  0xE2
#define COMMAND_RESET                 0xE3
#define COMMAND_TIME_QUERY            0xE8
#define COMMAND_TIME_REPORT           0xE9
#define COMMAND_TIME_SET              0xE4
#define COMMAND_DEVICE_INFO_QUERY     0xEA
#define COMMAND_DEVICE_INFO_REPORT    0xEB

#define schar(x) static_cast<char>(x)

QT_BEGIN_NAMESPACE

class Q_SMARTBULB_EXPORT SmartBulb : public QObject
{
public:
    enum BulbState
    {
        ON = 0x00,
        OFF = 0xff
    };

private:
    Q_OBJECT
    Q_ENUMS(BulbState)
    Q_PROPERTY(bool smartBulbDetecte MEMBER m_smartBulbDetecte NOTIFY detecteUpdated)
    Q_PROPERTY(bool etatRecherche MEMBER m_etatRecherche NOTIFY rechercheUpdated)
    Q_PROPERTY(bool etatConnexion MEMBER m_etatConnexion NOTIFY connecteUpdated)
    Q_PROPERTY(bool connexionErreur MEMBER m_connexionErreur NOTIFY erreurUpdated)

public:
    explicit SmartBulb(QString adresseMAC, QString nom, QString password=QString::fromUtf8("password"), QObject *parent = nullptr);
    ~SmartBulb();
    Q_INVOKABLE void rechercher();
    Q_INVOKABLE void arreter();
    Q_INVOKABLE void connecter();
    Q_INVOKABLE void deconnecter();
    Q_INVOKABLE void allumer();
    Q_INVOKABLE void eteindre();
    Q_INVOKABLE void commanderRGB(int rouge, int vert, int bleu);
    Q_INVOKABLE void commander(int blanc);
    Q_INVOKABLE void reset();
    Q_INVOKABLE void lireEtat();

    bool etatConnexion() const;
    bool etatRecherche() const;
    bool connexionErreur() const;
    bool smartBulbDetecte() const;
    bool estConnecte() const;
    bool estDetecte() const;

Q_SIGNALS:
    void connecteUpdated();
    void rechercheUpdated();
    void erreurUpdated();
    void detecteUpdated();
    void smartBulbUpdated();
    void etatOnlineUpdated();
    void etatChanged(const QByteArray &value);

public Q_SLOTS:

private Q_SLOTS:
    void ajouterSmartBulb(const QBluetoothDeviceInfo&);
    void rechercheTerminee();
    void rechercheErreur(QBluetoothDeviceDiscoveryAgent::Error);
    void ajouterService(QBluetoothUuid serviceUuid);
    void smartBulbConnecte();
    void smartBulbDeconnecte();
    void connecteErreur(QLowEnergyController::Error);
    void serviceCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void characteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void characteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void serviceDetailsDiscovered(QLowEnergyService::ServiceState newState);

private:
    int                              m_vendor;
    int                              m_meshId;
    int                              m_packetCount;
    QBluetoothDeviceDiscoveryAgent  *m_discoveryAgent;
    QLowEnergyController            *m_controller;
    QLowEnergyService               *m_service;
    QLowEnergyCharacteristic         m_characteristicCommande;
    QLowEnergyCharacteristic         m_characteristicConfiguration;
    QLowEnergyCharacteristic         m_characteristicPair;
    bool                             m_etatConnexion;
    bool                             m_etatRecherche;
    bool                             m_connexionErreur;
    bool                             m_smartBulbDetecte;
    bool                             m_etatOnline;
    QString                          m_nomRecherche;
    QString                          m_adresseMACRecherche;
    // cf. https://github.com/vpaeder/telinkpp/
    std::string                      reverse_address;
    std::string                      shared_key;
    std::string                      address;
    std::string                      name;
    std::string                      password;
    std::string                      data;

    void connecterSmartBulb();
    void gererNotification(bool notification);
    void writePair(const QByteArray &data);
    void write(const QByteArray &data, bool reponse=false);
    // cf. https://github.com/vpaeder/telinkpp/
    bool appairer();
    std::string combine_name_and_password() const;
    void generate_shared_key(const std::string & data1, const std::string & data2);
    std::string key_encrypt(std::string & key) const;
    std::string encrypt_packet(std::string & packet) const;
    std::string decrypt_packet(std::string & packet) const;
    std::string build_packet(int command, const std::string & data);
    void send_packet(int command, const std::string & data);
};

QT_END_NAMESPACE

#endif // #ifndef SMARTBULB_H
