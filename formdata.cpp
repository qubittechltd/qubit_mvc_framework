#include "formdata.h"
#include <QRegularExpression>
#include <QString>
QString extractBoundary(const QString &contentType) {
    static const QRegularExpression re("boundary=(.*)", QRegularExpression::CaseInsensitiveOption);
    auto match = re.match(contentType);
    if (match.hasMatch()) {
        return "--" + match.captured(1).trimmed();
    }
    return QString();
}

QUrlQuery parseFormData(const QHttpServerRequest &request) {

    QString boundary;
    auto postData = request.body();
    QString dataStr = QString::fromUtf8(postData);
    auto formData = request.query();

    for(auto & h: request.headers()){
        if(h.first=="Content-Type"){
            boundary = extractBoundary(h.second);
            break;
        }
    }

    if(boundary.isEmpty()){
        QUrlQuery query(dataStr);
        QList<QPair<QString, QString>> items = query.queryItems();
        for (const QPair<QString, QString> &item : items) {
            formData.addQueryItem(item.first, item.second);
        }
    }else{
        // Convert the raw data to a QString for splitting, assuming UTF-8 encoding.

        // Split the data using the boundary.
        auto parts = dataStr.split(boundary, Qt::SkipEmptyParts);

        for (const auto& part : parts) {
            if (part.trimmed().isEmpty()
                || part.trimmed().endsWith("--")){
                continue;
            }

            auto headerBodySplit = part.indexOf("\r\n\r\n");
            auto headers = part.left(headerBodySplit).trimmed();
            auto value = part.mid(headerBodySplit + 4).trimmed();

            static QRegularExpression nameExp("name=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
            auto nameMatch = nameExp.match(headers);

            QString name = nameMatch.captured(1);
            formData.addQueryItem(name, value);
        }
    }

    // qDebug("post_data : \n%s",postData.data());
    return formData;
}

QMap<QString, QByteArray> parseMultipartFormData(const QByteArray &rawData, const QString &boundary) {
    QMap<QString, QByteArray> parts;
    QByteArray delimiter = boundary.toUtf8();
    QByteArray endDelimiter = (boundary + "--").toUtf8();
    QList<QByteArray> splitData = rawData.split('\n');

    QString currentName;
    QByteArray currentPart;

    bool capture = false;
    for (const QByteArray &line : splitData) {
        if (line.contains(delimiter)) {
            if (!currentName.isEmpty() && !currentPart.isEmpty()) {
                parts.insert(currentName, currentPart);
                currentPart.clear();
            }
            capture = false;
        } else if (line.startsWith("Content-Disposition:")) {
            QRegularExpression re("name=\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
            auto match = re.match(line);
            if (match.hasMatch()) {
                currentName = match.captured(1);
                capture = true;
            }
        } else if (capture && !line.trimmed().isEmpty() && !line.contains(endDelimiter)) {
            currentPart.append(line.trimmed() + "\n");
        }
    }

    // Add the last part
    if (!currentName.isEmpty() && !currentPart.isEmpty()) {
        parts.insert(currentName, currentPart);
    }

    return parts;
}
