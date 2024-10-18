#ifndef CACHEFILES_H
#define CACHEFILES_H

#include "QFileSystemWatcher"
#include <QObject>
#include <QTimer>
#include <QList>

class CacheFiles : public QObject
{
    Q_OBJECT
    const struct  PARAMS{
        QString path;
        QString realPath;
        QByteArray data;
        QByteArray hash;
    } p;
public:
    static const CacheFiles & load_file(QObject *context, const QString &filePath);
    inline const PARAMS & params() const { return p;}
    bool operator==(const QString &other_path){
        return p.path == other_path;
    }
signals:

private:
    explicit CacheFiles(QObject *parent, const PARAMS &);
    ~CacheFiles();
    QTimer timer;
};

#endif // CACHEFILES_H
