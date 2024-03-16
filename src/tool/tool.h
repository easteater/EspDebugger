#ifndef TOOL_H
#define TOOL_H


#include <QString>
#include <QDateTime>
class Tool
{
public:
    QString getCurrentDateTime();
    QString convertDMMtoDD(QString  dmmString);
    Tool();
};

#endif // TOOL_H
