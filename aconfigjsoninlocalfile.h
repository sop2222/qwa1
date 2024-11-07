#ifndef ACONFIGJSONINLOCALFILE_H
#define ACONFIGJSONINLOCALFILE_H

#include <QObject>

class AConfigJsonInLocalFile : public QObject
{
    Q_OBJECT
public:
    //properties of MQTT protocol
    QString brokerURL;
    QString mqttTopic;
    QString fileFullName;
    int brockerPort;
    //Filters for Adress validation
    int cellMinAddress, cellMaxAddress;
    int rackMinAddress, rackMaxAddress;
    int floorMinAddress, floorMaxAddress;
    QString warehousesValidLetters, goodsTypeValidLetters;
    QVector<unsigned char> rackExcludeAddresses;
    //Methods
    explicit AConfigJsonInLocalFile(QObject *parent = nullptr);
    void ReadJsonFile();

signals:

};

#endif // ACONFIGJSONINLOCALFILE_H
