#ifndef UTILITY_H
#define UTILITY_H

#include <QByteArray>
#include <QString>

QString readTextFile(const QString &fileName);
QByteArray readJsonFile(const QString &filename);

#endif // UTILITY_H
