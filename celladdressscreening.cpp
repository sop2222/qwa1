#include "celladdressscreening.h"

CellAddressScreening::CellAddressScreening(QObject *parent) : QObject(parent)
{

}

int CellAddressScreening::CheckCellAddress(LedDirective *testSubject)
{
    testSubject->colorR = 0;
    testSubject->colorG = 0;
    testSubject->colorB = 0;

    return 0;
}
