#include "tool.h"

QString Tool::getCurrentDateTime()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString formattedDateTime = currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");
    return formattedDateTime;
}

/**
  Translate to real latitude and longitude.
 * @brief convertDMMtoDD
 * @param dmmString
 * @return
 */
QString Tool::  convertDMMtoDD(QString  dmmString)
{
    // Extract the degree part, extract the minute part, and convert the degree-minute format to decimal degrees.
    double dmm = dmmString.toDouble();
    // 提取度数部分
    int degrees = static_cast<int>(dmm / 100);
    // 提取分数部分
    double minutes = dmm - degrees * 100;
    // 将度分格式转换为度数格式
    double dd = degrees + minutes / 60;
    return QString::number(dd);
}

Tool::Tool()
{

}
