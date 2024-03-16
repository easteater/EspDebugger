/****
 *
 * The main command parser of this software is currently functional. The implementation of the command parsing part is referenced from the list defined in the initializeMap method.
 * 本软件的主要指令解析器部分 目前可实现 实现解析指令部分参考 initializeMap 方法中定义的列表
 * Note that the functionality of checksum validation has not been implemented yet. All parsers do not currently address the checksum issue. This will be addressed later.
 * 注意,目前尚未实现校验码校验码的功能,所有解析器不考虑校验码码问题,后补
 *
 * */
#include "gnssparser.h"
#include <QDateTime>
#include <QDebug>
#include <QRandomGenerator>


//=====================================public funcition begin  公共方法区=================================
//define template 模版声明
template<typename T>
T createAndInitialize()
{
    return T();
}
template<typename T>
void initializeMap(QMap<QString, T>& map, const QStringList& prefixes)
{
    for (const QString& prefix : prefixes)
    {
        map["$"+prefix+"GGA"] = createAndInitialize<T>();
        map["$"+prefix+"GSA"] = createAndInitialize<T>();
        map["$"+prefix+"GLL"] = createAndInitialize<T>();
        map["$"+prefix+"GSA"] = createAndInitialize<T>();
        map["$"+prefix+"GSV"] = createAndInitialize<T>();
        map["$"+prefix+"RMC"] = createAndInitialize<T>();
    }
}

//=====================================public funcition over 公共方法区结束=================================

/**

 * @brief  Initialization function, currently used primarily for initializing the information of the defined gnssRuntimeData variables.
  初始化函数, 目前主要用于初始化已定义的  gnssRuntimeData 变量信息
 */
GNSSParser::GNSSParser()
{
    //Initialize the Map(gnssRuntimeData) to support commands for multi-satellite systems. 初始化 gnssRuntimeData, 使之支持多星系统的 指令的支持
    const QStringList prefixes = { "GN", "GP", "BD", "GL" };
    initializeMap(gnssRuntimeData.gsaMap, prefixes);
    initializeMap(gnssRuntimeData.gllMap, prefixes);
    initializeMap(gnssRuntimeData.gsaMap, prefixes);
}



/**
 *
 * @brief GNSSParser::parseNMea0180Txt parses a single NMEA-0180 protocol command statement whose type is uncertain.
The main logic is to extract the first command before the first ',', for example, in the statement $GNZDA,,,,,,*56, it extracts $GNZDA, and then derives the command as ZDA based on $GNZDA.
Then, it calls the parseZDA function based on the extracted command ZDA.

GNSSParser::parseNMea0180Txt 解析一条不确定是什么类型的单条 nmea-0180协议的命令语句
 * 主要逻辑是从这条语句中找到第一个,前的命令, 比如 $GNZDA,,,,,,*56取的是$GNZDA 然后根据$GNZDA取到命令为 ZDA
 * 然后根据 ZDA  调用 parseZDA函数
 * @param singleCmdTxt
 */
bool GNSSParser::parseNMea0180Txt(const QString&  singleCmdTxt) {
    //qDebug()<< "attempting to parse :" << singleCmdTxt;

    QString orgCommand = singleCmdTxt.left(singleCmdTxt.indexOf(','));
    //Not found, does not meet the structural requirements.  没有找到,不符合结构要求
   if (orgCommand.isEmpty()) {
       return false;
   }
   //Include the complete command prefix in the command order list. 完整的命令开头 加入命令顺序列表
   if (orgCommand.size() > 3 && orgCommand.front() == "$" &&  !cmdOrderList.contains(orgCommand)) {
       cmdOrderList.append(orgCommand);
   }
   //$GNZDA=>ZDA
   QString command = orgCommand.right(3);
   // Got GSV message for the first time.
   if (lastCommand != command && command == "GSV") {
       this->clearGsvBuffer();
   }
   lastCommand = command;
   // Define a function pointer type primarily to map commands to their corresponding parsing functions, allowing for finding the appropriate parser based on the command 定义函数指针类型, 主要作用是把指令和对应解析函数的映射,即通过命令找到对应的解析器
   typedef bool (GNSSParser::*ParseFunction)(const QString& );
   QMap<QString, ParseFunction> parseFunctions;
   parseFunctions["GGA"] = &GNSSParser::parseGGA;
   parseFunctions["GSV"] = &GNSSParser::parseGSV;
   parseFunctions["RMC"] = &GNSSParser::parseRMC;
   parseFunctions["GSA"] = &GNSSParser::parseGSA;
   parseFunctions["GLL"] = &GNSSParser::parseGLL;

   // Lookup the parsing function corresponding to a command and invoke the parser for execution. 查找命令对应的解析函数 并调用解析器进行执行
   ParseFunction parseFunction = parseFunctions.value(command, nullptr);
   if (parseFunction != nullptr) {
         (this->*parseFunction)(singleCmdTxt);
   }

   //qDebug()<<command<<"Parser, this command is currently not supported : " << singleCmdTxt;
    return true;
}




/**
 * @brief  Parser implementation method, parseGGA parses the GGA command. Subsequent methods follow the same pattern and are not individually annotated .  解析器实现方法,parseGGA解析的是GGA 指令,后续的方法都是该规律不再一一备注
 * @param cmdTxt
 * @return
 */
bool GNSSParser::parseGGA(const QString& cmdTxt)
{
    // Use ',' to split the statement into fields and check if the number of fields is correct, starting with "$GNGGA", and ensuring at least 14 fields in the statement 使用,分割语句成字段 并 检查语句字段数量是否正确并且以 "$GNGGA" 开头 并且字段至少为14个字段
    QStringList fields = cmdTxt.split(',');
    if (fields.size() >= 14 && fields.at(0) == "$GNGGA") {
        // Parse the fields and populate the data structure. 解析字段并填充数据结构
        gnssRuntimeData.gga.utcTime = fields.at(1);
        gnssRuntimeData.gga.latitude = fields.at(2);
        gnssRuntimeData.gga.latitudeDirection = fields.at(3);
        gnssRuntimeData.gga.longitude = fields.at(4);
        gnssRuntimeData.gga.longitudeDirection = fields.at(5);
        gnssRuntimeData.gga.gpsStatus = fields.at(6);
        gnssRuntimeData.gga.satelliteCount = fields.at(7);
        gnssRuntimeData.gga.hdop = fields.at(8);
        gnssRuntimeData.gga.altitude = fields.at(9);
        gnssRuntimeData.gga.geoidalSeparation = fields.at(10);
        gnssRuntimeData.gga.differentialTime = fields.at(11);
        gnssRuntimeData.gga.differentialStationID = fields.at(12);
    } else {
      //  qDebug()<< "Parsing failed.:" <<cmdTxt ;
//        return false;
    }
    qDebug()<< "gga  Parsing complete " ;
    return true;
}


bool GNSSParser:: parseGSV(const QString& gsvData)
{
    //4 卫星编号 5 卫星仰角 6 卫星方位角 7 信噪比
    GSV gsv = gnssRuntimeData.gsv;
    QStringList fields = gsvData.split(",");
    int fieldCount = fields.size();
    QRandomGenerator rand;
    if (fieldCount < 4) return false;
    gsv.totalMessages = fields[1].toInt();
    gsv.messageNumber = fields[2].toInt();
    //It could be BDGSV or GNGSV, etc. Regardless of the type, as long as it's the last message in each set, taking the third field will give the visible satellite count for that type. Accumulate these counts. 有可能是BDGSV也有可能是GNGSV等,不管哪种,只要是每个消息的最后一条,取第3个字段就是这种类型的可视总数,进行累加
    if (gsv.messageNumber  == gsv.totalMessages) {
        gsv.totalSatellites += fields[3].toInt();
    }
    //Parse all satellites. 解析所有卫星
    int satelliteCount = (fieldCount - 4) / 4;
    for (int i = 0; i < satelliteCount; ++i)
    {
        SatelliteInfo satellite;
        int index = i * 4 + 4;

        satellite.prn = fields[index] ;
        satellite.elevation = fields[index + 1].toInt();
        satellite.azimuth = fields[index + 2].toInt();
        satellite.isValid = false;

        satellite.snr = fields[index + 3].toInt();
        //Satellite number, azimuth, and elevation are considered valid as long as they are not empty. 编号,方位角,仰角只要不是空就是有效的
        if (fields[index]!= "" && fields[index+1]!= "" && fields[index+2]!= "") {
            gsv.totalUsed++;
            satellite.isValid = true;
             qDebug()<<"visible:" <<fields[index] <<":"<<fields[index+1] <<":"<<fields[index+2] ;
        } else  {
            qDebug()<<"not visible:" <<fields[index] <<":"<<fields[index+1] <<":"<<fields[index+2] ;
        }
        //Attention: If the satellite identifier is empty, a random value will be generated to avoid displaying empty satellites. This functionality may lead to inaccuracies in the satellite positioning system display. The default is to generate a value of -999 in such cases. 注意,如果取到的卫星编码是空时,则随机产生一个以免显示空卫星,该功能会导致定位卫星系统显示出错, 默认是-999-0生成一个
        if (satellite.prn == "") satellite.prn = "-" + QString::number(rand.bounded(0,999));
        gsv.satellites[satellite.prn] = satellite;



    }
    //Return the parsed command only when the last set of data arrives during this communication. 本次通信中,最后一组数据到达 时才返回解析命令
   gnssRuntimeData.gsv = gsv;
   return gsv.messageNumber == gsv.totalMessages;
}


bool GNSSParser:: parseRMC(const QString& message) {
    QStringList fields = message.split(",");
    RMC rmc;
    if (fields.size() >= 13) {
        QTime time = QTime::fromString(fields[1], "hhmmss.zzz");
        QDate rmcDate = QDate::fromString(fields[9], "ddMMyy");

        rmc.utcTime = time.toString("HH:mm:ss");
        rmc.status = fields[2];
        this->ready = rmc.status == "A";//在这设置一次RCM
        rmc.latitude = tool.convertDMMtoDD(fields[3]);

        rmc.latitudeDirection = fields[4];
        rmc.longitude = tool.convertDMMtoDD(fields[5]);
        rmc.longitudeDirection = fields[6];
        rmc.groundSpeed = fields[7];
        rmc.groundHeading = fields[8];
        rmc.utcDate = rmcDate.toString("yyyy-MM-dd");
        rmc.magneticVariation = fields[10];
        rmc.magneticVariationDirection = fields[11];
        rmc.modeIndicator = fields[12];
    }
    gnssRuntimeData.rmc = rmc;

    //qDebug()<< "rmc ok:"  ;
    return true;
}
QString GNSSParser::getSatelliteType(const QString& prn) {
    bool ok;
    int prnValue = prn.toInt(&ok);

    if (!ok) {
        return "Invalid PRN";
    }

    if (prnValue >= 1 && prnValue <= 32) {
        return "GPS[" + prn + "]";
    } else if (prnValue >= 33 && prnValue <= 64) {
        return "SBAS[" + prn + "]";
    } else if (prnValue >= 65 && prnValue <= 96) {
        return "GLONASS[" + prn + "]";
    } else if (prnValue >= 193 && prnValue <= 199) {
        return "QZSS[" + prn + "]";
    } else if (prnValue >= 201 && prnValue <= 261) {
        return "BD[" + prn + "]";
    } else if (prnValue >= 301 && prnValue <= 336) {
        return "GALILEO[" + prn + "]";
    } else if (prnValue >= 901 && prnValue <= 918) {
        return "IRNSS[" + prn + "]";
    }

    return "UNKNOW[" + prn + "]";;
}


//
bool GNSSParser:: parseGSA(const QString& message) {
    QStringList fields = message.split(",");
    if (fields.size() < 18)
    {
       //qDebug() << "Invalid GPGSA sentence";
       return false;
    }
    GSA gsa;
    gsa.mode2 = fields[1];  // model 2
    gsa.mode1 = fields[2];  // model 1
    gsa.satellitePRNs = fields.mid(3, 12);  // satellite PRNs
    gsa.pdop = fields[15];  // PDOP
    gsa.hdop = fields[16];  // HDOP
    gsa.vdop = fields[17];  // VDOP

    gnssRuntimeData.gsaMap[ fields[0]] = gsa;

    //qDebug()<< "gsa over"  ;
    return true;
}

bool GNSSParser::isReady() {
    return  ready;
}
bool GNSSParser:: parseGLL(const QString& message) {
    QStringList fields = message.split(",");
    if (fields.size() < 7) {
        qDebug() << "Invalid GLL sentence";
        return false;
    }

    GLL gll;
    gll.latitude = fields[1];                    // 纬度
    gll.latitudeDirection = fields[2];           // 纬度方向
    gll.longitude = fields[3];                   // 经度
    gll.longitudeDirection = fields[4];          // 经度方向
    gll.utcTime = fields[5];                     // UTC时间
    gll.status = fields[6];                      // 状态
    gll.modeIndicator = fields[7];               // 模式指示
    gnssRuntimeData.gllMap[ fields[0]] = gll;

    qDebug()<< "gll over "  ;
    return true;
}
GnssRuntimeData  GNSSParser::getGNSSRuntimeData() {
    return gnssRuntimeData;
}

void GNSSParser::clearGsvBuffer() {
    this->gnssRuntimeData.gsv.totalUsed = 0;//可用清0
     this->gnssRuntimeData.gsv.satellites.clear();
    this->gnssRuntimeData.gsv.totalSatellites = 0;//可视清0
}


