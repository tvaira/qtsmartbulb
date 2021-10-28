#include "qsmartbulb.h"

#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtCore/QtEndian>
#include <QtCore/QVector>
#include <QtCore/QString>

#include <functional>
#include <algorithm>
#include <thread>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <exception>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <boost/algorithm/string.hpp>

QT_BEGIN_NAMESPACE

SmartBulb::SmartBulb(QString adresseMAC, QString nom, QString password, QObject *parent) : QObject(parent), m_vendor(0x211), m_meshId(0), m_packetCount(1), m_discoveryAgent(nullptr), m_controller(nullptr), m_service(nullptr), m_etatConnexion(false), m_etatRecherche(false), m_connexionErreur(false), m_smartBulbDetecte(false), m_etatOnline(false)
{
    this->address = adresseMAC.toStdString();
    this->name = nom.toStdString();
    this->name.append(16-name.size(), 0);
    this->password = password.toStdString();
    this->password.append(16 - password.size(), 0);
    m_nomRecherche = nom;
    m_adresseMACRecherche = adresseMAC;
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent();
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(5000);
    // Slot pour la recherche d'appareils BLE
    connect(m_discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)), this, SLOT(ajouterSmartBulb(QBluetoothDeviceInfo)));
    connect(m_discoveryAgent, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)), this, SLOT(rechercheErreur(QBluetoothDeviceDiscoveryAgent::Error)));
    connect(m_discoveryAgent, SIGNAL(finished()), this, SLOT(rechercheTerminee()));
}

SmartBulb::~SmartBulb()
{
    if(m_controller)
        m_controller->disconnectFromDevice();
    delete m_controller;
}

void SmartBulb::rechercher()
{
    m_smartBulbDetecte = false;
    emit detecteUpdated();
    emit smartBulbUpdated();

    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    if(m_discoveryAgent->isActive())
    {
        m_etatRecherche = true;
        emit rechercheUpdated();
    }
}

void SmartBulb::arreter()
{
    if(m_etatRecherche && m_discoveryAgent->isActive())
    {
        m_discoveryAgent->stop();
    }
}

void SmartBulb::connecter()
{
    this->connecterSmartBulb();
}

void SmartBulb::deconnecter()
{
    if(m_controller)
        m_controller->disconnectFromDevice();
}

void SmartBulb::allumer()
{
    std::string datas = {1, 1, schar(255), 0, 0, 0, 0, 3, 0, 0};
    this->send_packet(COMMAND_LIGHT_ON_OFF, datas);
}

void SmartBulb::eteindre()
{
    std::string datas = {1, 1, 0, 0, 0, 0, 0, 3, 0, 0};
    this->send_packet(COMMAND_LIGHT_ON_OFF, datas);
}

void SmartBulb::commanderRGB(int rouge, int vert, int bleu)
{
    this->send_packet(COMMAND_LIGHT_ATTRIBUTES_SET, {1, 0x60, schar(rouge), schar(vert), schar(bleu), 0, 0, 2, 0});
}

void SmartBulb::commander(int blanc)
{
    std::string datas = {1, 0x61, schar(blanc%101), 0, 0, 0, 0, 2, 0};
    this->send_packet(COMMAND_LIGHT_ATTRIBUTES_SET, datas);
}

void SmartBulb::reset()
{
    std::string datas = {1,0,0,0,0,0,0,0,0,0};
    this->send_packet(COMMAND_RESET, datas);
}

void SmartBulb::lireEtat()
{

}

bool SmartBulb::etatConnexion() const
{
    return m_etatConnexion;
}

bool SmartBulb::etatRecherche() const
{
    return m_etatRecherche;
}

bool SmartBulb::connexionErreur() const
{
    return m_connexionErreur;
}

bool SmartBulb::smartBulbDetecte() const
{
    return m_smartBulbDetecte;
}

bool SmartBulb::estConnecte() const
{
    return m_etatOnline;
}

bool SmartBulb::estDetecte() const
{
    return m_smartBulbDetecte;
}

void SmartBulb::ajouterSmartBulb(const QBluetoothDeviceInfo &info)
{
    // Bluetooth Low Energy ?
    if(info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
        //qDebug() << Q_FUNC_INFO << info.name() << info.address().toString();
        // Magic Blue ?
        if( (!m_nomRecherche.isEmpty() && info.name().startsWith(m_nomRecherche)) || (!m_adresseMACRecherche.isEmpty() && info.address().toString() == m_adresseMACRecherche) )
        {
            m_smartBulbDetecte = true;
        }
    }
}

void SmartBulb::rechercheTerminee()
{
    m_etatRecherche = false;
    emit rechercheUpdated();
    emit detecteUpdated();
    emit smartBulbUpdated();
}

void SmartBulb::rechercheErreur(QBluetoothDeviceDiscoveryAgent::Error erreur)
{
    Q_UNUSED(erreur)
    m_etatRecherche = false;
    emit rechercheUpdated();
    emit detecteUpdated();
    emit smartBulbUpdated();
}

void SmartBulb::connecterSmartBulb()
{
    m_controller =  new QLowEnergyController(QBluetoothAddress(m_adresseMACRecherche), this);

    // Slot pour la récupération des services
    connect(m_controller, SIGNAL(serviceDiscovered(QBluetoothUuid)), this, SLOT(ajouterService(QBluetoothUuid)));
    connect(m_controller, SIGNAL(connected()), this, SLOT(smartBulbConnecte()));
    connect(m_controller, SIGNAL(disconnected()), this, SLOT(smartBulbDeconnecte()));
    connect(m_controller, SIGNAL(error(QLowEnergyController::Error)), this, SLOT(connecteErreur(QLowEnergyController::Error)));

    m_connexionErreur = false;
    emit erreurUpdated();
    m_controller->setRemoteAddressType(QLowEnergyController::PublicAddress);
    m_controller->connectToDevice();
}

void SmartBulb::ajouterService(QBluetoothUuid serviceUuid)
{
    //qDebug() << Q_FUNC_INFO << serviceUuid.toString();
    if(serviceUuid.toString() == Q_SMARTBULB_SERVICE_UUID)
    {
        m_service = m_controller->createServiceObject(serviceUuid);
        if(m_service->state() == QLowEnergyService::DiscoveryRequired)
        {
            // Slot pour la récupération des caractéristiques
            connect(m_service, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(serviceDetailsDiscovered(QLowEnergyService::ServiceState)));

            m_etatOnline = false;
            m_service->discoverDetails();
        }
    }
}

void SmartBulb::serviceCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << c.uuid().toString() << value;
}

void SmartBulb::characteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    //qDebug() << Q_FUNC_INFO << c.uuid().toString() << m_etatServicePair << value.size() << value;
    if(c.uuid().toString() == Q_SMARTBULB_CHARACTERISTIC_PAIR)
    {
        if(value.size() > 1)
        {
            // generate shared key
            std::string response_string = value.toStdString();
            std::string data1 = this->data.substr(0,8), data2 = response_string.substr(1,9);
            this->generate_shared_key(data1, data2);
            m_etatOnline = true;
            emit etatOnlineUpdated();
        }
    }
}

void SmartBulb::characteristicWritten(const QLowEnergyCharacteristic &c, const QByteArray &value)
{
    Q_UNUSED(c)
    Q_UNUSED(value)
    //qDebug() << Q_FUNC_INFO << c.uuid().toString() << value.size() << value;
}

void SmartBulb::serviceDetailsDiscovered(QLowEnergyService::ServiceState newState)
{
    // découverte ?
    if(newState != QLowEnergyService::ServiceDiscovered)
    {
        return;
    }

    //qDebug() << Q_FUNC_INFO << "service" << m_service->serviceUuid().toString();
    if(m_service->serviceUuid().toString() == Q_SMARTBULB_SERVICE_UUID)
    {
        m_characteristicCommande = m_service->characteristic(QBluetoothUuid(Q_SMARTBULB_CHARACTERISTIC_UUID));
        //qDebug() << Q_FUNC_INFO << "characteristic" << m_characteristicCommande.uuid().toString() << m_characteristicCommande.isValid() << m_characteristicCommande.properties();
        m_characteristicConfiguration = m_service->characteristic(QBluetoothUuid(Q_SMARTBULB_CHARACTERISTIC_CONFIGURATION));
        //qDebug() << Q_FUNC_INFO << "characteristic" << m_characteristicConfiguration.uuid().toString() << m_characteristicConfiguration.isValid() << m_characteristicConfiguration.properties();
        m_characteristicPair = m_service->characteristic(QBluetoothUuid(Q_SMARTBULB_CHARACTERISTIC_PAIR));
        //qDebug() << Q_FUNC_INFO << "characteristic" << m_characteristicPair.uuid().toString() << m_characteristicPair.isValid() << m_characteristicPair.properties();

        connect(m_service, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)), this, SLOT(serviceCharacteristicChanged(QLowEnergyCharacteristic,QByteArray)));
        connect(m_service, SIGNAL(characteristicRead(QLowEnergyCharacteristic,QByteArray)), this, SLOT(characteristicRead(QLowEnergyCharacteristic,QByteArray)));
        connect(m_service, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)), this, SLOT(characteristicWritten(QLowEnergyCharacteristic,QByteArray)));

        this->appairer();
    }
}

void SmartBulb::smartBulbConnecte()
{
    m_etatConnexion = true;
    emit connecteUpdated();
    m_controller->discoverServices();
}

void SmartBulb::smartBulbDeconnecte()
{
    m_etatConnexion = false;
    emit connecteUpdated();
}

void SmartBulb::connecteErreur(QLowEnergyController::Error error)
{
    Q_UNUSED(error)
    //qDebug() << Q_FUNC_INFO << error;
    m_etatConnexion = false;
    m_connexionErreur = true;
    emit connecteUpdated();
    emit erreurUpdated();
}

void SmartBulb::gererNotification(bool notification)
{
    if(m_service && m_characteristicConfiguration.isValid())
    {
        //qDebug() << Q_FUNC_INFO << m_characteristicConfiguration.uuid().toString() << m_characteristicConfiguration.properties();
        if(m_characteristicConfiguration.properties() & QLowEnergyCharacteristic::Notify)
        {
            QLowEnergyDescriptor descripteurNotification = m_characteristicConfiguration.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if(descripteurNotification.isValid())
            {
                // active la notification : 0100 ou désactive 0000
                if(notification)
                    m_service->writeDescriptor(descripteurNotification, QByteArray::fromHex("1"));
                else
                    m_service->writeDescriptor(descripteurNotification, QByteArray::fromHex("0"));
            }
        }
    }
}

void SmartBulb::writePair(const QByteArray &data)
{
    if(m_service && m_characteristicPair.isValid())
    {
        if(m_characteristicPair.properties() & QLowEnergyCharacteristic::Write)
        {
            m_service->writeCharacteristic(m_characteristicPair, data, QLowEnergyService::WriteWithResponse);
        }
    }
}

void SmartBulb::write(const QByteArray &data, bool reponse)
{
    if(m_service && m_characteristicCommande.isValid())
    {
        if(m_characteristicCommande.properties() & QLowEnergyCharacteristic::Write)
        {
            //qDebug() << Q_FUNC_INFO << m_service->serviceUuid().toString() << m_characteristicCommande.uuid().toString() << data << data.length();
            if(!reponse)
                m_service->writeCharacteristic(m_characteristicCommande, data, QLowEnergyService::WriteWithoutResponse);
            else
                m_service->writeCharacteristic(m_characteristicCommande, data, QLowEnergyService::WriteWithResponse);
        }
    }
}

static std::string encrypt(std::string key, std::string data) {
    std::reverse(key.begin(), key.end());
    std::reverse(data.begin(), data.end());

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex (ctx, EVP_aes_128_ecb(), NULL, (const unsigned char*)key.c_str(), NULL);
    EVP_CIPHER_CTX_set_padding(ctx, false);
    unsigned char buffer[1024], *pointer = buffer;
    int outlen;
    if (!EVP_EncryptUpdate(ctx, pointer, &outlen, (const unsigned char*)data.c_str(), data.length()))
    {
        qDebug() << Q_FUNC_INFO << "AES encryption failed in encryption stage";
      //throw std::runtime_error("AES encryption failed in encryption stage.");
      return std::string();
    }
    pointer += outlen;
    if (!EVP_EncryptFinal_ex(ctx, pointer, &outlen))
    {
        qDebug() << Q_FUNC_INFO << "AES encryption failed in final stage";
      //throw std::runtime_error("AES encryption failed in final stage.");
      return std::string();
    }
    pointer += outlen;
    std::string result = std::string((char*)buffer, pointer-buffer);
    std::reverse(result.begin(), result.end());
    EVP_CIPHER_CTX_free(ctx);

    return result;
}

static void print_hex_string(const std::string desc, const std::string & str) {
    std::stringstream stream;
    stream << desc << " : ";
    for (auto & chr : str)
      stream << std::hex << std::setfill('0') << std::setw(2) << (unsigned short)(chr & 0x00FF) << ",";
    std::string stringhex = stream.str().substr(0,stream.tellp()-1LL);
    qDebug() << QString::fromStdString(stringhex);
}

bool SmartBulb::appairer() {
    this->reverse_address = "";
    std::vector<std::string> mac;
    boost::split(mac, address, []( char c ){ return c == ':'; });
    for (auto rit = mac.rbegin(); rit != mac.rend(); ++rit)
      this->reverse_address.push_back(std::stoul(*rit, nullptr, 16));

    /* create public key */
    unsigned char buffer[8];
    int rc = RAND_bytes(buffer, 8);
    if(rc != 1) {
      unsigned long err = ERR_get_error();
      std::cerr << "Cannot generate random key. Error " << err << std::endl;
      return false;
    }
    this->data = std::string((char*)buffer).substr(0,8);
    // 2nd part of key is encrypted with mesh name and password
    this->data.append(8,0);
    std::string enc_data = this->key_encrypt(this->data);
    std::string packet = '\x0c' + this->data.substr(0,8) + enc_data.substr(0,8);

    /* send public key to device and get response */
    if(m_characteristicPair.isValid())
    {
        this->writePair(QByteArray::fromStdString(packet));
        m_service->readCharacteristic(m_characteristicPair);
    }

    return true;
}

std::string SmartBulb::combine_name_and_password() const {
    std::string data;
    for (int i=0; i<16; i++)
      data.push_back(this->name[i] ^ this->password[i]);
    return data;
}

void SmartBulb::generate_shared_key(const std::string & data1, const std::string & data2) {
    std::string key = this->combine_name_and_password();
    this->shared_key = encrypt(key, data1.substr(0,8) + data2.substr(0,8));
}

std::string SmartBulb::key_encrypt(std::string & key) const {
    std::string data = combine_name_and_password();
    std::string result = encrypt(key, data);
    return result;
}

std::string SmartBulb::encrypt_packet(std::string & packet) const {
    std::string auth_nonce = this->reverse_address.substr(0,4) + '\1' + packet.substr(0,3) + '\x0f';
    auth_nonce.append(7,0);
    std::string authenticator = encrypt(this->shared_key, auth_nonce);

    for (int i=0; i<15; i++)
      authenticator[i] ^= packet[i+5];

    std::string mac = encrypt(this->shared_key, authenticator);

    for (int i=0; i<2; i++)
      packet[i+3] = mac[i];

    std::string iv = '\0' + this->reverse_address.substr(0,4) + '\1' + packet.substr(0,3);
    iv.append(7,0);
    std::string buffer = encrypt(this->shared_key, iv);

    for (int i=0; i<15; i++)
      packet[i+5] ^= buffer[i];

    return packet;
}

std::string SmartBulb::decrypt_packet(std::string & packet) const {
    std::string iv = '\0' + this->reverse_address.substr(0,3) + packet.substr(0,5);
    iv.append(7,0);

    std::string result = encrypt(this->shared_key, iv);

    for (unsigned int i=0; i<packet.size()-7; i++)
      packet[i+7] ^= result[i];

    return packet;
}

std::string SmartBulb::build_packet(int command, const std::string & data) {
    /* Telink mesh packets take the following form:
       bytes 0-1   : packet counter
       bytes 2-4   : not used (=0)
       bytes 5-6   : mesh ID
       bytes 7     : command code
       bytes 8-9   : vendor code
       bytes 10-20 : command data

      All multi-byte elements are in little-endian form.
      Packet counter runs between 1 and 0xffff.
    */
    std::string packet;
    packet.resize(20, 0);
    packet[0] = this->m_packetCount & 0xff;
    packet[1] = (this->m_packetCount++ >> 8) & 0xff;
    packet[5] = this->m_meshId & 0xff;
    packet[6] = (this->m_meshId >> 8) & 0xff;
    packet[7] = command & 0xff;
    packet[8] = this->m_vendor & 0xff;
    packet[9] = (this->m_vendor >> 8) & 0xff;
    for (unsigned int i=0; i<data.size(); i++)
      packet[i+10] = data[i];

    #ifdef DEBUG
    print_hex_string("packet", packet);
    #endif
    std::string enc_packet = this->encrypt_packet(packet);

    if (this->m_packetCount > 0xffff)
      this->m_packetCount = 1;

    return enc_packet;
}

void SmartBulb::send_packet(int command, const std::string & data) {
    #ifdef DEBUG
    qDebug("command %02X", (unsigned char)command);
    print_hex_string("data", data);
    #endif
    std::string enc_packet = this->build_packet(command, data);
    this->write(QByteArray::fromStdString(enc_packet));
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

QT_END_NAMESPACE
