#ifndef TABLE_BASE_IMPL_H
#define TABLE_BASE_IMPL_H

#include <QMetaObject>
#include <QMetaProperty>
#include <QDateTime>
#include "qubit_mvc_framework/utilities/model/table_attribute_impl.h"
#include "utilities/model/table_base.h"


#define PARAM_INIT(CLASS) ParamInitializer<ParamsBaseImpl<CLASS>> _ = this;

template<typename T>
struct ParamInitializer{
    ParamInitializer(T * _param) {
        _param->init();
    }
};

struct ParamsBaseImplData{
    std::vector<TableAttributeImpl*> table_attributes_vec;
    std::atomic_bool isInitialized = false;

    // // Copy
    ParamsBaseImplData(){}
    ParamsBaseImplData(const ParamsBaseImplData& other) {}
    ParamsBaseImplData& operator=(const ParamsBaseImplData& other) { return *this; }

    // // Move
    ParamsBaseImplData(ParamsBaseImplData&& other) noexcept {}
    ParamsBaseImplData& operator=(ParamsBaseImplData&& other) noexcept { return *this;}

};

template<typename T>
class ParamsBaseImpl  : public ParamsBase {
    template<typename M,typename P> friend class ModelBase;
    template<typename M,typename P> friend class ModelBaseImplSqlOrm;
    template<typename P> friend struct ParamInitializer;
    template<typename P> friend class  ModelBaseData;
    template<typename M> friend class HasOne;
    template<typename M> friend class HasMany;
    friend class ParamsBase;
    friend struct ParamsBaseImplData;
    friend T;
    ParamsBaseImplData _data;
public:
    inline const std::vector<TableAttributeImpl*> &  getTableAttributesVec() const{
        return _data.table_attributes_vec;
    }

    static constexpr bool hasPrimary(){ return false;}

    TableAttributeImpl* firstUniqueAttribute(){
        auto this_param = static_cast<T*>(this);
        if constexpr (T::hasPrimary()){
            return this_param->primaryKey();
        }
        for (auto & attr : _data.table_attributes_vec) {
            if(attr->isUnique()){
                return attr;
            }
        }
        return nullptr;
    }
protected:

    void init(){
        bool expected = false;
        if(!_data.isInitialized.compare_exchange_strong(expected, true)){
            return;
        }

        const QMetaObject & metaObject = T::staticMetaObject;
        int propertyCount = metaObject.propertyCount();
        int propertyOffset = metaObject.propertyOffset();
        for (int index = 0; index < propertyCount; ++index) {
            QMetaProperty metaProperty = metaObject.property(index);
            QVariant q = metaProperty.readOnGadget(this);
            if(q.canConvert<TableAttributeImpl*>()){
                if(auto inf = q.value<TableAttributeImpl*>()){

                    Q_ASSERT_X(index < 64,"Model cannot have more than 63 attributes",__FILE__);

                    inf->_index = index;
                    inf->_flag  = FLAG(index);
                    inf->_name  = metaProperty.name();
                    inf->_isBaseAtrribute = (index < propertyOffset );
                    _data.table_attributes_vec.push_back(inf);
                }
            }

        }
    }


};




#endif // TABLE_BASE_IMPL_H
