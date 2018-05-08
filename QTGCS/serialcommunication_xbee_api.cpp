#include "serialcommunication_xbee_api.h"

SerialCommunication_XBEE_API::SerialCommunication_XBEE_API(QObject *parent) :
    QObject(parent)
{
}

SerialCommunication_XBEE_API::SerialCommunication_XBEE_API(QSerialPort *ser, QList<QuadStates *> tempObjList)
{
    xbee_api_portFound = false;
    serial = ser;
    qsList = tempObjList;

    xb = new QTXB(serial);
    QObject::connect(xb, SIGNAL(receivedATCommandResponse(ATCommandResponse*)), xb, SLOT(displayATCommandResponse(ATCommandResponse*)));
    QObject::connect(xb, SIGNAL(receivedModemStatus(ModemStatus*)), xb, SLOT(displayModemStatus(ModemStatus*)));
    QObject::connect(xb, SIGNAL(receivedTransmitStatus(TransmitStatus*)), xb, SLOT(displayTransmitStatus(TransmitStatus*)));
    QObject::connect(xb, SIGNAL(receivedRXIndicator(RXIndicator*)), xb, SLOT(displayRXIndicator(RXIndicator*)));
    QObject::connect(xb, SIGNAL(receivedRXIndicatorExplicit(RXIndicatorExplicit*)), xb, SLOT(displayRXIndicatorExplicit(RXIndicatorExplicit*)));
    QObject::connect(xb, SIGNAL(receivedNodeIdentificationIndicator(NodeIdentificationIndicator*)), xb, SLOT(displayNodeIdentificationIndicator(NodeIdentificationIndicator*)));
    QObject::connect(xb, SIGNAL(receivedRemoteCommandResponse(RemoteCommandResponse*)), xb, SLOT(displayRemoteCommandResponse(RemoteCommandResponse*)));

    connect(xb, SIGNAL(packetReady(RXIndicator*)), this, SLOT(readData(RXIndicator*)));
    connect(this, SIGNAL(boxidsDownloaded(int)), this, SLOT(boxidsDownloadedFlag(int)) );
}

SerialCommunication_XBEE_API::~SerialCommunication_XBEE_API()
{
    if(false)
        if(serial->isOpen())
        {
            serial->close();
            qDebug() << "XBEE API: Serial Port closed successfully";
        }
}

void SerialCommunication_XBEE_API::send(int objInd, QByteArray data)
{
    //qDebug() << qsList.at(objInd)->address_long << data;
    xb->unicast(qsList.at(objInd)->address_long, data);
}

void SerialCommunication_XBEE_API::sendCMD(int objInd, int cmd)
{
    switch (cmd) {
    case MSP_BOXIDS:
    {
        boxidsDownloadFlag[objInd] = false;
        QByteArray output;
        output.append("$M<");
        output.append(char(0xFF & 0));
        output.append(char(0xFF & cmd));
        output.append((char(0xFF & 0) ^ char(0xFF & cmd)));
        //qDebug() << output;
        while (!boxidsDownloadFlag[objInd]) {
            send(objInd, output);
            QTime dieTime= QTime::currentTime().addMSecs(1000);
            while( QTime::currentTime() < dieTime )
            {
                if (boxidsDownloadFlag[objInd])
                {
                    break;
                }
                QEventLoop loop;
                QTimer::singleShot(1, &loop, SLOT(quit()));
                loop.exec();
            }
        }
        break;
    }
    default:
    {
        QByteArray output;
        output.append("$M<");
        output.append(char(0xFF & 0));
        output.append(char(0xFF & cmd));
        output.append((char(0xFF & 0) ^ char(0xFF & cmd)));
        send(objInd, output);
        break;
    }
    }
}

void SerialCommunication_XBEE_API::sendCMD(int objInd, int cmd, Msp_rc_channels raw_rc)
{
    QByteArray output;
    char checksum = 0;
    output.append("$M<");
    output.append(char(0xFF & 16));
    output.append(char(0xFF & cmd));
    checksum = (char(0xFF & 16)) ^ (char(0xFF & cmd));
    for (int i = 0; i < 8; i++)
    {
        uint16_t tempValue = raw_rc.rcData[i];
        output.append(char(0xFF & tempValue));
        checksum = checksum ^ (char(0xFF & tempValue));
        output.append(char(0xFF & (tempValue >> 8)));
        checksum = checksum ^ (char(0xFF & (tempValue >> 8)));
    }
    // checksum byte
    output.append(checksum );
    send(objInd, output);
}

void SerialCommunication_XBEE_API::sendCMD(int objInd, int cmd, int ind)
{
    QByteArray output;
    output.append("$M<");
    output.append(char(0xFF & 1));
    output.append(char(0xFF & cmd));
    output.append(char(0xFF & ind));
    // checksum byte
    output.append((char(0xFF & 1) ^ char(0xFF & cmd)) ^ char(0xFF & ind) );
    send(objInd, output);
}

void SerialCommunication_XBEE_API::sendCMD(int objInd, int cmd, Mission tempMission)
{
    QByteArray output;
    char checksum = 0;
    output.append("$M<");
    output.append(char(0xFF & 21));
    output.append(char(0xFF & cmd));
    checksum = (char(0xFF & 21)) ^ (char(0xFF & cmd));

    output.append(char(0xFF & tempMission.wp_no));
    checksum = checksum ^ (char(0xFF & tempMission.wp_no));

    output.append(char(0xFF & tempMission.wp_action));
    checksum = checksum ^ (char(0xFF & tempMission.wp_action));

    output.append(char(0xFF & tempMission.wp_lat));
    checksum = checksum ^ (char(0xFF & tempMission.wp_lat));
    output.append(char(0xFF & (tempMission.wp_lat >> 8)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_lat >> 8)));
    output.append(char(0xFF & (tempMission.wp_lat >> 16)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_lat >> 16)));
    output.append(char(0xFF & (tempMission.wp_lat >> 24)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_lat >> 24)));

    output.append(char(0xFF & tempMission.wp_lon));
    checksum = checksum ^ (char(0xFF & tempMission.wp_lon));
    output.append(char(0xFF & (tempMission.wp_lon >> 8)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_lon >> 8)));
    output.append(char(0xFF & (tempMission.wp_lon >> 16)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_lon >> 16)));
    output.append(char(0xFF & (tempMission.wp_lon >> 24)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_lon >> 24)));

    output.append(char(0xFF & tempMission.wp_alt));
    checksum = checksum ^ (char(0xFF & tempMission.wp_alt));
    output.append(char(0xFF & (tempMission.wp_alt >> 8)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_alt >> 8)));
    output.append(char(0xFF & (tempMission.wp_alt >> 16)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_alt >> 16)));
    output.append(char(0xFF & (tempMission.wp_alt >> 24)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_alt >> 24)));

    output.append(char(0xFF & tempMission.wp_p1));
    checksum = checksum ^ (char(0xFF & tempMission.wp_p1));
    output.append(char(0xFF & (tempMission.wp_p1 >> 8)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_p1 >> 8)));

    output.append(char(0xFF & tempMission.wp_p2));
    checksum = checksum ^ (char(0xFF & tempMission.wp_p2));
    output.append(char(0xFF & (tempMission.wp_p2 >> 8)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_p2 >> 8)));

    output.append(char(0xFF & tempMission.wp_p3));
    checksum = checksum ^ (char(0xFF & tempMission.wp_p3));
    output.append(char(0xFF & (tempMission.wp_p3 >> 8)));
    checksum = checksum ^ (char(0xFF & (tempMission.wp_p3 >> 8)));

    output.append(char(0xFF & tempMission.wp_flag));
    checksum = checksum ^ (char(0xFF & tempMission.wp_flag));

    output.append(checksum);
    send(objInd, output);
}

void SerialCommunication_XBEE_API::parseSensorStatus(QuadStates *tempQS)
{
    uint16_t activeSensors = tempQS->msp_status_ex.packSensorStatus;
    QString tempSensorFlags = QString::number(activeSensors, 2).rightJustified(16, '0');

    if (tempSensorFlags.at(15) == '1')
    {
        tempQS->msp_sensor_flags.acc = true;
    }
    else if (tempSensorFlags.at(15) == '0')
    {
        tempQS->msp_sensor_flags.acc = false;
    }
    if (tempSensorFlags.at(14) == '1')
    {
        tempQS->msp_sensor_flags.baro = true;
    }
    else if (tempSensorFlags.at(14) == '0')
    {
        tempQS->msp_sensor_flags.baro = false;
    }
    if (tempSensorFlags.at(13) == '1')
    {
        tempQS->msp_sensor_flags.mag = true;
    }
    else if (tempSensorFlags.at(13) == '0')
    {
        tempQS->msp_sensor_flags.mag = false;
    }
    if (tempSensorFlags.at(12) == '1')
    {
        tempQS->msp_sensor_flags.gps = true;
    }
    else if (tempSensorFlags.at(12) == '0')
    {
        tempQS->msp_sensor_flags.gps = false;
    }
    if (tempSensorFlags.at(11) == '1')
    {
        tempQS->msp_sensor_flags.sonar = true;
    }
    else if (tempSensorFlags.at(11) == '0')
    {
        tempQS->msp_sensor_flags.sonar = false;
    }
    if (tempSensorFlags.at(9) == '1')
    {
        tempQS->msp_sensor_flags.pitot = true;
    }
    else if (tempSensorFlags.at(9) == '0')
    {
        tempQS->msp_sensor_flags.pitot = false;
    }
    if (tempSensorFlags.at(0) == '1')
    {
        tempQS->msp_sensor_flags.hardware = true;
    }
    else if (tempSensorFlags.at(0) == '0')
    {
        tempQS->msp_sensor_flags.hardware = false;
    }
}

void SerialCommunication_XBEE_API::parseArmingFlags(QuadStates *tempQS)
{
    uint16_t armingFlags = tempQS->msp_status_ex.armingFlags;
    QString tempArmingFlags = QString::number(armingFlags, 2).rightJustified(16, '0');

    if (tempArmingFlags.at(15) == '1')
    {
        tempQS->msp_arm_status.OK_TO_ARM = true;
    }
    else if (tempArmingFlags.at(15) == '0')
    {
        tempQS->msp_arm_status.OK_TO_ARM = false;
    }
    if (tempArmingFlags.at(14) == '1')
    {
        tempQS->msp_arm_status.PREVENT_ARMING = true;
    }
    else if (tempArmingFlags.at(14) == '0')
    {
        tempQS->msp_arm_status.PREVENT_ARMING = false;
    }
    if (tempArmingFlags.at(13) == '1')
    {
        tempQS->msp_arm_status.ARMED = true;
    }
    else if (tempArmingFlags.at(13) == '0')
    {
        tempQS->msp_arm_status.ARMED = false;
    }
    if (tempArmingFlags.at(12) == '1')
    {
        tempQS->msp_arm_status.WAS_EVER_ARMED = true;
    }
    else if (tempArmingFlags.at(12) == '0')
    {
        tempQS->msp_arm_status.WAS_EVER_ARMED = false;
    }
    if (tempArmingFlags.at(7) == '1')
    {
        tempQS->msp_arm_status.BLOCK_UAV_NOT_LEVEL = true;
    }
    else if (tempArmingFlags.at(7) == '0')
    {
        tempQS->msp_arm_status.BLOCK_UAV_NOT_LEVEL = false;
    }
    if (tempArmingFlags.at(6) == '1')
    {
        tempQS->msp_arm_status.BLOCK_SENSORS_CALIB = true;
    }
    else if (tempArmingFlags.at(6) == '0')
    {
        tempQS->msp_arm_status.BLOCK_SENSORS_CALIB = false;
    }
    if (tempArmingFlags.at(5) == '1')
    {
        tempQS->msp_arm_status.BLOCK_SYSTEM_OVERLOAD = true;
    }
    else if (tempArmingFlags.at(5) == '0')
    {
        tempQS->msp_arm_status.BLOCK_SYSTEM_OVERLOAD = false;
    }
    if (tempArmingFlags.at(4) == '1')
    {
        tempQS->msp_arm_status.BLOCK_NAV_SAFETY = true;
    }
    else if (tempArmingFlags.at(4) == '0')
    {
        tempQS->msp_arm_status.BLOCK_NAV_SAFETY = false;
    }
    if (tempArmingFlags.at(3) == '1')
    {
        tempQS->msp_arm_status.BLOCK_COMPASS_NOT_CALIB = true;
    }
    else if (tempArmingFlags.at(3) == '0')
    {
        tempQS->msp_arm_status.BLOCK_COMPASS_NOT_CALIB = false;
    }
    if (tempArmingFlags.at(2) == '1')
    {
        tempQS->msp_arm_status.BLOCK_ACC_NOT_CALIB = true;
    }
    else if (tempArmingFlags.at(2) == '0')
    {
        tempQS->msp_arm_status.BLOCK_ACC_NOT_CALIB = false;
    }
    if (tempArmingFlags.at(1) == '1')
    {
        tempQS->msp_arm_status.UNUSED = true;
    }
    else if (tempArmingFlags.at(1) == '0')
    {
        tempQS->msp_arm_status.UNUSED = false;
    }
    if (tempArmingFlags.at(0) == '1')
    {
        tempQS->msp_arm_status.BLOCK_HARDWARE_FAILURE = true;
    }
    else if (tempArmingFlags.at(0) == '0')
    {
        tempQS->msp_arm_status.BLOCK_HARDWARE_FAILURE = false;
    }
}

void SerialCommunication_XBEE_API::parseFlightModeFlags(QuadStates *tempQS)
{
    uint32_t flightModeFlags = tempQS->msp_status_ex.packFlightModeFlags;
    QString tempFlightModeFlags = QString::number(flightModeFlags, 2).rightJustified(32, '0');
    for (unsigned int i=0; i< sizeof(tempQS->active_boxids.box_id); i++)
    {
        uint8_t tempInd = tempQS->active_boxids.box_id[i];
        if (tempInd >= 0)
        {
            bool status = false;
            if (tempFlightModeFlags.at(31-i) == '1')
            {
                status = true;
            }
            else if (tempFlightModeFlags.at(31-i) == '0')
            {
                status = false;
            }

            switch (tempInd) {
            case 0: // arm
            {
                tempQS->msp_flight_modes.arm = status;
                break;
            }
            case 1: // angle
            {
                tempQS->msp_flight_modes.angle = status;
                break;
            }
            case 2: // horizon
            {
                tempQS->msp_flight_modes.horizon = status;
                break;
            }
            case 3: // nav althold
            {
                tempQS->msp_flight_modes.nav_althold = status;
                break;
            }
            case 5: // mag
            {
                tempQS->msp_flight_modes.mag = status;
                break;
            }
            case 6: // headfree
            {
                tempQS->msp_flight_modes.headfree = status;
                break;
            }
            case 7: // headadj
            {
                tempQS->msp_flight_modes.headadj = status;
                break;
            }
            case 10: // nav rth
            {
                tempQS->msp_flight_modes.nav_rth = status;
                break;
            }
            case 11: // nav poshold
            {
                tempQS->msp_flight_modes.nav_poshold = status;
                break;
            }
            case 12: // passthru
            {
                tempQS->msp_flight_modes.passthru = status;
                break;
            }
            case 13: // beeper
            {
                tempQS->msp_flight_modes.osd_sw = status;
                break;
            }
            case 19: // osd sw
            {
                tempQS->msp_flight_modes.nav_althold = status;
                break;
            }
            case 26: // black box
            {
                tempQS->msp_flight_modes.blackbox = status;
                break;
            }
            case 27: // failsafe
            {
                tempQS->msp_flight_modes.failsafe = status;
                break;
            }
            case 28: // nav wp
            {
                tempQS->msp_flight_modes.nav_wp = status;
                break;
            }
            case 29: // air mode
            {
                tempQS->msp_flight_modes.air_mode = status;
                break;
            }
            case 30: // home reset
            {
                tempQS->msp_flight_modes.home_reset = status;
                break;
            }
            case 31: // gcs nav
            {
                tempQS->msp_flight_modes.gcs_nav = status;
                break;
            }
            case 32: // heading lock
            {
                tempQS->msp_flight_modes.heading_lock = status;
                break;
            }
            case 33: // surface
            {
                tempQS->msp_flight_modes.surface = status;
                break;
            }
            case 35: // turn assist
            {
                tempQS->msp_flight_modes.turn_assist = status;
                break;
            }
            case 36: // nav launch
            {
                tempQS->msp_flight_modes.nav_launch = status;
                break;
            }
            default:
                break;
            }
        }
    }
}

void SerialCommunication_XBEE_API::downloadMission(int objInd, int id, QuadStates *tempObj)
{
    qDebug() << "Start download mission";
    missionDownloadFlag[objInd] = false;
    connect(this, SIGNAL(missionDownloaded(int)), this, SLOT(missionDownloadedFlag(int)));
    while (!missionDownloadFlag[objInd])
    {
        sendCMD(objInd, MSP_WP, id);
        QTime dieTime= QTime::currentTime().addMSecs(100);
        while( QTime::currentTime() < dieTime )
        {
            if (missionDownloadFlag[objInd])
            {
                break;
            }
            QEventLoop loop;
            QTimer::singleShot(1, &loop, SLOT(quit()));
            loop.exec();
        }
    }
    qDebug() << "Mission downloaded"
             << tempObj->temp_mission.mi.wp_no
             << tempObj->temp_mission.mi.wp_action
             << tempObj->temp_mission.mi.wp_lat
             << tempObj->temp_mission.mi.wp_lon
             << tempObj->temp_mission.mi.wp_alt
             << tempObj->temp_mission.mi.wp_p1
             << tempObj->temp_mission.mi.wp_p2
             << tempObj->temp_mission.mi.wp_p3
             << tempObj->temp_mission.mi.wp_flag;
}

void SerialCommunication_XBEE_API::missionDownloadedFlag(int objInd)
{
    missionDownloadFlag[objInd] = true;
}

void SerialCommunication_XBEE_API::downloadMissions(int objInd)
{
    QuadStates *tempQS;
    tempQS = qsList.at(objInd);
    int ind = 1;
    while(true)
    {
        downloadMission(objInd, ind, tempQS);
        if (tempQS->download_mission_list.missions.length() >= ind) // exist
        {
            tempQS->download_mission_list.missions.replace(ind-1,tempQS->temp_mission.mi);
        }
        else if (tempQS->download_mission_list.missions.length() < ind) // not exist
        {
            tempQS->download_mission_list.missions.append(tempQS->temp_mission.mi);
        }

        if (tempQS->temp_mission.mi.wp_flag == 0xa5)
        {
            break;
        }
        else
        {
            ind = ind + 1;
        }
    }
    qDebug() << "All missions downloaded";
}

void SerialCommunication_XBEE_API::uploadMission(int objInd, Mission tempMission, QuadStates *tempObj)
{
    missionUploadFlag[objInd] = false;
    while (!missionUploadFlag[objInd])
    {
        sendCMD(objInd, MSP_SET_WP, tempMission);
        QTime dieTime= QTime::currentTime().addMSecs(100);
        while( QTime::currentTime() < dieTime )
        {
            QEventLoop loop;
            QTimer::singleShot(1, &loop, SLOT(quit()));
            loop.exec();
        }
        downloadMission(objInd, tempMission.wp_no,tempObj);
        missionUploadFlag[objInd] = checkMissionUpload(tempMission, tempObj->temp_mission.mi);
    }
}

void SerialCommunication_XBEE_API::missionUploadedFlag(int objInd)
{
    //
    if (objInd)
    {}
}

void SerialCommunication_XBEE_API::uploadMissions(int objInd)
{
    QuadStates *tempObj;
    tempObj = qsList.at(objInd);
    for (int i = 0; i< tempObj->mission_list.missions.length(); i++)
    {
        qDebug() << "API - Upload Mission"
                 << tempObj->mission_list.missions.at(i).wp_no
                 << tempObj->mission_list.missions.at(i).wp_action
                 << tempObj->mission_list.missions.at(i).wp_lat
                 << tempObj->mission_list.missions.at(i).wp_lon
                 << tempObj->mission_list.missions.at(i).wp_alt
                 << tempObj->mission_list.missions.at(i).wp_p1
                 << tempObj->mission_list.missions.at(i).wp_p2
                 << tempObj->mission_list.missions.at(i).wp_p3
                 << tempObj->mission_list.missions.at(i).wp_flag;
        uploadMission(objInd, tempObj->mission_list.missions.at(i), tempObj);
    }
    qDebug() << "All missions uploaded";
}

bool SerialCommunication_XBEE_API::checkMissionUpload(Mission mi_send, Mission mi_rec)
{
    if ((mi_send.wp_no == mi_rec.wp_no) && (mi_send.wp_action == mi_rec.wp_action)
            && (mi_send.wp_lat == mi_rec.wp_lat) && (mi_send.wp_lon == mi_rec.wp_lon)
            && (mi_send.wp_alt == mi_rec.wp_alt) && (mi_send.wp_p1 == mi_rec.wp_p1)
            && (mi_send.wp_p2 == mi_rec.wp_p2) && (mi_send.wp_p3 == mi_rec.wp_p3)
            && (mi_send.wp_flag == mi_rec.wp_flag))
    {
        qDebug() << "Same";
        return true;
    }
    else
    {
        return false;
    }
}

void SerialCommunication_XBEE_API::radioControl(int radioMode)
{
    qDebug() << "XBEE API" << radioMode;
    switch (radioMode) {
    case 0:  // 0000 0000 radio is off
    {
        break;
    }
    case 1:  // 0000 0001 radio is on, but arm disarm disnav
    {
        for (int i = 0; i < qsList.length(); i++)
        {
            if (qsList.at(i)->address_long.toHex().length() > 2)
            {
                Msp_rc_channels msp_raw_rc;
                msp_raw_rc.rcData[0] = 1500;
                msp_raw_rc.rcData[1] = 1500;
                msp_raw_rc.rcData[2] = 1000;
                msp_raw_rc.rcData[3] = 1500;
                msp_raw_rc.rcData[4] = 1000;
                msp_raw_rc.rcData[5] = 1000;
                msp_raw_rc.rcData[6] = 1000;
                msp_raw_rc.rcData[7] = 1000;
                sendCMD(i, MSP_SET_RAW_RC, msp_raw_rc);
            }
        }
        break;
    }
    case 3:  // 0000 0011 radio is on, quad1 is armed but disnav
    {
        qDebug() << "Inside case 3";
        for (int i = 0; i < qsList.length(); i++)
        {
            qDebug() << "Inside for" + QString::number(i,10);
            if (qsList.at(i)->address_long.toHex().length() > 2)
            {
                qDebug() << "Inside if" + QString::number(i,10);
                Msp_rc_channels msp_raw_rc;
                msp_raw_rc.rcData[0] = 1500;
                msp_raw_rc.rcData[1] = 1500;
                msp_raw_rc.rcData[2] = 1000;
                msp_raw_rc.rcData[3] = 1500;
                msp_raw_rc.rcData[4] = 1000;
                msp_raw_rc.rcData[5] = 1000;
                msp_raw_rc.rcData[6] = 1000;
                msp_raw_rc.rcData[7] = 1000;
                if (i == 0)
                {
                    msp_raw_rc.rcData[4] = 1350;
                }
                qDebug() << msp_raw_rc.rcData[0] << msp_raw_rc.rcData[1]
                         << msp_raw_rc.rcData[2] << msp_raw_rc.rcData[3]
                         << msp_raw_rc.rcData[4] << msp_raw_rc.rcData[5]
                         << msp_raw_rc.rcData[6] << msp_raw_rc.rcData[7];
                sendCMD(i, MSP_SET_RAW_RC, msp_raw_rc);
            }
        }
        break;
    }
    case 5:  // 0000 0101
    {
        break;
    }
    case 9:  // 0000 1001
    {
        break;
    }
    case 15:  // 0000 1111 radio is on, all quads armed and disnaved
    {
        for (int i = 0; i < qsList.length(); i++)
        {
            if (qsList.at(i)->address_long.toHex().length() > 2)
            {
                Msp_rc_channels msp_raw_rc;
                msp_raw_rc.rcData[0] = 1500;
                msp_raw_rc.rcData[1] = 1500;
                msp_raw_rc.rcData[2] = 1000;
                msp_raw_rc.rcData[3] = 1500;
                msp_raw_rc.rcData[4] = 1350;
                msp_raw_rc.rcData[5] = 1000;
                msp_raw_rc.rcData[6] = 1000;
                msp_raw_rc.rcData[7] = 1000;
                sendCMD(i, MSP_SET_RAW_RC, msp_raw_rc);
            }
        }
        break;
    }
    case 19:  // 0001 0011 radio is on, quad1 is armed and naved
    {
        for (int i = 0; i < qsList.length(); i++)
        {
            if (qsList.at(i)->address_long.toHex().length() > 2)
            {
                Msp_rc_channels msp_raw_rc;
                msp_raw_rc.rcData[0] = 1500;
                msp_raw_rc.rcData[1] = 1500;
                msp_raw_rc.rcData[2] = 1000;
                msp_raw_rc.rcData[3] = 1500;
                msp_raw_rc.rcData[4] = 1000;
                msp_raw_rc.rcData[5] = 1000;
                msp_raw_rc.rcData[6] = 1000;
                msp_raw_rc.rcData[7] = 1000;
                if (i == 0)
                {
                    msp_raw_rc.rcData[2] = 1000;
                    msp_raw_rc.rcData[4] = 1350;
                    msp_raw_rc.rcData[7] = 1150;
                }
                qDebug() << msp_raw_rc.rcData[0] << msp_raw_rc.rcData[1]
                         << msp_raw_rc.rcData[2] << msp_raw_rc.rcData[3]
                         << msp_raw_rc.rcData[4] << msp_raw_rc.rcData[5]
                         << msp_raw_rc.rcData[6] << msp_raw_rc.rcData[7];
                sendCMD(i, MSP_SET_RAW_RC, msp_raw_rc);
            }
        }
        break;
    }
    case 37:  // 0010 0101
    {
        break;
    }
    case 81:  // 0100 1001
    {
        break;
    }
    case 127:  // 0111 1111 radio is on, all quads armed and naved
    {
        for (int i = 0; i < qsList.length(); i++)
        {
            if (qsList.at(i)->address_long.toHex().length() > 2)
            {
                Msp_rc_channels msp_raw_rc;
                msp_raw_rc.rcData[0] = 1500;
                msp_raw_rc.rcData[1] = 1500;
                msp_raw_rc.rcData[2] = 1000;
                msp_raw_rc.rcData[3] = 1500;
                msp_raw_rc.rcData[4] = 1350;
                msp_raw_rc.rcData[5] = 1000;
                msp_raw_rc.rcData[6] = 1000;
                msp_raw_rc.rcData[7] = 1150;
                sendCMD(i, MSP_SET_RAW_RC, msp_raw_rc);
            }
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

void SerialCommunication_XBEE_API::arm(QuadStates *){}

void SerialCommunication_XBEE_API::disarm(QuadStates *){}

void SerialCommunication_XBEE_API::nav(QuadStates *){}

void SerialCommunication_XBEE_API::disnav(QuadStates *){}

void SerialCommunication_XBEE_API::readData(RXIndicator *digiMeshPacket)
{
    QByteArray packet = digiMeshPacket->getData();
    /*
    qDebug() << "SC XBEE API readData"
             << packet.toHex()
             << digiMeshPacket->getSrcAddr64().toHex()
             << digiMeshPacket->getSrcAddr16().toHex();
             */
    QByteArray newPacket;
    unsigned startDelimiter = 0x24;
    if ((unsigned char)packet.at(0) == (unsigned char)startDelimiter)
    {
        if (packet.length() > 4)
        {
            unsigned length = packet.at(3)+6;
            if((unsigned char)packet.size() >= (unsigned char)length){
                newPacket.append(packet.left(length));
                for (int i =0; i< qsList.length(); i++)
                {
                    if (qsList.at(i)->address_long == digiMeshPacket->getSrcAddr64())
                    {
                        processPacket(i, newPacket);
                        break;
                    }
                }
            }
        }
    }
}

void SerialCommunication_XBEE_API::processPacket(int ind, QByteArray packet)
{
    unsigned length = (unsigned char)packet.at(3);
    unsigned cmdCode = (unsigned char)packet.at(4);
    QByteArray data = packet.mid(5, length+1);

    switch (cmdCode) {
    case MSP_STATUS_EX:
    {
        uint16_t cycletime = ((0xFF & data.at(1)) << 8) + (0xFF & data.at(0));
        int i2cError = ((0xFF & data.at(3)) << 8) + (0xFF & data.at(2));
        uint16_t activeSensors = ((0xFF & data.at(5)) << 8) + (0xFF & data.at(4));
        uint32_t flightModes = ((0xFF & data.at(9)) << 24) + ((0xFF & data.at(8)) << 16) + ((0xFF & data.at(7)) << 8) + (0xFF & data.at(6));
        uint8_t configProfile = (0xFF & data.at(10));
        uint16_t systemLoadPercent = ((0xFF & data.at(12)) << 8) + (0xFF & data.at(11));
        uint16_t armingFlags = ((0xFF & data.at(14)) << 8) + (0xFF & data.at(13));
        uint8_t accAxisFlags = (0xFF & data.at(15));
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_status_ex.cycletime = cycletime;
        tempQS->msp_status_ex.i2cGetErrorCounter = i2cError;
        tempQS->msp_status_ex.packSensorStatus = activeSensors;
        tempQS->msp_status_ex.packFlightModeFlags = flightModes;
        tempQS->msp_status_ex.getConfigProfile = configProfile;
        tempQS->msp_status_ex.averageSystemLoadPercent = systemLoadPercent;
        tempQS->msp_status_ex.armingFlags = armingFlags;
        tempQS->msp_status_ex.accGetCalibrationAxisFlags = accAxisFlags;
        //qDebug() << tempQS->msp_status_ex.packFlightModeFlags;
        parseSensorStatus(tempQS);
        parseFlightModeFlags(tempQS);
        parseArmingFlags(tempQS);
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_STATUS:
    {
        uint16_t cycletime = ((0xFF & data.at(1)) << 8) + (0xFF & data.at(0));
        int i2cError = ((0xFF & data.at(3)) << 8) + (0xFF & data.at(2));
        uint16_t activeSensors = ((0xFF & data.at(5)) << 8) + (0xFF & data.at(4));
        uint32_t flightModes = ((0xFF & data.at(9)) << 24) + ((0xFF & data.at(8)) << 16) + ((0xFF & data.at(7)) << 8) + (0xFF & data.at(6));
        uint8_t configProfile = (0xFF & data.at(10));

        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_status.cycletime = cycletime;
        tempQS->msp_status.i2cGetErrorCounter = i2cError;
        tempQS->msp_status.packSensorStatus = activeSensors;
        tempQS->msp_status.packFlightModeFlags = flightModes;
        tempQS->msp_status.getConfigProfile = configProfile;

        parseSensorStatus(tempQS);
        parseFlightModeFlags(tempQS);
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_BOXIDS:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);

        for (unsigned int i=0; i< length; i++)
        {
            tempQS->active_boxids.box_id[i] = (0xFF & data.at(i));
        }
        qsList.replace(ind, tempQS);
        emit boxidsDownloaded(ind);
        break;
    }
    case MSP_ALTITUDE:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_altitude.estimatedActualPosition = ((0xFF & data.at(0))) + ((0xFF & data.at(1)) << 8) + ((0xFF & data.at(2)) << 16) + ((0xFF & data.at(3)) << 24);
        tempQS->msp_altitude.estimatedActualVelocity = ((0xFF & data.at(4))) + ((0xFF & data.at(5)) << 8);
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_SONAR_ALTITUDE:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_sonar_altitude.rangefinderGetLatestAltitude = ((0xFF & data.at(0))) + ((0xFF & data.at(1)) << 8) + ((0xFF & data.at(2)) << 16) + ((0xFF & data.at(3)) << 24);
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_ATTITUDE:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_attitude.roll = ((0xFF & data.at(0))) + ((0xFF & data.at(1)) << 8);
        tempQS->msp_attitude.pitch = ((0xFF & data.at(2))) + ((0xFF & data.at(3)) << 8);
        tempQS->msp_attitude.yaw = ((0xFF & data.at(4))) + ((0xFF & data.at(5)) << 8);
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_RC:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        for (int i=0; i<8;i++)
        {
            tempQS->msp_rc_channels.rcData[i] = ((0xFF & data.at(2*i))) + ((0xFF & data.at(2*i+1)) << 8);
        }
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_ANALOG:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_analog.vbat = (0xFF & data.at(0));
        tempQS->msp_analog.mAhDrawn = (0xFF & data.at(1)) + ((0xFF & data.at(2)) << 8);
        tempQS->msp_analog.rssi = (0xFF & data.at(3)) + ((0xFF & data.at(4)) << 8);
        tempQS->msp_analog.amp = (0xFF & data.at(5)) + ((0xFF & data.at(6)) << 8);
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_RAW_IMU:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_raw_imu.acc[0] = ((0xFF & data.at(0))) + ((0xFF & data.at(1)) << 8);
        tempQS->msp_raw_imu.acc[1] = ((0xFF & data.at(2))) + ((0xFF & data.at(3)) << 8);
        tempQS->msp_raw_imu.acc[2] = ((0xFF & data.at(4))) + ((0xFF & data.at(5)) << 8);
        tempQS->msp_raw_imu.gyro[0] = ((0xFF & data.at(6))) + ((0xFF & data.at(7)) << 8);
        tempQS->msp_raw_imu.gyro[1] = ((0xFF & data.at(8))) + ((0xFF & data.at(9)) << 8);
        tempQS->msp_raw_imu.gyro[2] = ((0xFF & data.at(10))) + ((0xFF & data.at(11)) << 8);
        tempQS->msp_raw_imu.mag[0] = ((0xFF & data.at(12))) + ((0xFF & data.at(13)) << 8);
        tempQS->msp_raw_imu.mag[1] = ((0xFF & data.at(14))) + ((0xFF & data.at(15)) << 8);
        tempQS->msp_raw_imu.mag[2] = ((0xFF & data.at(16))) + ((0xFF & data.at(17)) << 8);
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_MOTOR:
    {
        break;
    }
    case MSP_SENSOR_STATUS:
    {
        break;
    }
    case MSP_LOOP_TIME:
    {
        break;
    }
    case MSP_MISC:
    {
        break;
    }
    case MSP_MODE_RANGES:
    {
        break;
    }
    case MSP_RAW_GPS:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_raw_gps.gpsSol_fixType = (0xFF & data.at(0));
        tempQS->msp_raw_gps.gpsSol_numSat = (0xFF & data.at(1));
        tempQS->msp_raw_gps.gpsSol_llh_lat = ((0xFF & data.at(2))) + ((0xFF & data.at(3)) << 8) + ((0xFF & data.at(4)) << 16) + ((0xFF & data.at(5)) << 24);
        tempQS->msp_raw_gps.gpsSol_llh_lon = ((0xFF & data.at(6))) + ((0xFF & data.at(7)) << 8) + ((0xFF & data.at(8)) << 16) + ((0xFF & data.at(9)) << 24);
        tempQS->msp_raw_gps.gpsSol_llh_alt = ((0xFF & data.at(10))) + ((0xFF & data.at(11)) << 8);
        tempQS->msp_raw_gps.gpsSol_groundSpeed = ((0xFF & data.at(12))) + ((0xFF & data.at(13)) << 8);
        tempQS->msp_raw_gps.gpsSol_groundCourse = ((0xFF & data.at(14))) + ((0xFF & data.at(15)) << 8);
        tempQS->msp_raw_gps.gpsSol_hdop = (0xFF & data.at(16));
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_COMP_GPS:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_comp_gps.gps_distanceToHome = (0xFF & data.at(0)) + ((0xFF & data.at(1)) << 8);
        tempQS->msp_comp_gps.gps_directionToHome = (0xFF & data.at(2)) + + ((0xFF & data.at(3)) << 8);
        tempQS->msp_comp_gps.gpsSol_flags_gpsHeartbeat = (0xFF & data.at(4));
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_NAV_STATUS:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_nav_status.nav_status_mode = (0xFF & data.at(0));
        tempQS->msp_nav_status.nav_status_state = (0xFF & data.at(1));
        tempQS->msp_nav_status.nav_status_activeWPAction = (0xFF & data.at(2));
        tempQS->msp_nav_status.nav_status_activeWPNumber = (0xFF & data.at(3));
        tempQS->msp_nav_status.nav_status_error = (0xFF & data.at(4));
        tempQS->msp_nav_status.MagHoldHeading = (0xFF & data.at(5));
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_GPSSVINFO:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_gps_svinfo.gpsSol_hdop1 = (0xFF & data.at(3));
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_GPSSTATISTICS:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_gps_statistics.gpsStats_lastMessageDt = (0xFF & data.at(0)) + ((0xFF & data.at(1)) << 8);
        tempQS->msp_gps_statistics.gpsStats_errors = (0xFF & data.at(2)) + ((0xFF & data.at(3)) << 8) + ((0xFF & data.at(4)) << 16) + ((0xFF & data.at(5)) << 24);
        tempQS->msp_gps_statistics.gpsStats_timeouts = (0xFF & data.at(6)) + ((0xFF & data.at(7)) << 8) + ((0xFF & data.at(8)) << 16) + ((0xFF & data.at(9)) << 24);
        tempQS->msp_gps_statistics.gpsStats_packetCount = (0xFF & data.at(10)) + ((0xFF & data.at(11)) << 8) + ((0xFF & data.at(12)) << 16) + ((0xFF & data.at(13)) << 24);
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_FEATURE:
    {
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->msp_feature.featureMask = (0xFF & data.at(0)) + ((0xFF & data.at(1)) << 8) + ((0xFF & data.at(2)) << 16) + ((0xFF & data.at(3)) << 24);
        qsList.replace(ind, tempQS);
        break;
    }
    case MSP_WP:
    {
        Mission tempMission;
        tempMission.wp_no = (0xFF & data.at(0));
        tempMission.wp_action = (0xFF & data.at(1));
        tempMission.wp_lat = ((0xFF & data.at(2))) + ((0xFF & data.at(3)) << 8) + ((0xFF & data.at(4)) << 16) + ((0xFF & data.at(5)) << 24);
        tempMission.wp_lon = ((0xFF & data.at(6))) + ((0xFF & data.at(7)) << 8) + ((0xFF & data.at(8)) << 16) + ((0xFF & data.at(9)) << 24);
        tempMission.wp_alt = ((0xFF & data.at(10))) + ((0xFF & data.at(11)) << 8) + ((0xFF & data.at(12)) << 16) + ((0xFF & data.at(13)) << 24);
        tempMission.wp_p1 = ((0xFF & data.at(14))) + ((0xFF & data.at(15)) << 8);
        tempMission.wp_p2 = ((0xFF & data.at(16))) + ((0xFF & data.at(17)) << 8);
        tempMission.wp_p3 = ((0xFF & data.at(18))) + ((0xFF & data.at(19)) << 8);
        tempMission.wp_flag = (0xFF & data.at(20));
        QuadStates *tempQS;
        tempQS = qsList.at(ind);
        tempQS->temp_mission.mi = tempMission;
        qsList.replace(ind, tempQS);
        emit missionDownloaded(ind);
        break;
    }
    default:
    {
        qDebug() << "Error:  Unknown Packet: " << packet.toHex();
        break;
    }

    }  // end of switch
    emit qsReady(&qsList);
}

void SerialCommunication_XBEE_API::boxidsDownloadedFlag(int objInd)
{
    boxidsDownloadFlag[objInd] = true;
}



