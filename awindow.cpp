#include "awindow.h"
#include "ui_awindow.h"
#include "aconfigjsoninlocalfile.h"
#include <QtMqtt/QMqttClient>
#include <QApplication>
#include <QScrollBar>

//Can delete later, only used once, only for showing a random cell test
#include <QRandomGenerator>

//For parsing Json
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>

LedDirective currentDirective; // Our global variable for this AWindow to access message content
                               // currently the scope matters not, because the structure is sent by method arguments

AWindow::AWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AWindow)
{
    ui->setupUi(this);

    //Create an Json config class instance for to read parameters
    j_config = new AConfigJsonInLocalFile(this);
    j_config->ReadJsonFile();

    //Create an qtMQTT class instance for our 1 broker
    m_client = new QMqttClient(this);
    m_client->setHostname(j_config->brokerURL);
    m_client->setPort(j_config->brockerPort);
    ui->lineEdit->setText(j_config->mqttTopic); //also a default for MQTT, but stored in a TextEdit control

    //Create an whVisual window that visualizes a Warehouse rack
    whVisual = new AwhPicture(this);
    whVisual->move(50,50);

    //Set up a method of an AWindow class (this main window) as slot for m_client signals
    connect(m_client, &QMqttClient::stateChanged, this, &AWindow::updateLogStateChange); // When connection status changes
    connect(m_client, &QMqttClient::disconnected, this, &AWindow::brokerDisconnected); // When disconnect happens
    // The most inportant one, same as previos, but to receive a message from our sub.topic
    connect(m_client, &QMqttClient::messageReceived, this, &AWindow::messageParse);   // Where the magic happens
}

void AWindow::showEvent(QShowEvent *event)
{
    QString content;
    if(j_config->fileFullName == "")
        content =  " No qwa1.json config file(defaults used)\n";
    else
        content =  " Using config: " + j_config->fileFullName + "\n";
    ui->LogEdit->insertPlainText(QDateTime::currentDateTime().time().toString() + content); //DEBUG - to log
    ui->statusbar->showMessage(content); //DEBUG - to status bar
    event->accept();
}

void AWindow::closeEvent (QCloseEvent *event)
{
    QCoreApplication::quit();
    event->accept();
}

AWindow::~AWindow()
{
    m_client->disconnect();  // Not sure I need that
    m_client->deleteLater();
    //delete m_client;
    //delete whVisual;
    delete ui;
}

// Everything is initiated by pushing this button in a beta version
// To Do: get rid of the button entirely or turn it to simply ReConnect button
void AWindow::on_DoItButton_clicked()
{
    m_client->connectToHost();
    // some tests //this->rectanglePaint(Qt::red); // Just tried to paint things
    ui->DoItButton->setText("Wait...Subscribe");
    if (m_client->state() == 2)
    {
        auto subscription = m_client->subscribe(ui->lineEdit->text(),
         static_cast<quint8>(ui->spinBoxQoS->text().toUInt()));
        if (!subscription)
        {
            ui->statusbar->showMessage("Could not subscribe. Is there a valid connection?",1000);
            return;
        }
        else
        {
            const QString content = QLatin1String(" Subscribed to: ")
                    + ui->lineEdit->text()
                    + QLatin1Char('\n');
            ui->LogEdit->insertPlainText( QDateTime::currentDateTime().time().toString() + content); //DEBUG - to log
            ui->statusbar->showMessage(content,1000); //DEBUG - to status bar
        }
    }
}

// This unfortunately is a poorly written part of qtMQTT, if you switch off LAN connection
// this method fails to be called somehow
void AWindow::brokerDisconnected()
{
    ui->DoItButton->setText(tr("ReConnect"));
    ui->LogEdit->insertPlainText(QDateTime::currentDateTime().time().toString()  + " Disconnected from MQTT broker"); //DEBUG - to log
    ui->statusbar->showMessage("Disconnected from MQTT broker\n",5000); //DEBUG - to status bar
}

// When something changes with respect to MQTT broker connection
void AWindow::updateLogStateChange()
{
    QString logMessage;
    //==========================
    //  MQTT connecton state
    //==========================
    switch (m_client->state())
    {
        case 0: logMessage = QString::fromStdString("Failed or disconnected"); break;
        case 1: logMessage = QString::fromStdString("Connecting to broker"); break;
        case 2: logMessage = QString::fromStdString("Connected to ");
                logMessage += j_config->brokerURL;
                ui->DoItButton->setText(tr("Subscribe \"") + ui->lineEdit->text() + "\"");
        break;
    }
    ui->LogEdit->insertPlainText(QDateTime::currentDateTime().time().toString() +  " " + logMessage + "\n"); //DEBUG - to log
    ui->statusbar->showMessage(logMessage,1000); //DEBUG - to status bar
}

// A method that is connected to m_client's signal that message has arrived
void AWindow::messageParse(const QByteArray &message, const QMqttTopicName &topic)
{
    int errorCodeOfValidate;
    //================
    //  get message
    //================
    const QString content = QDateTime::currentDateTime().time().toString()
                + QLatin1String(" Topic: ")
                + topic.name()
                + QLatin1String(" Message: \n")
                + message;
                /*+ QLatin1Char('\n');*/
    ui->LogEdit->insertPlainText(content.toUtf8());  //Debug - full str
    ui->statusbar->showMessage("Message received",2000); //DEBUG - to status bar

    //================
    //   parse json
    //================

    //Convert the QString text to Bytearray first
    QByteArray jsonData = message;
    if(jsonData.isEmpty() == true) ui->LogEdit->insertPlainText("ERR: Need to fill JSON data");

    //Assign the json text to a JSON object
    QJsonDocument jsonDocument = QJsonDocument::fromJson(jsonData);
    if(jsonDocument.isObject() == false) ui->LogEdit->insertPlainText("ERR: It is not a JSON object");

    //Validate json Object content
    errorCodeOfValidate = this->validateJsonContent(jsonDocument, currentDirective);
    QString bigStrDebugStruct = QString::fromLatin1("\nParsed structure reads:\n ");
    bigStrDebugStruct.append(currentDirective.cellPrefix1);  // Append one Qchar, nothing else works
    bigStrDebugStruct += QString::fromLatin1(".");
    bigStrDebugStruct += QString::number(currentDirective.cellAdressNumbers[1]);
    bigStrDebugStruct += QString::fromLatin1(".");
    bigStrDebugStruct.append(currentDirective.cellPrefix3);  // Append one Qchar, nothing else works
    for (int i=3; i < 7; i++)
    {
        bigStrDebugStruct += QString::fromLatin1(".");
        bigStrDebugStruct += QString::number(currentDirective.cellAdressNumbers[i]);
    }
    bigStrDebugStruct += QString::fromLatin1("\n--------------------\n");
    ui->LogEdit->insertPlainText(bigStrDebugStruct);

    switch (errorCodeOfValidate)
    {
        case 1:  ui->LogEdit->insertPlainText("ERR: cell address unrecognizable as a whole"); break;
        case 2:  ui->LogEdit->insertPlainText("ERR: more or less then 7 \".\" separators in cell address"); break;
        case 3:  ui->LogEdit->insertPlainText("ERR: integer expected in that address segment"); break;
        case 10:  ui->LogEdit->insertPlainText("ERR: color component R wrong value"); break;
        case 11:  ui->LogEdit->insertPlainText("ERR: color component G wrong value"); break;
        case 12:  ui->LogEdit->insertPlainText("ERR: color component B wrong value"); break;
    }

    if (!errorCodeOfValidate)
    {
        ui->statusbar->showMessage("Attention! Another windows to the right =>>",5000); //DEBUG - to status bar
        this->showWHpictureWindow();
    }
    //This below is a correct spell casting to just do one thing - change button color to R G B
    QPalette pal = ui->LogEdit->palette();
    pal.setColor(QPalette::Button, QColor(currentDirective.colorR,currentDirective.colorG,currentDirective.colorB));
    pal.setColor(QPalette::Base, QColor(currentDirective.colorR,currentDirective.colorG,currentDirective.colorB));
    pal.setColor(QPalette::Window, QColor(currentDirective.colorR,currentDirective.colorG,currentDirective.colorB));
    if (currentDirective.colorR + currentDirective.colorG + currentDirective.colorB < 50) //The background is too dark
        pal.setColor(QPalette::Text, QColor(200,200,200));
    else //Background is light enough
        pal.setColor(QPalette::Text, QColor(20,20,20));
    ui->LogEdit->setAutoFillBackground(true);
    ui->LogEdit->setPalette(pal);
    ui->LogEdit->update();

}

// A method that is called to validate the fields with data from a json Document, that was the MQTT message payload
int AWindow::validateJsonContent(const QJsonDocument &jsonFromMessage, LedDirective &parsingLedDirective)
{
    //Values from Json
    int colorRGB[3];
    //Names of said values
    static QString colorJsonValueName[3] =
    {
        "colorR",
        "colorG",
        "colorB",
    };
    bool intConversionSuccess; //A thing needed to call .toInt() function properly

    //Then get the main JSON object and parse value by value
    QJsonObject jsonObject = jsonFromMessage.object();

    //Validate colors, R, G and B to be either decimal ints 0-255 of hexes 00-FF
    for (int colorNumber = 0; colorNumber < 3; colorNumber++)
    {
        colorRGB[colorNumber] = jsonObject.value(colorJsonValueName[colorNumber]).toString().toInt(&intConversionSuccess,10);
        //colorRGB[colorNumber] = jsonObject.value(colorJsonValueName[colorNumber]).toUtf8().toInt(); //also works, just thinking
        if(colorRGB[colorNumber] < 0 || colorRGB[colorNumber] > 255 || !intConversionSuccess)
            return 10 + colorNumber;
    }
    parsingLedDirective.colorR = colorRGB[0];
    parsingLedDirective.colorG = colorRGB[1];
    parsingLedDirective.colorB = colorRGB[2];

    QString cellAddressString = jsonObject.value("cellID").toString();
    if (cellAddressString.length() <= 0) return 1; //ERR - no cell ID
    ui->LogEdit->insertPlainText("\nString Cell ID:\n" + cellAddressString); //DEBUG - to log

    //And then the bug TDEV-5476 happens
    QStringList cellAddressTokens = cellAddressString.split(".");
    if(cellAddressTokens.count() != 7)
        return 2; //ERR - Number of points is not 6 must permanently fix the TDEV-5476 bug
    for (int i=0; i < cellAddressTokens.count(); i++)
    {
        if (i == 1 || i == 3 || i == 4 || i == 5 || i == 6) // if position must contain a number, not a letter (i.e. only pos. [-,1,-,3,4,5,6,7])
        {
            parsingLedDirective.cellAdressNumbers[i] = cellAddressTokens.at(i).toDouble(&intConversionSuccess);
            if(!intConversionSuccess) return 3;
        }
        // ui->LogEdit->insertPlainText("\ntoken #" + QString::number(i) + " is " + cellAddressTokens.at(i)); //DEBUG - to log
    }

    // if address part/segment/token that needs to be a letter is a string longer then 1 character, then take 1st character to Qchar in structure x2
    // this did not work parsingLedDirective.cellPrefix3 = parsingLedDirective.cellPrefix1 = 'z';
    if (cellAddressTokens.at(1).length() >= 1)
        parsingLedDirective.cellPrefix1 = cellAddressTokens.at(0).at(0).toUpper().toLatin1();
    if (cellAddressTokens.at(3).length() >= 1)
        parsingLedDirective.cellPrefix3 = cellAddressTokens.at(2).at(0).toUpper().toLatin1();

    //No errors parsing
    return 0;
}

void AWindow::showWHpictureWindow()
{
    whVisual->showNormal();
    whVisual->move(AWindow::x() + AWindow::width() + 50, whVisual->y());
    // Circle of target color in WHpicture window
    if(whVisual->isVisible())
    {
        whVisual->markCells(currentDirective);
    }

}

void AWindow::on_quick2windowsButton_clicked()
{
    LedDirective fakeDirective;  //to impregnate it with random address and not disturb the actual one
    //This method is DEBUG mode temporarly one
    //in release do delete method and button as well
    //as the #include directive on top (QRandomGenerator)
    whVisual->showNormal();
    whVisual->move(this->x() + this->width() + 50, whVisual->y());
    if(whVisual->isVisible())
    {
        fakeDirective.colorB = 100;
        fakeDirective.cellAdressNumbers[6] = QRandomGenerator::global()->bounded(18) + 1;
        whVisual->markCells(fakeDirective);
    }
}

void AWindow::on_lineEdit_editingFinished()
{
    if(m_client->state() == 2)  //If connected to broker one might change the default topic to another one
        ui->DoItButton->setText(tr("Subscribe \"") + ui->lineEdit->text() + "\"");
    ui->DoItButton->setFocus(); //Change the text on that infamous button to to what are we actually subscribing
}

void AWindow::on_LogEdit_textChanged()
{
    QScrollBar *sb = ui->LogEdit->verticalScrollBar();
    sb->setValue(sb->maximum());
}
