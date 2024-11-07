#include "aconfigjsoninlocalfile.h"
#include <QCoreApplication>
#include <QDebug>

//To reach a file with Json
#include <QFile>
#include <QDir>
#include <QStandardPaths>
//For parsing Json
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

#define CONFIG_FILE_NAME "qwa1.json"

AConfigJsonInLocalFile::AConfigJsonInLocalFile(QObject *parent) : QObject(parent)
{
    //Set properties default values
    this->brokerURL = QString::fromStdString("broker.emqx.io");
    this->brockerPort = 1883;
    this->mqttTopic = "teez";
    this->cellMinAddress = 1;
    this->cellMaxAddress = 18;
    this->rackMinAddress = 1;
    this->rackMaxAddress = 200;
    this->floorMinAddress = 1;
    this->floorMaxAddress = 1;
    this->warehousesValidLetters = "AB";
    this->goodsTypeValidLetters = "SM";
    this->rackExcludeAddresses.clear();
}


void AConfigJsonInLocalFile::ReadJsonFile()
{
    QString val;
    QFile jsonConfigFile;
    //QDir mydir = QDir(QCoreApplication::applicationDirPath()); //Did not work with Appimage, config becomes part of distr
    QStringList configFileDir = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);

    QDir mydir;
    mydir.setPath(configFileDir.at(0));
    qWarning() << tr("Searching directory: ")  << mydir.path().toUtf8();
    this->fileFullName = "";
    //See, if there is a config file with Json nearby
    if(mydir.exists(CONFIG_FILE_NAME) == true)
    {
        this->fileFullName = mydir.toNativeSeparators(mydir.filePath(CONFIG_FILE_NAME));
        qWarning() << this->fileFullName.toUtf8() << tr("It exists ");
    }
    else if(configFileDir.count() > 1)
    {
        mydir.setPath(configFileDir.at(1));
        qWarning() << tr("Searching directory: ")  << mydir.path().toUtf8();
        if(mydir.exists(CONFIG_FILE_NAME) == true)
        {
            this->fileFullName = mydir.toNativeSeparators(mydir.filePath(CONFIG_FILE_NAME));  //Repetative code - make a method of it
            qWarning() << this->fileFullName.toUtf8() << tr("It exists ");
        }
        else
            return;  //File not found, be content with defaults
    }
    else
        return;  //File not found, be content with defaults
    jsonConfigFile.setFileName(this->fileFullName);
    jsonConfigFile.open(QIODevice::ReadOnly | QIODevice::Text);
    val = jsonConfigFile.readAll();
    jsonConfigFile.close();
    // Parsing actual config

    QJsonDocument configJsonDoc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject configJsonObj = configJsonDoc.object();

    // MQTT settings
    QJsonValue value = configJsonObj.value(QString("mqtt"));
    QJsonObject item = value.toObject();

    QJsonValue subobj = item["brokerURL"];
    qWarning() << tr("QJsonObject[mqtt]: Broker URL")  << subobj.toString();
    if (subobj.isString() && not subobj.isNull())
       this->brokerURL = subobj.toString();

    QJsonValue subobj2 = item["brokerPort"];
    qWarning() << tr("QJsonObject[mqtt]: Broker Port")  << subobj2.toString();
    if (subobj2.isDouble() && not subobj2.isNull())  //This might not be enough of validation for integers.
                                                     //Please, just don't set negative port value or excceding 65535
                                                     //btw in case of test.mosquitto.org broker connects allright throug random port 3453
       this->brockerPort = subobj2.toInt();
    subobj = item["topicName"];
    qWarning() << tr("QJsonObject[mqtt]: topicName")  << subobj.toString();
    if (subobj.isString() && not subobj.isNull())
       this->mqttTopic = subobj.toString();

    // Cell address settings
    value = configJsonObj.value(QString("cellAddress"));
    item = value.toObject();

    subobj = item["cellMinAddress"];         // 1
    if (subobj.isDouble() && not subobj.isNull())
        this->cellMinAddress = subobj.toInt();
    qWarning() << tr("cellMinAddress")  << QString::number(this->cellMinAddress);

    subobj = item["cellMaxAddress"];        // 2
    if (subobj.isDouble() && not subobj.isNull())
        this->cellMaxAddress = subobj.toInt();
    qWarning() << tr("cellMaxAddress")  << QString::number(this->cellMaxAddress);

    subobj = item["rackMinAddress"];         // 3
    if (subobj.isDouble() && not subobj.isNull())
        this->rackMinAddress = subobj.toInt();
    qWarning() << tr("rackMinAddress")  << QString::number(this->rackMinAddress);

    subobj = item["rackMaxAddress"];         // 4
    if (subobj.isDouble() && not subobj.isNull())
        this->rackMaxAddress = subobj.toInt();
    qWarning() << tr("rackMaxAddress")  << QString::number(this->rackMaxAddress);

    subobj = item["floorMinAddress"];        // 5
    if (subobj.isDouble() && not subobj.isNull())
        this->floorMinAddress = subobj.toInt();
    qWarning() << tr("floorMinAddress")  << QString::number(this->floorMinAddress);

    subobj = item["floorMaxAddress"];        // 6
    if (subobj.isDouble() && not subobj.isNull())
        this->floorMaxAddress = subobj.toInt();
    qWarning() << tr("floorMaxAddress")  << QString::number(this->floorMaxAddress);

    subobj = item["warehousesValidLetters"]; // 7
    if (subobj.isString() && not subobj.isNull())
       this->warehousesValidLetters = subobj.toString().toUpper();
    qWarning() << tr("warehousesValidLetters")  << this->warehousesValidLetters;

    subobj = item["goodsTypeValidLetters"];  // 8
    if (subobj.isString() && not subobj.isNull())
       this->goodsTypeValidLetters = subobj.toString().toUpper();
    qWarning() << tr("goodsTypeValidLetters")  << this->goodsTypeValidLetters;

    //Now for the array
    /* in case of array get array and convert into int*/
    qWarning() << tr("rackExcludeAddresses") << item["rackExcludeAddresses"];  // 9
    QJsonArray test = item["rackExcludeAddresses"].toArray();
    foreach (QJsonValue i, test)
    {
        this->rackExcludeAddresses.push_back(i.toInt());
    }
    for (int j=0; j<this->rackExcludeAddresses.count(); j++)
        qWarning() << tr("exclude cell ") << QString::number( this->rackExcludeAddresses.at(j));
    // END : Cell address settings
}
