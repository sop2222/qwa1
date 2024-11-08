#ifndef CELLADDRESSSCREENING_H
#define CELLADDRESSSCREENING_H

#include <QObject>
#include "aconfigjsoninlocalfile.h"
#include "dataStructure.h"

class CellAddressScreening : public QObject
{
    Q_OBJECT
public:
    explicit CellAddressScreening(QObject *parent = nullptr);
    int CheckCellAddress(LedDirective *testSubject);

private:
    LedDirective *subjectScreening;


signals:

};

#endif // CELLADDRESSSCREENING_H
