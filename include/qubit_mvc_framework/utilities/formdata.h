#ifndef FORMDATA_H
#define FORMDATA_H
#include "QtHttpServer/QHttpServerRequest"
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QDebug>
#include <QRegularExpression>
#include <QByteArray>
#include <QString>
#include <QMap>
#include <QList>
#include <QRegularExpression>


 // https://chat.openai.com/share/0e716759-cfe6-4e9e-b1af-6c1865eaaba4
// Utility function to extract boundary from Content-Type header
QString extractBoundary(const QString &contentType);

QUrlQuery parseFormData(const QHttpServerRequest &request);

// Parse the raw POST data
QMap<QString, QByteArray> parseMultipartFormData(const QByteArray &rawData, const QString &boundary);



#endif // FORMDATA_H
