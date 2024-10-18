#pragma once
#include <QReadWriteLock>
#include <QReadLocker>
#include <QWriteLocker>


template<typename FIELD>
class TableAttribute;
class TableAttributeImpl;

class TableAttributeBase {
    template<typename FIELD>
    friend class TableAttribute;
    friend class TableAttributeImpl;
    template<typename MODEL ,typename PARAMS>
    friend class  ModelBase;
    template<typename MODEL ,typename PARAMS>
    friend class ModelBaseImplSqlOrm;
    template<typename PARAMS>
    friend class  ModelBaseData;
public:
    TableAttributeBase(){
    }
    void ack_saved(){
        volatile QWriteLocker lock(&_dataLock);
        _savedStamp = _dataStamp;
        _savedTrials = 0;
    }

    bool increment_failed(){
        volatile QWriteLocker lock(&_dataLock);
        return _savedTrials++;
    }
    qint64 read_attribute_flags (){
        volatile QReadLocker lock(&_dataLock);
        return _attribute_flags;
    }
protected:
    qint64   _savedStamp=0;
    qint64   _savedTrials=0;
    qint64   _dataStamp=0;
    quint64  _attribute_flags=0;
    QReadWriteLock _dataLock;
};
