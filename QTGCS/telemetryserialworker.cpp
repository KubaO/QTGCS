#include "telemetryserialworker.h"

TelemetrySerialWorker::TelemetrySerialWorker(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<LogMessage>("LogMessage");
    teleMode = 0;
    teleSerialOn = false;
    teleSerialPortName = "";
    teleConnectionMethod = "";
}

//TelemetrySerialWorker::TelemetrySerialWorker(/*QString portName, QString connectionName, QString addrList[],*/ QObject *parent) :
//    QObject(parent)
//{}

void TelemetrySerialWorker::setTelemetrySerialPortName(QString portName)
{
    teleSerialPortName = portName;
}

void TelemetrySerialWorker::setTelemetryConnectionMethod(QString connectionMethod)
{
    teleConnectionMethod = connectionMethod;
}

void TelemetrySerialWorker::setTelemetrySerialOn(bool value)
{
    //
    //qDebug() << teleSerialPortName << teleConnectionMethod;
    //if (value) {}
    if (value == true)
    {
        teleSerialOn = value;
        // After teleSerialOn is set to true, create teleSerial and open it.
        //    Also open a new timer. But the timer will be stopped if
        //    teleMode changed, while the teleSerial will not be closed
        //    until teleSerialOn is set to false.

        // Create serial
        teleSerial = new QSerialPort;
        teleSerial->setPortName(teleSerialPortName);

        // Create serial communication handle
        scHandle = new SerialCommunication(teleSerial, teleConnectionMethod, teleAddressList);
        connect(scHandle,SIGNAL(quadsStatesChangeRequest(QList<QuadStates *> *)), this, SLOT(updateQuadsStates(QList<QuadStates *> *)) );

        // connect for log message
        connect(scHandle, SIGNAL(logMessageRequest(LogMessage)), this, SLOT(logMessage(LogMessage)));

        // preload required info
        scHandle->PreLoadInfo();

        // Create timer for telemetry.
        teleTimer = new QTimer();
        QObject::connect(teleTimer, SIGNAL(timeout()), this, SLOT(overviewPageInfoMode())); // SLOT to be filled.
        teleTimer->start(200);

        //qDebug() << "Start in " << QThread::currentThreadId();
        LogMessage tempLogMessage;
        tempLogMessage.id = QString("TelemetrySerial Worker");
        tempLogMessage.message = QString("Telemetry started.");
        emit logMessageRequest(tempLogMessage);
    }
    else if (value == false)
    {
        teleSerialOn = value;
        //qDebug() << "Stop in " << QThread::currentThreadId();
        if (teleTimer)
        {
            if (teleTimer->isActive())
            {
                teleTimer->stop();
            }
            delete teleTimer;
            teleTimer = NULL;
        }


        teleSerial->flush();
        if (teleSerial->isOpen())
        {
            teleSerial->close();
        }

        // Here the handle teleSerial and scHandle should be deleted,
        //    but doing this will cause the app crash.
        //    In the future, a signal and slot can be implemented:
        //    when the SerialCommunication class receives close signal,
        //    it will send out a signal to this class after it finishes
        //    that loop, then this class will delete these handles when
        //    receives the signal.
        //delete teleSerial;
        //delete scHandle;
        LogMessage tempLogMessage;
        tempLogMessage.id = QString("TelemetrySerial Worker");
        tempLogMessage.message = QString("Telemetry closed.");
        emit logMessageRequest(tempLogMessage);
    }
}

void TelemetrySerialWorker::setTelemetryMode(int mode)
{
    // End old timer
    //qDebug() << "Stop in " << QThread::currentThreadId();
    //qDebug() << "setTeleMode" << QThread::currentThreadId();
    //qDebug() << "set mode" << mode << teleMode;
    if (teleTimer)
    {
        //qDebug() << "Before stop" << teleTimer->isActive() << teleTimer->remainingTime();
        if (teleTimer->isActive())
        {
            //qDebug() << teleTimer->remainingTime();
            teleTimer->stop();
            //qDebug() << teleTimer->remainingTime();
            teleTimer->deleteLater();
            //teleTimer = NULL;
        }
        //qDebug() << "After stop" << teleTimer->isActive();
    }


    // Create new timer
    if (mode == 0)
    {
        teleTimer = new QTimer();
        QObject::connect(teleTimer, SIGNAL(timeout()), this, SLOT(overviewPageInfoMode())); // SLOT to be filled.
        teleTimer->start(200);
        //qDebug() << "Start in " << QThread::currentThreadId();
    }
    else if (mode > 0 && mode < 10)
    {
        teleTimer = new QTimer();
        QObject::connect(teleTimer, SIGNAL(timeout()), this, SLOT(overviewPageInfoMode())); // SLOT to be changed.
        teleTimer->start(200);
        //qDebug() << "Start in " << QThread::currentThreadId();
    }
    else if (mode > 10 && mode < 20)
    {
        //teleTimer = new QTimer();
        //teleTimer->singleShot(200, 1, this, SLOT(uploadMissionMode(mode)));
        //QTimer::singleShot(200, this, SLOT(uploadMissionMode()));
        //QObject::connect(teleTimer, SIGNAL(timeout()), this, SLOT(uploadMissionMode(mode)));
        //uploadMissionMode(mode);
        uploadMissionMode();
    }
    else if (mode > 20 && mode < 30)
    {
        downloadMissionMode();
    }
}

void TelemetrySerialWorker::teleModeUpdate(int mode)
{
    //qDebug() << "Current tele mode is " << mode;
    if (teleMode != mode)
    {
        teleMode = mode;
        //
        LogMessage tempLogMessage;
        tempLogMessage.id = QString("TelemetrySerial Worker");
        tempLogMessage.message = QString("Tele mode changed to: ") + QString::number(mode, 10);
        emit logMessageRequest(tempLogMessage);
        //
        setTelemetryMode(teleMode);
        emit teleModeChangeRequest(teleMode);
        //qDebug() << "teleModeUpdate" << QThread::currentThreadId();
    }
}

void TelemetrySerialWorker::teleSerialOnUpdate(bool value)
{
    if (teleSerialOn != value)
    {
        //teleSerialOn = value;
        setTelemetrySerialOn(value);
        //qDebug() << "teleSerialOnUpdate" << QThread::currentThreadId();
    }
}

void TelemetrySerialWorker::logMessage(LogMessage tempMessage)
{
    emit logMessageRequest(tempMessage);
}

void TelemetrySerialWorker::updateQuadsStates(QList<QuadStates *> *tempObjList)
{
    // Process quadsStatesChangeRequest from SerialCommunication class
    emit quadsStatesChangeRequest(tempObjList);
}

void TelemetrySerialWorker::overviewPageInfoMode()
{
    scHandle->RegularCheck();
    //qDebug() << "Running" << QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss_zzz");
    //tempLogMessage.id = QString("TelemetrySerial Worker");
    //tempLogMessage.message = QString("Running "+ QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss_zzz"));
    //emit logMessageRequest(tempLogMessage);
}

void TelemetrySerialWorker::uploadMissionMode()
{
    int mode = teleMode;
    // Check mode validity
    if (mode > 10 && mode < 20)
    {
        int objId = mode - 10;
        int objInd = objId - 1;
        scHandle->UploadMissions(objInd, mi_list_air[objInd]);
        mode = mode - 10;
        teleModeUpdate(mode);
    }
}

void TelemetrySerialWorker::downloadMissionMode()
{
    int mode = teleMode;
    // Check mode validity
    if (mode > 20 && mode < 30)
    {
        int objId = mode - 20;
        int objInd = objId - 1;
        scHandle->DownloadMissions(objInd);
        //scHandle->UploadMissions(objInd, mi_list_air[objInd]);
        mode = mode - 20;
        teleModeUpdate(mode);
    }
}
