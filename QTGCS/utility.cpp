#include "utility.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>

QString readTextFile(const QString &fileName)
{
    if (fileName.isEmpty())
        return {};
    QFileInfo info(fileName);
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Reading file" << info.absoluteFilePath();
        QTextStream in(&file);
        QString contents = in.readAll();
        if (file.error() != QFile::NoError || in.status() != QTextStream::Ok)
            qDebug() << "Errors:" << in.status() << "/" << file.errorString();
        return contents;
    }
    qDebug() << "Cannot open file" << info.absoluteFilePath();
    return {};
}

QByteArray readJsonFile(const QString &filename)
{
    return readTextFile(filename).toUtf8();
}
