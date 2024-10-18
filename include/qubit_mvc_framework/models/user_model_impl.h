#ifndef USERMODEL_IMPL_H
#define USERMODEL_IMPL_H
#include <qubit_mvc_framework/utilities/model/model.h>
#include "qubit_mvc_framework/utilities/model/model_relationships.h"
#include "qubit_mvc_framework/utilities/model/table_base_impl.h"
#include "qubit_mvc_framework/utilities/system_settings.h"
#include <QDateTime>

class ProductModel;
class UserParamsImpl : public ParamsBaseImpl<UserParamsImpl> {
    Q_GADGET
public:
    PARAM_INIT(UserParamsImpl);
};
Q_DECLARE_METATYPE(UserParamsImpl)


class UserModelImpl  {
public:
    static bool init_db(QSqlDatabase & default_db){
        return false;
    }
    operator bool(){
        return false;
    }

    // static inline auto Default() { return UserModelImpl();}

    // static inline auto DefaultShr() { return std::make_shared<UserModelImpl>();}

};

#endif // USERMODEL_IMPL_H
