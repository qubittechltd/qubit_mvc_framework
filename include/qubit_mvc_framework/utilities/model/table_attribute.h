#pragma once
#include <QDateTime>
#include <QReadWriteLock>
#include <type_traits>
#include <QReadLocker>
// #include "config/app.h"
#include "qubit_mvc_framework/utilities/model/sql_operator.h"
#include "qubit_mvc_framework/utilities/model/table_attribute_base.h"
#include "qubit_mvc_framework/utilities/model/table_attribute_impl.h"

template<typename T,typename FIELD>
concept FIELD_OR_QVARIANT = std::is_same_v<T, FIELD> || std::is_same_v<T, QVariant>;

template<typename FIELD>
concept FIELD_IS_QDATETIME =  std::is_same_v<FIELD, QDateTime>;

template<typename FIELD>
concept FIELD_IS_QSTRING_OR_QBYTEARRAY =  std::is_same_v<FIELD, QString> || std::is_same_v<FIELD, QByteArray>;

template<typename T>
concept T_IS_NOT_CHAR_PTR = !std::is_same_v<T, char *> ;

template<typename T>
struct PARAM_DEFAULT_TYPE{
    PARAM_DEFAULT_TYPE(const T &v) : value(v){}
    const T & value;
};

template<typename T,typename FIELD>
concept T_IS_DEFAULT_TYPE = std::is_same_v<T, PARAM_DEFAULT_TYPE<FIELD>> ;

template<typename FIELD>
class TableAttribute : public  TableAttributeImpl {
    friend class TableAttributeImpl;
    friend class TableAttributeBase;
    friend class ParamsBase;
    template<typename P> friend class  ModelBaseData;
    template<typename M,typename P> friend class ModelBase;
    // template<typename M,typename P> friend class ModelBaseImpl;
    // template<typename M,typename P> friend class ModelBaseImplSqlOrm;

    const SQL_OPERATOR::TYPE sql_operator = SQL_OPERATOR::TYPE::_NONE ;
public:
    TableAttribute():TableAttributeImpl(){
        _data = QVariant::fromValue(FIELD());
    };

    std::string typeName() override {
        std::string name(typeid(FIELD).name());
        size_t pos = name.find_first_not_of("0123456789");
        if (pos != std::string::npos) {
            name = name.substr(pos);
        }
        return name;
    }

    const bool isOfType(const std::type_info &t) override {
        return typeid(FIELD) == t;
    }

    template<typename T>
    requires T_IS_DEFAULT_TYPE<T,FIELD>
    TableAttribute(const T &v) : TableAttributeImpl(v.value,true){}

    template<typename T>
    requires FIELD_OR_QVARIANT<T,FIELD> && T_IS_NOT_CHAR_PTR<T>
    TableAttribute(const T &v):TableAttributeImpl(v){}

    template<typename T = FIELD>
    TableAttribute(const std::tuple<T,SQL_OPERATOR::TYPE> &v):
    TableAttribute(std::get<0>(v)){
        const_cast<SQL_OPERATOR::TYPE &>(sql_operator) = std::get<1>(v);
    }

    template<typename T=void>
        requires (FIELD_IS_QSTRING_OR_QBYTEARRAY<FIELD> && not FIELD_IS_QDATETIME<FIELD>)
    TableAttribute(const char * v):TableAttribute(FIELD::fromStdString(v)){}

    template<typename T=void>
    requires FIELD_IS_QDATETIME<FIELD>
    TableAttribute(const char * v):TableAttribute(QDateTime::fromString(QString::fromStdString(v),app::database_datetime_format)){}

    TableAttribute(const QDateTime &v):TableAttributeImpl(v.toString(app::database_datetime_format)){}

    TableAttribute& operator=(const QDateTime &v) {
        *this = QVariant::fromValue(v.toString("yyyy-MM-dd hh:mm:ss"));
        return *this;
    }

    template<typename T>
    requires (FIELD_OR_QVARIANT<T, FIELD> && not FIELD_IS_QDATETIME<FIELD>)
    TableAttribute& operator=(const T & other) {//atomic
        if(_base){
            auto m_data = value<T>();
            if(m_data != other){
                volatile QWriteLocker lock(&_base->_dataLock);
                _data = other;
                _isInitialized = true;
                _isDefaulted   = false;
                _base->_attribute_flags |= _flag;
                _base->_dataStamp = QDateTime::currentMSecsSinceEpoch();
            }
        }else{
            Q_ASSERT_X(false,"unexcepted / wrong path","TableAttribute& operator=(const FIELD & other)");
        }
        return *this;
    }

    template<typename T=FIELD>
    requires (not FIELD_IS_QDATETIME<FIELD>)
    T value() const {
        return _data.value<T>();
    }

    template<typename T=QDateTime>
    T value() const {
        auto cc = _data.value<QString>();
        return T::fromString(cc);
    }

    const FIELD operator()() const {//atomic
        return *this;
    }

    operator const FIELD() const {//atomic
        if(_base){
            volatile QReadLocker lock(&_base->_dataLock);
            return value();
        }
        return value();
    }

    operator TableAttribute<FIELD>() const {//atomic
        if(_base){
            volatile QReadLocker lock(&_base->_dataLock);
            return value();
        }
        return value();
    }

    using Type = FIELD; // Extracted type

protected:
    TableAttribute& operator=(const TableAttribute& other) {

        if(other._base)
            other._base->_dataLock.lockForRead();

        auto base  = other._base;
        auto data  = other._data;
        auto isInitialized = other._isInitialized;
        auto isDefaulted   = other._isDefaulted;

        if(other._base)
            other._base->_dataLock.unlock();

        if(_base)
            _base->_dataLock.lockForWrite();

        if(_data != data){
            _data = data;
            _isInitialized = isInitialized;
            _isDefaulted   = isDefaulted;
            _base->_attribute_flags |= _flag;
            _base->_dataStamp = QDateTime::currentMSecsSinceEpoch();
        }

        if(_base)
            _base->_dataLock.unlock();

        return *this;
    };

public:
    // TableAttribute& operator=(const TableAttribute& other) = delete;
    TableAttribute(TableAttribute&& other) = delete;
    explicit TableAttribute(const TableAttribute& other){
        if(other._base)
            other._base->_dataLock.lockForRead();

        auto name  = other._name;
        auto base  = other._base;
        auto data  = other._data;
        auto flag  = other._flag;
        auto index = other._index;
        auto isInitialized = other._isInitialized;
        auto isDefaulted = other._isDefaulted;

        if(other._base)
            other._base->_dataLock.unlock();

        _data = data;
        _base =base;
        _name    = name;
        _flag   = flag;
        _index  = index;
        _isInitialized = isInitialized;
        _isDefaulted   = isDefaulted;

    };

};

