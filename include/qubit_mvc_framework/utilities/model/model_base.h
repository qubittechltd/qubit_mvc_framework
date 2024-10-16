#pragma once
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QReadWriteLock>

#include <QString>
#include <QThread>
#include <QUuid>
#include "qubit_mvc_framework/utilities/migrator.h"
#include "qubit_mvc_framework/utilities/model/model_base_data.h"
#include "qubit_mvc_framework/utilities/model/model_base_impl_sql.h"
#include "qubit_mvc_framework/utilities/model/table_attribute_base.h"
#include "qubit_mvc_framework/utilities/model/model_base_impl.h"
#include "qubit_mvc_framework/utilities/model/table_attribute_impl.h"
#include <QRandomGenerator>
#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h> //for abi::__cxa_demangle

#endif
template<typename MODEL ,typename PARAMS>
class  ModelBase : public ModelBaseImplSqlOrm<MODEL,PARAMS> {
    friend class ModelBaseImplSqlOrm<MODEL,PARAMS>;
    typedef ModelBaseImplSqlOrm<MODEL,PARAMS> ModelBasImSql;
    friend struct Eloquent::OrmEndFirst<MODEL>;
    friend struct Migrator<MODEL,PARAMS>;
    friend struct DeMigrator<MODEL,PARAMS>;

    template<typename FIELD> friend class SAVABLE;
    template<typename M> friend struct Eloquent::FromCPO;
    friend class Migration;
    typedef ModelBaseImplSqlOrm<MODEL,PARAMS> Base;
protected:
    explicit ModelBase(){}
    explicit ModelBase(const PARAMS &p) : _sh_data_ptr(createNonHash(p)){

    }

    explicit ModelBase(std::shared_ptr<ModelBaseData<PARAMS>> sh_p) :_sh_data_ptr(sh_p){}

public:
    static inline auto Default() { return MODEL();}

    static inline auto create(const PARAMS &p){
        return  MODEL(p);;
    }

    static typename Base::Collection create_vec(std::vector<const PARAMS> &pp){
        typename Base::Collection vec;
        vec.reserve(pp.size());
        for(auto && p :  pp) {
            vec.emplace_back(createNonHash(p));
        }
        return std::move(vec);
    }


    static void squeeze(){
        qint64 _cached_index (0);
        QMutexLocker locker(&_entriesLock);
        std::erase_if(_entries,[&_cached_index](auto entry){
            if(entry){
                entry->_cached_index = _cached_index++;
                return false;
            }
            return true;
        });
    }

    QByteArray hash() {
        volatile const QReadLocker locker(&_hashLock);
        return _hash ;
    }

    inline qint64 cached_index() const{
        return data_ptr()->_cached_index;
    }

    bool operator==(const ModelBase<MODEL,PARAMS>& other) const {

        if(bool(Base::primary_key()) ^ bool(other.primary_key())){ //XOR
            return false;
        }

        if(Base::primary_key()){
            return *Base::primary_key() == *other.primary_key();
        }

        if(data_ptr()->table_attributes_vec.size() != other.data_ptr()->table_attributes_vec.size()){
            return false;
        }

        for(int i = 0; i < data_ptr()->table_attributes_vec.size(); ++i) {
            if(*data_ptr()->table_attributes_vec.at(i) != *other.data_ptr()->table_attributes_vec.at(i)){
                return false;
            }
        }
        return true;
    }

    operator bool() const{
        if(_sh_data_ptr ||
            _temp_sh_data_ptr ||
            !_w_data_ptr.expired())
            return true;
        else
            return false;
    }

    bool isEmpty(){
        return operator bool();
    }

    auto operator->() const {
        return &(data_ptr()->_params);
    }

    auto operator->(){
        return &(data_ptr()->_params);
    }

    void relax_ref(){
        if(_sh_data_ptr){

            _temp_sh_data_ptr = _sh_data_ptr;

            _sh_data_ptr.reset();
        }
    }

    bool is_relaxed(){
        qt_assert_x(_temp_sh_data_ptr != _sh_data_ptr,"logic is wrong");

        return (!_temp_sh_data_ptr || _sh_data_ptr);
    }

    bool is_weak(){
        return (!_sh_data_ptr && !_temp_sh_data_ptr && !_w_data_ptr.expired());
    }

    void reset(){
        _sh_data_ptr.reset();
        _w_data_ptr.reset();
    }

    ModelBase(const ModelBase& m){
        _sh_data_ptr = m._sh_data_ptr;
    };

    ModelBase& operator=(const ModelBase& m) {
        if(m._sh_data_ptr){
            _sh_data_ptr = m._sh_data_ptr;
            _temp_sh_data_ptr.reset();
            _w_data_ptr.reset();
        }else if(m._temp_sh_data_ptr) {
            _w_data_ptr = m._temp_sh_data_ptr;
            _sh_data_ptr.reset();
            _temp_sh_data_ptr.reset();
        }else{
            _sh_data_ptr.reset();
            _temp_sh_data_ptr.reset();
            _w_data_ptr.reset();
        }
        return *this;
    };

    ModelBase(const ModelBase&& m) = delete;
    ModelBase& operator=(const ModelBase&& m)  = delete;

    const std::shared_ptr<ModelBaseData<PARAMS>> data_ptr() const {
        if(_sh_data_ptr)
            return _sh_data_ptr;
        else if (_temp_sh_data_ptr)
            return _temp_sh_data_ptr;
        else if (!_w_data_ptr.expired())
            return _w_data_ptr.lock();
        else
            return nullptr;
    }

    template<typename T>
    inline T safe_data_ptr(std::function<T(ModelBaseData<PARAMS> * raw_ptr)> func) const {

        ModelBaseData<PARAMS> * ptr = nullptr;

        if(_sh_data_ptr)
            ptr = _sh_data_ptr.get();
        else if (_temp_sh_data_ptr)
            ptr = _temp_sh_data_ptr.get();
        else if (!_w_data_ptr.expired())
            ptr = _w_data_ptr.lock().get();

        return func(ptr);
    }

    inline void safe_data_ptr(std::function<void(ModelBaseData<PARAMS> * raw_ptr)> func) const {
        safe_data_ptr<int>([&](ModelBaseData<PARAMS> * _raw_ptr) -> int {
            func(_raw_ptr);
            return 0;
        });
    }



protected:

    static QMap< int, int> registerMetaType(const char * name = nullptr) {

        if(name){
            return {{
                qRegisterMetaType<Migrator<MODEL,PARAMS>>(name),
                qRegisterMetaType<DeMigrator<MODEL,PARAMS>>(name)
            }};

        }else{
            int failed = 0;
#if defined(__GNUC__) || defined(__clang__)
            char* demangled = abi::__cxa_demangle(typeid(MODEL).name(), nullptr, nullptr, &failed);
            std::string __name(demangled ? demangled : typeid(MODEL).name());
            if(demangled)
                free(demangled);
#else
            std::string __name(typeid(MODEL).name());
#endif
            if(!failed){
                return {{
                    qRegisterMetaType<Migrator<MODEL,PARAMS>>(__name.c_str()),
                    qRegisterMetaType<DeMigrator<MODEL,PARAMS>>(__name.c_str())
                }};
            }
        }
        return {{0,0}};
    }

    static constexpr int migrationTypeId() {
        auto type = QMetaType::fromType<PARAMS>();
        return type.id();
    }

    static void migrate_up() {
        qDebug("running default migrate_up for model ( %s )",typeid(MODEL).name());
        PARAMS params;

        params.init();

        auto info = ModelBasImSql::get_db_Info();
        auto columns = params.getTableAttributesVec();


        QString query_str = "CREATE TABLE IF NOT EXISTS " + info.table_name + " (";

        const QString defaultSqlType = "VARCHAR(255)";
        const QMap<QString, QString> typeMapping = {
            {"bool",                "BOOL"},
            {"i",                   "INT)"},
            {"d",                   "DECIMAL(14, 2)"},
            {"qint8",               "TINYINT"},
            {"quint8",              "TINYINT UNSIGNED"},
            {"qint16",              "SMALLINT"},
            {"quint16",             "SMALLINT UNSIGNED"},
            {"qint32",              "INT"},
            {"quint32",             "INT UNSIGNED"},
            {"qint64",              "BIGINT"},
            {"quint64",             "BIGINT UNSIGNED"},
            {"wchar_t",             "CHAR(1)"},
            {"char16_t",            defaultSqlType},
            {"char32_t",            defaultSqlType},
            {"char",                "CHAR(1)"},
            {"signed char",         "TINYINT"},
            {"unsigned char",       "TINYINT UNSIGNED"},
            {"short",               "SMALLINT"},
            {"unsigned short",      "SMALLINT UNSIGNED"},
            {"int",                 "INT"},
            {"unsigned int",        "INT UNSIGNED"},
            {"long",                "BIGINT"},
            {"unsigned long",       "BIGINT UNSIGNED"},
            {"long long",           "BIGINT"},
            {"unsigned long long",  "BIGINT UNSIGNED"},
            // {"QByteArray",          "BLOB"},
             {"QByteArray",         defaultSqlType},
            {"QString",             defaultSqlType},
            {"QDateTime",           defaultSqlType},
            {"QDate",               defaultSqlType},
            {"QTime",               defaultSqlType},
            {"QVariant",            defaultSqlType},
            {"std::string",         defaultSqlType},
            {"std::vector",         "BLOB"},  // Depending on content, could also map to JSON or TEXT
            {"std::list",           "BLOB"},
            {"std::map",            "BLOB"},
            {"std::set",            "BLOB"},
            {"std::array",          "BLOB"}
        };

        for (auto c : columns) {
            QString dataType = c->typeName();

            if (!typeMapping.contains(dataType)) {
                // qWarning("Error: Can not translate variable'%s' From table'%s'  .No SQL type mapping available for C++ type '%s'",
                //        c->_name, info.table_name, dataType);
                return;
            }
            QString sqlType = typeMapping.value(dataType);

            query_str += "`" + c->_name + "` " + sqlType;

            if (c != columns.back()) {
                query_str += ", ";
            }
        }
        query_str += ")";

        QSqlDatabase default_db;
        if(!MODEL::init_db(default_db)){
            qWarning("Failed to initilize default connection");
            return;
        }

        QSqlQuery query(default_db);
        if(!query.exec(query_str)){
            qWarning() << "SQL Error:" << query.lastError().text();
            qWarning() << "SQL Error:" << query.lastQuery();
        }
    }

    static void migrate_down() {
        qDebug("running default migrate_down for model ( %s )",typeid(MODEL).name());
        auto info = ModelBasImSql::get_db_Info();
        QString query_str =  "DROP TABLE `"+ info.table_name+"`";

        QSqlDatabase db;
        if(!MODEL::init_db(db)){
            qWarning("Failed to initilize default connection");
            return;
        }

        QSqlQuery query(db);
        if (!query.exec(query_str)) {
            qWarning() << "SQL Error:" << query.lastError().text();
            qWarning() << "SQL Error:" << query.lastQuery();
        }
    }

    static void factory(const int count){
        // PARAMS params;

        // params.init();

        // auto info = ModelBasImSql::get_db_Info();
        // auto columns = params.getTableAttributesVec();
        // QRandomGenerator *rng = QRandomGenerator::global();
        // for(int i = 0; i < count; i++){
        //     for (TableAttributeImpl* inf : params.getTableAttributesVec()) {
        //         if(!inf->isBaseAtrribute() && !inf->_isDefaulted){
        //             if(inf->isOfType(typeid(QDateTime))){
        //                 QDateTime now = QDateTime::currentDateTime();
        //                 qint64 randomMSecs = rng->bounded(qint64(365 * 24 * 60 * 60 * 1000));
        //                 QDateTime randomDateTime = now.addMSecs(-randomMSecs);
        //                 inf->set_value(randomDateTime);
        //             }else if(inf->isOfType(typeid(QString)) || inf->isOfType(typeid(QByteArray))){
        //                 const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        //                 int length = rng->bounded(5, 21); // 5 to 20
        //                 QString randomString;
        //                 for (int j = 0; j < length; ++j) {
        //                     int index = rng->bounded(chars.length());
        //                     randomString.append(chars.at(index));
        //                 }
        //                 inf->set_value(randomString);
        //             }else if(inf->isOfType(typeid(QByteArray))){
        //                 int length = rng->bounded(10, 51);
        //                 QByteArray randomBytes;
        //                 for (int j = 0; j < length; ++j) {
        //                     char byte = rng->bounded(256);
        //                     randomBytes.append(byte);
        //                 }
        //                 inf->set_value(randomBytes);
        //             }else if(inf->isOfType(typeid(int))){
        //                 int randomInt = rng->bounded(1001);
        //                 inf->set_value(randomInt) ;
        //             }else if(inf->isOfType(typeid(double))){
        //                 double randomDouble = rng->generateDouble() * 1000.0;
        //                 inf->set_value(randomDouble);
        //             }else if(inf->isOfType(typeid(bool))){
        //                 bool randomBool = rng->bounded(2) == 1;
        //                 inf->set_value(randomBool);
        //             }
        //         }
        //     }
        //     auto factory_model = MODEL::create(params);
        //     factory_model.flush();
        // }

    }

    static auto UseSoftDelete(){
        return false;
    }

    static quint64 UseCache(){
        return QUBIT_MVC_APP::model_cache_time();
    }

    inline PARAMS * params(){
        return &(data_ptr()->_params);
    }

    inline const PARAMS * params() const {
        return &(data_ptr()->_params);
    }

    template<typename T>
    inline const T * params(){
        return static_cast<const T *>(&data_ptr()->_params);
    }

    static std::shared_ptr<ModelBaseData<PARAMS>> createNonHash(){//loaded up
        return createNonHash(PARAMS());
    }

    static void free_func (ModelBaseData<PARAMS> * obj){
        if(!QCoreApplication::instance() || (QThread::currentThread() == QCoreApplication::instance()->thread())){
            delete obj;
        }else{
            QMetaObject::invokeMethod(QCoreApplication::instance(),[obj]{
                delete obj;
            },Qt::QueuedConnection);
        }
    };

    static std::shared_ptr<ModelBaseData<PARAMS>> createRawModel(ModelBaseData<PARAMS> * data){
        return std::shared_ptr<ModelBaseData<PARAMS>>(data,[](auto *obj){

            if(not MODEL::UseCache()){
                free_func(obj);
            }else if(obj->_params.deleted_at.value().isValid()){

                QMutexLocker locker(&_entriesLock);

                Q_ASSERT_X((obj->_cached_index != -1)
                               || (obj->_cached_index < _entries.size())
                               || !_entries.at(obj->_cached_index)
                               || obj != _entries.at(obj->_cached_index).operator->(),
                           __FUNCTION__,"Object must be the same");

                _entries.at(obj->_cached_index).reset();

            } else{
                obj->_released = true;
            }
        });
    }

    static std::shared_ptr<ModelBaseData<PARAMS>> createNonHash(const PARAMS &p){

        auto data = MODEL::createData(p);

        std::shared_ptr<ModelBaseData<PARAMS>> shPtr = createRawModel(data);

        if(MODEL::UseCache()){
            bool didUpdate = false;
            if constexpr (PARAMS::hasPrimary()){
                if(auto primary_key = data->_params.primaryKey()){
                    QMutexLocker locker(&_entriesLock);
                    for(auto & e : _entries){
                        if(primary_key->value() == e->_params.primaryKey()->value()){
                            data->_cached_index = e->_cached_index.load();
                            e.swap(shPtr);
                            didUpdate = true;
                            break;
                        }
                    }
                }
            }else if(auto unique = data->_params.firstUniqueAttribute()){
                QMutexLocker locker(&_entriesLock);
                for(auto & e : _entries){
                    if(unique->value() == e->_params.firstUniqueAttribute()->value()){
                        data->_cached_index = e->_cached_index.load();
                        e.swap(shPtr);
                        didUpdate = true;
                        break;
                    }
                }
            }

            if(!didUpdate){

                QMutexLocker locker(&_entriesLock);
                if((_entries.max_size() - _entries.size() )== 0 ){
                    _entries.reserve(_entries.size() + ModelBaseImpl::RESERVED_VEC_SIZE);
                }
                data->_cached_index = _entries.size();

                _entries.emplace_back(std::shared_ptr<ModelBaseData<PARAMS>>(data,free_func));
            }

        }

        return shPtr;
    }

    template<typename T>
    static auto safeEntries(std::function<T(const typename std::vector<std::shared_ptr<ModelBaseData<PARAMS>>> &)> fn){
        QMutexLocker locker(&_entriesLock);
        return fn(ModelBase<MODEL,PARAMS>::_entries);
    }

    static auto retriveFromCache(const QMap<QString, QVariant> & map){

        if(!MODEL::UseCache()){
            return MODEL::Default();
        }

        const quint64 now  = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

        if(_entriesLock.tryLock(std::min(MODEL::UseCache(),quint64(2'000)))){ //two second

            const QScopeGuard qMutexUnlocker ([]{  _entriesLock.unlock(); });

            for(auto & modelBaseDataShPtr : ModelBase<MODEL,PARAMS>::_entries){
                if(!modelBaseDataShPtr){
                    continue;
                }

                if constexpr (PARAMS::hasPrimary()){
                    if(auto primary_key = modelBaseDataShPtr->_params.primaryKey()){
                        auto key_value_itr = map.find(primary_key->name());
                        if(key_value_itr != map.end()){
                            if(primary_key->value() ==    key_value_itr.value()){
                                quint64 ct = modelBaseDataShPtr->_params._existInDataBase;
                                if((ct - now ) > MODEL::UseCache()){
                                    return MODEL::Default();
                                }
                            }
                        }
                    }
                }else if constexpr (auto unique = modelBaseDataShPtr->firstUniqueAttribute()){
                    auto key_value_itr = map.find(unique->name());
                    if(key_value_itr != map.end()){
                        if(unique->value() == key_value_itr.value()){
                            quint64 createdTime = modelBaseDataShPtr->_params._existInDataBase;
                            if((createdTime - now ) > MODEL::UseCache()){
                                return MODEL::Default();
                            }
                        }
                    }
                }else{
                    quint64 createdTime = modelBaseDataShPtr->_params._existInDataBase;
                    if((createdTime - now ) > MODEL::UseCache()){
                        modelBaseDataShPtr.reset();//reset reached end of life
                    }
                    continue;
                }

                auto sh_data_ptr = MODEL::createRawModel(modelBaseDataShPtr.get());

                return MODEL(sh_data_ptr);
            }
        }
        return MODEL::Default();
    }

    static ModelBaseData<PARAMS> * createData(const PARAMS & p) {
        return new ModelBaseData<PARAMS>(p);
    }


private:
    std::shared_ptr<ModelBaseData<PARAMS>>  _temp_sh_data_ptr;
    std::shared_ptr<ModelBaseData<PARAMS>>  _sh_data_ptr;
    std::weak_ptr<ModelBaseData<PARAMS>>    _w_data_ptr;

    static QByteArray        _hash;
    static QReadWriteLock    _hashLock;
    static std::vector<std::shared_ptr<ModelBaseData<PARAMS>>> _entries;
    static QMutex    _entriesLock;

    static std::vector<std::type_info> all_registered_models;

};

template<typename MODEL ,typename PARAMS>
std::vector<std::shared_ptr<ModelBaseData<PARAMS>>> ModelBase<MODEL,PARAMS>::_entries;

template<typename MODEL ,typename PARAMS>
QReadWriteLock  ModelBase<MODEL,PARAMS>::_hashLock;

template<typename MODEL ,typename PARAMS>
QMutex  ModelBase<MODEL,PARAMS>::_entriesLock;

template<typename MODEL ,typename PARAMS>
QByteArray ModelBase<MODEL,PARAMS>::_hash;

///////////////////////////////////////////////////////////////////////////////////////////////////
