#pragma once
#include "qubit_mvc_framework/utilities/model/table_attribute_base.h"
#include "qubit_mvc_framework/utilities/model/table_attribute_impl.h"
#include "qubit_mvc_framework/utilities/model/db_info.h"

class  ModelBaseImpl{
    template<typename MODEL ,typename PARAMS> friend class ModelBaseImplSqlOrm;
    template<typename MODEL ,typename PARAMS> friend class ModelBase;
protected:
    static const int RESERVED_VEC_SIZE = 100;
    static DB_INFO dummy_db_info;
};

