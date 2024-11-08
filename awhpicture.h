#ifndef AWHPICTURE_H
#define AWHPICTURE_H

#include "dataStructure.h"

#include <QDialog>
#include <QGraphicsScene>

namespace Ui {
class AwhPicture;
}

class AwhPicture : public QDialog
{
    Q_OBJECT

public:
    explicit AwhPicture(QWidget *parent = nullptr);
    ~AwhPicture();
    void markCells(LedDirective &renderLedDirective);
    void loadBackgroungImage(QGraphicsScene *scene);

private slots:

private:
    Ui::AwhPicture *ui;
    QImage *rackImage;
    QGraphicsScene *scene;
    QPixmap *rackBackgroundPixmap;
};

#endif // AWHPICTURE_H
