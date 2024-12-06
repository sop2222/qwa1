#ifndef DATASTRUCTURE_H
#define DATASTRUCTURE_H

struct LedDirective
{
    unsigned int colorR, colorG, colorB;
    unsigned int illuminateState, fallBackState;
    char cellPrefix1, cellPrefix3;
    unsigned int cellAdressNumbers[7];
    unsigned int illuminateTime;
};


#endif // DATASTRUCTURE_H
