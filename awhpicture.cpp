#include "awhpicture.h"
#include "ui_awhpicture.h"
#include "aconfigjsoninlocalfile.h"
#include "dataStructure.h"

#include <QDir>
#include <QGraphicsScene>
#include <QGraphicsItem>

#define NAME_OF_BG_IMG "inactiveDeck.jpg"
#define NAME_OF_BG_IMG_RES ":/images/Resources/inactiveDeck.jpg"
#define DRAW_ELLIPSE_HORIZONTAL_SPACING 113
#define DRAW_ELLIPSE_VERTICAL_SPACING 100
#define DRAW_ELLIPSE_HEIGHT 27
#define DRAW_ELLIPSE_WIDTH 58
#define DRAW_ELLIPSE_TOPMOST_LEFT_X 27
#define DRAW_ELLIPSE_TOPMOST_LEFT_Y 70 // deprecated
#define DRAW_ELLIPSE_BOTTOMMOST_LEFT_Y 516

//An array of heights (or delta Y) in pixels of backgroung image
//separating rows of cells, that are of a different height
//const int drawEllipseDeltaY[7] = {111,127,72,69,67,67,67};
const int drawEllipseDeltaY[7] = {67,69,72,127,111,30,30};
//intentionally longer then 6 shelfs to graphically debug out of bounary

AwhPicture::AwhPicture(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AwhPicture)
{
    ui->setupUi(this);
}

AwhPicture::~AwhPicture()
{
    delete ui;
    delete scene;
}

void AwhPicture::loadBackgroungImage(QGraphicsScene *scene)
{
    if(scene == nullptr) return; // IDK if it helps, pointer must be initialized as nullptr somwhere
                                 // or this might as well point anywhere

    //Load a bitmap image from the CORRECT PATH!!! now it is relative path with exist() check
    // is file is not there, app should continue as if nothing happened, pixmap for background
    // is not critical to show imitation leds in the right order
    QDir mydir = QDir(QCoreApplication::applicationDirPath());
    QString url = "";
    if(mydir.exists(NAME_OF_BG_IMG) == true)
    {
        url = mydir.toNativeSeparators(mydir.filePath(NAME_OF_BG_IMG));
        //QString url = mydir.toNativeSeparators(mydir.currentPath());
        //QString url = R"(L:\QT\qwa1\resources\inactiveDeck.jpg)";
    }
    else    // if the external file does not exist, use internal resource
        url = NAME_OF_BG_IMG_RES;
    QPixmap rackBackgroundPixmap(url);
    scene->addPixmap(rackBackgroundPixmap);
    scene->update();
    // end Load a bitmap
}

void AwhPicture::markCells(LedDirective &renderLedDirective)
{
    int rectCalculateX, rectCalculateY;
    scene = new QGraphicsScene(this);
    ui->drawRack->setScene(scene);
    scene->setForegroundBrush(QColor(200, 210, 200, 23));
    this->loadBackgroungImage(scene);
    //Change titile to the requested settion
    this->setWindowTitle(QString::fromStdString("Warehouse \"").append(renderLedDirective.cellPrefix1) + "\" rack section #"
                         + QString::number(renderLedDirective.cellAdressNumbers[5]));

    //Render only the appropriate Cell from address
    if(renderLedDirective.cellAdressNumbers[6] > 0)
    {
        QPen pen1 = QPen(Qt::SolidLine); //Create a pen for a line
        pen1.setWidth(6);  // thick
        pen1.setCosmetic(true); //more trasform-prouf style of Pen
        //pen1.setColor(QColor(45,250,50));  //Set a shade of green of my liking
        pen1.setColor(QColor(renderLedDirective.colorR,renderLedDirective.colorG,renderLedDirective.colorB));
        //where to place the initial c/ircl/e Ellipse and what would be its size
        rectCalculateX = DRAW_ELLIPSE_TOPMOST_LEFT_X;
        // Please understand the commented line below, means that target cell is not first in its row
        //if(renderLedDirective.cellAdressNumbers[6] % 3 != 1)  // Cell number is not 1, not 4, not 7, not 10.....
        // Thus we need a multipler *0 for that Cell delta X, next Cell's delta X be multiplied by *1, next *2
        // and then in the next row again *0, *1, *2 and the next one and so on..
        rectCalculateX += DRAW_ELLIPSE_HORIZONTAL_SPACING * ((renderLedDirective.cellAdressNumbers[6]+2) % 3);
        //Now the position of the shelf
        rectCalculateY = DRAW_ELLIPSE_BOTTOMMOST_LEFT_Y;
        for(unsigned int additionYpos = 0; additionYpos < ((renderLedDirective.cellAdressNumbers[6]-1)/3); additionYpos++)
        {
            rectCalculateY -= drawEllipseDeltaY[additionYpos];
        }
        QRectF rect1 = QRectF(rectCalculateX,rectCalculateY,DRAW_ELLIPSE_WIDTH,DRAW_ELLIPSE_HEIGHT);
        scene->addEllipse(rect1,pen1);
    }
    scene->update();
}
