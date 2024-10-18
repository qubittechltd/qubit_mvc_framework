#pragma once
#include <QMetaObject>
#include <QMetaProperty>
#include <QDateTime>
#include "qubit_mvc_framework/utilities/model/macros.h"
#include "qubit_mvc_framework/utilities/model/table_attribute_impl.h"
#include "qubit_mvc_framework/utilities/model/table_attribute.h"

class ParamsBase  {
    Q_GADGET
    template<class M ,class P> friend class  ModelBase;
    template<class M ,class P> friend class  ModelBaseImplSqlOrm;
    template<typename T>       friend class  ParamsBaseImpl;
      template<typename T> friend struct Eloquent::OrmEndFirst;
protected:
    QAtomicInteger<quint64> _existInDataBase = 0;//write only on creation
    template<typename ORIGN_TYPE>
    PARAM_DEFAULT_TYPE<ORIGN_TYPE> DEFAULT(const ORIGN_TYPE & data){
        return data;
    }

public:

    MODEL_ATTRIBUTE_DEFAULT(QDateTime,    created_at, QDateTime::currentDateTimeUtc());
    MODEL_ATTRIBUTE_DEFAULT(QDateTime,    updated_at, QDateTime::currentDateTimeUtc());
    MODEL_ATTRIBUTE(QDateTime,    deleted_at);

    template<typename PARAMS>
    static auto create(const QSqlRecord & record){
        PARAMS params;
        params.init();
        for(TableAttributeImpl* inf : params.getTableAttributesVec()){
            auto value = record.value(inf->name());
            if(value.isValid()){
                inf->set_value(value);
            }
        }
        params._existInDataBase=QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

        return params;
    }

    static quint64 FLAG(quint64 N){
        return N ? 2 * FLAG(N-1) : 1 ;
    }


};
