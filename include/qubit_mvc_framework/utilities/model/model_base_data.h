#ifndef MODELBASEDATA_H
#define MODELBASEDATA_H

#include "qubit_mvc_framework/utilities/model/sql_query.h"
#include "qubit_mvc_framework/utilities/model/table_attribute_base.h"
#include <QByteArray>
#include <QUuid>
#include <QAtomicInteger>

template<typename PARAMS>
class  ModelBaseData {
    template<typename M,typename P> friend class ModelBase;
    template<typename M,typename P> friend class ModelBaseImplSqlOrm;
    template<typename M> friend class HasOne;
    template<typename M> friend class HasMany;
    template<typename T> friend struct Eloquent::OrmEndFirst;

protected:
    ModelBaseData(const PARAMS &p) : _params(std::move(p)){

        _params.init();

        auto & vec = _params.getTableAttributesVec();

        for(auto & inf : vec){
            inf->_base=&table_attribute_base;
            if(!_params._existInDataBase)
               table_attribute_base._attribute_flags |= inf->_flag;
        }

        max_attribute_flags=((static_cast<quint64>(1) << vec.size()) - 1);
    }
    PARAMS                   _params;
    std::atomic_bool         _released =false;
    std::atomic_int64_t      _cached_index = -1;
    TableAttributeBase       table_attribute_base;

    quint64 max_attribute_flags;
};
#endif // MODELBASEDATA_H
