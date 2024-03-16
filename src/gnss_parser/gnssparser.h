#ifndef GNSSPARSER_H
#define GNSSPARSER_H
#include <QString>
#include <QRegularExpression>
#include <tool.h>


struct SatelliteInfo
{
    QString prn;
    int elevation;
    int azimuth;
    int snr;
    bool isValid;
};


#include <QString>
#include <QDebug>

struct GGA {
    QString utcTime;
    QString latitude;
    QString latitudeDirection;
    QString longitude;
    QString longitudeDirection;
    QString gpsStatus;
    QString satelliteCount;
    QString hdop;
    QString altitude;
    QString geoidalSeparation;
    QString differentialTime;
    QString differentialStationID;
};

struct RMC {
    QString utcTime;
    QString status;
    QString latitude;
    QString latitudeDirection;
    QString longitude;
    QString longitudeDirection;
    QString groundSpeed;
    QString groundHeading;
    QString utcDate;
    QString magneticVariation;
    QString magneticVariationDirection;
    QString modeIndicator;
};


struct GSV
{
    int totalMessages;
    int messageNumber;
    int totalSatellites;
    int totalUsed;
    QMap<QString, SatelliteInfo> satellites;
};

struct GLL {
    QString latitude;
    QString latitudeDirection;
    QString longitude;
    QString longitudeDirection;
    QString utcTime;
    QString status;
    QString modeIndicator;
};

struct GSA {
    QString mode2;
    QString mode1;
    QList<QString> satellitePRNs;
    QString pdop;
    QString hdop;
    QString vdop;
};



struct GnssRuntimeData {
    GGA gga;
    QMap<QString, GLL> gllMap;
    QMap<QString, GSA> gsaMap;
    GSV gsv;
    RMC rmc;


};

class GNSSParser
{
private:
    GnssRuntimeData gnssRuntimeData;
    bool ready =false;
    QList<QString> cmdOrderList;
    Tool tool;
public:
    GnssRuntimeData getGNSSRuntimeData();
    bool  parseNMea0180Txt(const QString&  singleCmdTxt  );
    bool parseGGA(const QString& ggaData);
    bool parseGSV(const QString& gsvData);
    bool parseRMC(const QString& message) ;
    bool parseGSA(const QString& message);
    bool parseGLL(const QString& message);
    void clearGsvBuffer();
    bool isReady();
    QString getSatelliteType(const QString& prn) ;
    GNSSParser();
    QString lastCommand;
};



#endif // GNSSPARSER_H
