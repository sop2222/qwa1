#ifndef AWINDOW_H
#define AWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QtMqtt/QMqttClient>
#include "dataStructure.h"
#include "awhpicture.h"
#include "aconfigjsoninlocalfile.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AWindow; }
QT_END_NAMESPACE

class AWindow : public QMainWindow
{
    Q_OBJECT

public:
    AWindow(QWidget *parent = nullptr);
    ~AWindow();
    QWidget* m_myWidget;  //This is used to paint rectangles and not on a MainWindows, but elsewhwere
    AConfigJsonInLocalFile *j_config; // A not so nessesary class of my own to read config file

private slots:
    void on_DoItButton_clicked();
    void updateLogStateChange();
    void brokerDisconnected();
    void messageParse(const QByteArray &message, const QMqttTopicName &topic);

    //These methods were created by hand
    int validateJsonContent(const QJsonDocument &jsonFromMessage, LedDirective &parsingLedDirective);
    void showWHpictureWindow();

    void on_quick2windowsButton_clicked();

    void on_lineEdit_editingFinished();

    void on_LogEdit_textChanged();

    void showEvent(QShowEvent *event);
    void closeEvent (QCloseEvent *event);

private:
    QMqttClient *m_client;  //An instance of qtmqtt libruary thing to connect to MQTT broker
    AwhPicture *whVisual;   //A dialog to create and show as I visualise the section of a warehouse rack
    Ui::AWindow *ui;
};
#endif // AWINDOW_H
