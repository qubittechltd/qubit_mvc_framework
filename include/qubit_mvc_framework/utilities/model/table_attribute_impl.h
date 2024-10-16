#pragma once
#include <QReadWriteLock>
#include <QVariant>
#include <QReadLocker>
#include <QWriteLocker>
#include <QtCore/qmetaobject.h>
#include "qubit_mvc_framework/utilities/model/sql_query.h"
#include "qubit_mvc_framework/utilities/model/table_attribute_base.h"

class TableAttributeImpl {
    template<typename MODEL ,typename PARAMS>
    friend class  ModelBase;
    template<typename FIELD>
    friend class TableAttribute;
    template<typename MODEL ,typename PARAMS>
    friend class ModelBaseImplSqlOrm;
    friend class ParamsBase;
    template <typename T>
    friend class ParamsBaseImpl;
    template <typename T> friend class HasOne;
    template <typename T> friend class BelongsTo;
    template <typename T> friend class HasMany;
    template<typename MODEL ,typename PARAMS>
    friend class Migrations;
    template<typename T>
    friend struct Eloquent::OrmEndFirstOrFail;
    template<typename T>
    friend struct Eloquent::OrmEndGet;
    template<typename PARAMS>
    friend class  ModelBaseData;
protected:
    TableAttributeBase* _base = nullptr;
    quint64  _flag=0;//only set on construct of ModelBase is not expect to change hence no race condition
    qint8    _index=-1;
    QString  _name;
    QVariant _data;
    bool _isInitialized   = true;
    bool _isBaseAtrribute = false;
    bool _isDefaulted     = false;
    ~TableAttributeImpl()  {}
    TableAttributeImpl(QVariant d) :_data(d){}
    TableAttributeImpl(QVariant d,bool isDefaulted) :  _data(d) , _isDefaulted(isDefaulted){}

    void set_value(const QVariant & d){
        if(!_base) {
            _data = d;
            _isInitialized = true;
            _isDefaulted   = false;
        }else{
            volatile QWriteLocker lock(&_base->_dataLock);
            _data = d;
            _isInitialized = true;
            _isDefaulted   = false;
            _base->_attribute_flags |= _flag;
            _base->_dataStamp = QDateTime::currentMSecsSinceEpoch();
        }

    }

public:
    TableAttributeImpl():_isInitialized(false){}

    virtual std::string typeName() = 0;

    virtual const bool  isOfType(const std::type_info &t) = 0;

    template<typename T>
    auto value(){
        if(!_base) return _data.value<T>();
        volatile QReadLocker lock(&_base->_dataLock);
        return  _data.value<T>();
    }

    QVariant value() const{
        if(!_base) return   _data;
        volatile QReadLocker lock(&_base->_dataLock);
        return   _data;
    }
    QString name() const{
        return _name;
    }
    inline bool isValid(){
        return _isInitialized;
    }
    inline bool isBaseAtrribute(){
        return _isBaseAtrribute;
    }
    inline bool isDefaulted(){
        if(!_base) return   _isDefaulted;
        volatile QReadLocker lock(&_base->_dataLock);
        return   _isDefaulted;
    }
    inline bool isUnique(){
        return false;
    }
    bool operator==(const TableAttributeImpl& other) const {
        auto othr = const_cast<TableAttributeImpl*>(&other);
        auto ths = const_cast<TableAttributeImpl*>(this);
        return ths->name() == othr->name()
               && ths->value() == othr->value();
    }
};
Q_DECLARE_METATYPE(TableAttributeImpl)
