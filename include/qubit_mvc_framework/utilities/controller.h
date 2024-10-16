#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "QDebug"
#include "QMutex"
#include <QReadWriteLock>
#include <QObject>
#include <QByteArray>

class Controller : public QObject{
    template<typename T,typename> friend class ControllerSingleTon;
    friend class Session;
    Q_OBJECT
public:
    explicit Controller(QObject *parent):QObject(parent){};
protected:
    QReadWriteLock lock;
};
#endif // CONTROLLER_H
