#pragma once
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QThread>
#include <qubit_mvc_framework/config/app.h>
#include "qubit_mvc_framework/utilities/model/model_base_impl.h"
#include "qubit_mvc_framework/utilities/model/table_attribute_base.h"
#include "qubit_mvc_framework/utilities/model/table_attribute_impl.h"
#include "qubit_mvc_framework/utilities/model/db_info.h"
#include "qubit_mvc_framework/utilities/model/sql_query.h"

template<typename T>
struct is_string_type : std::false_type {};
template<>
struct is_string_type<std::string> : std::true_type {};
template<>
struct is_string_type<QString> : std::true_type {};
template<>
struct is_string_type<QByteArray> : std::true_type {};
template<>
struct is_string_type<const char*> : std::true_type {};

template<typename M ,typename P>
class ModelBase;

template<typename MODEL ,typename PARAMS>
class  ModelBaseImplSqlOrm : public ModelBaseImpl{
    friend ModelBaseImpl;
    template<typename M ,typename P>
    friend class ModelBase;
    typedef ModelBase<MODEL,PARAMS> Derived;
    template<class T>
    friend struct OrmEnd;
    template<typename M ,typename P>
    friend class Migrations;
    typedef ModelBase<MODEL,PARAMS>        DERIVED;
    template<typename M>
    friend struct Eloquent::OrmEndUpdate;
    template<typename M>
    friend struct Eloquent::OrmEndTruncateCPO;
#define _DERIVED_PTR       ((DERIVED*)this)
public:
    typedef MODEL*                         Ptr;
    typedef std::weak_ptr<MODEL>           WPtr;
    typedef std::shared_ptr<MODEL>         ShPtr;
    typedef std::vector<MODEL>             Collection;
    typedef Collection                     Vector;
    typedef std::vector<PARAMS>            ParamsVec;
    typedef PARAMS                         Params;

    QJsonDocument toJson(){
        auto this_model = (MODEL *)this;

        QJsonObject obj;
        this_model->safe_data_ptr([&](auto data_ptr){
            if(data_ptr){
                for(auto s  : data_ptr->_params.getTableAttributesVec()) {
                    QVariant qvariant = s->value();
                    switch (qvariant.metaType().id()) {
                    case QMetaType::QDateTime:{
                        auto dateTime = qvariant.value<QDateTime>();
                        obj.insert(s->name(),dateTime.toString(QUBIT_MVC_APP::database_datetime_format()));
                        break;
                    }
                    default:
                        obj.insert(s->name(),qvariant.toJsonValue());
                    }
                }
            }
        });

        return QJsonDocument(obj);
    };

    static Collection where(const PARAMS &params){

        auto info = get_db_Info();
        auto builder = Eloquent::SqlBuilder::all<MODEL>()
                           ->from(info.db_name,info.table_name)
                           ->whereBuilder();

        for (TableAttributeImpl* inf : params.getTableAttributesVec()) {
            if((!inf->isBaseAtrribute() && !inf->_isDefaulted) && inf->isValid()){
                builder = builder->where(inf->name(), "=", inf->value())
                                 ->clone();
            }
        }

        return builder->get();

    }

    auto update(const QMap<QString, QVariant> & map){
        auto this_model = (MODEL *)this;
        this_model->safe_data_ptr([&](auto data_ptr){
            if(data_ptr){
                for(auto s  : data_ptr->_params.getTableAttributesVec()) {
                    auto name = s->name();
                    if(map.contains(name)){
                        s->set_value(map.value(name));
                    }
                }
            }
        });
    }

    template<typename T1,typename T2>
    static auto where(T1 column ,T2 op,QVariant value){
        auto info = get_db_Info();
        return Eloquent::SqlBuilder::all<MODEL>()
            ->from(info.db_name,info.table_name)
            ->where(column,op,value)
            ->clone();
    }

    template<typename T1>
    static auto whereNull(T1 column){
        auto info = get_db_Info();
        return Eloquent::SqlBuilder::all<MODEL>()
            ->from(info.db_name,info.table_name)
            ->whereNull(column)
            ->clone();
    }

    static auto truncate(){
        auto info = get_db_Info();
        return Eloquent::SqlBuilder::all<MODEL>()
            ->from(info.db_name,info.table_name)
            ->truncate();
    }

    static auto count(){
        auto info = get_db_Info();
        return Eloquent::SqlBuilder::all<MODEL>()
                ->from(info.db_name,info.table_name)//safety
                ->count();
    }

    static auto firstOrCreate(const PARAMS &params){
        auto info = get_db_Info();
        auto builder = Eloquent::SqlBuilder::all<MODEL>()
                           ->from(info.db_name,info.table_name)
                           ->whereBuilder();

        // skip here all defalted values
        for (TableAttributeImpl* inf : params.getTableAttributesVec()) {
            if(!inf->isBaseAtrribute() && !inf->_isDefaulted){
                builder = builder->where(inf->name(), "=", inf->value())
                                 ->clone();
            }
        }

        auto models = builder->limit(1)
                             ->get();

        if(models.size()){
            return  models.front();
        }
        auto model = MODEL::create(params);
        model.flush();
        return model;
    }

    auto deleteLater(){
        auto this_model = (MODEL *)this;

        this_model->safe_data_ptr([&](auto data_ptr){
            if(data_ptr){
                auto deleted_at = (*this_model)->deleted_at.value();
                if(!deleted_at.isValid()){
                    (*this_model)->deleted_at  = QDateTime::currentDateTimeUtc();
                }
            }
        });

        this_model->relax_ref();

    }

    static bool protected_init_db(QSqlDatabase &db,int trials = 0){
        return  protected_init_db(db,ModelBaseImpl::dummy_db_info,trials);
    }

    static bool protected_init_db(QSqlDatabase &db,DB_INFO & _db_info,int trials = 0){
        if (_db_info) {
            if(!db_info){
                db_info.set_var(_db_info);
            }
        }
        auto info = get_db_Info();

        const QByteArray  connectionName = "db_thread_"+QByteArray::number((qint64) QThread::currentThread());

        if(QSqlDatabase::contains(connectionName)){
            db= QSqlDatabase::database(connectionName)  ;
            if(!db.isOpen()){
                if (!db.open()){

                    qWarning()<<"Open Database MySQL error:" + db.lastError().text()
                              <<"error code:"<<db.lastError().nativeErrorCode();
                    // default_db.removeDatabase(connectionName);
                    // default_db.close();
                    if(trials < 3){ //exponential pause
                        int exponential = (2 * std::pow(2,trials)) * 1'000;
                        QEventLoop loop;
                        QTimer::singleShot(exponential,&loop,&QEventLoop::quit);
                        if(loop.exec())
                            return false;
                    }else{
                        return false;
                    }


                    return protected_init_db(db,++trials);
                }

            }
        }else{
            db = QSqlDatabase::addDatabase("QMYSQL",connectionName);
            db.setDatabaseName(info.db_name);
            db.setHostName(info.db_hostname);
            db.setPort(info.db_port);
            if (!db.open(info.db_username,info.db_password)){
                qWarning()<<"Open Database MySQL error:" + db.lastError().text()
                          <<"error code:"<<db.lastError().nativeErrorCode();
                return false;
            }
        }

        return true;
    }

    static bool protected_init_table(QSqlDatabase & db){
        return protected_init_db(db,dummy_db_info);
    }

    static bool protected_init_table(QSqlDatabase & db,DB_INFO & info ){
        return protected_init_db(db,info);
    }

    static bool protected_init_table(QSqlDatabase & db,DB_INFO && info ){
        return protected_init_db(db,info);
    }

    virtual bool flush() {
        auto this_model = (MODEL *)this;

        return this_model->template safe_data_ptr<bool>([&](auto data_ptr){
            if(!data_ptr){
                //already released
                return false;
            }

            QWriteLocker lock_table_attribute_base(&data_ptr->table_attribute_base._dataLock);
            auto attribute_flags = data_ptr->table_attribute_base._attribute_flags;
            if(!attribute_flags) {// we don't need to save/update
                return true;
            }

            auto deleted_at =  &(this_model->params()->deleted_at);
            bool includes_deleting = (attribute_flags & deleted_at->_flag) && deleted_at->isValid();

            data_ptr->table_attribute_base._attribute_flags = 0;
            bool isInsertProcess(data_ptr->table_attribute_base._savedStamp==0 &&
                                 !this_model->params()->_existInDataBase);
            auto dataStamp = data_ptr->table_attribute_base._dataStamp;
            lock_table_attribute_base.unlock();

            auto primary_key = primaryKey();
            QString name_placeholders;
            QString names(primary_key  ? " " +primary_key->name() : "" );
            QString values(primary_key ? " :"+primary_key->name() : "");
            QString conditions(primary_key ? primary_key->name()+" = :"+primary_key->name() : "");

            QMap<QString,QVariant> toUpdate(primary_key ?
                QMap<QString,QVariant>{{ ":"+primary_key->name()+"",
                    primary_key->value()}} : QMap<QString,QVariant>{});

            for (auto s  : data_ptr->_params.getTableAttributesVec()) {
                auto name = s->name();
                auto data = s->value();
                if(s->isOfType(typeid(QDateTime))){
                    if(!data.isNull()){
                        data = QUBIT_MVC_APP::database_datetime_format().isEmpty() ? s->template value<QDateTime>().toString() :
                                   s->template value<QDateTime>().toString(QUBIT_MVC_APP::database_datetime_format());
                    }
                }
                if(primary_key == s || !data.isValid()){
                    continue;
                }

                bool about_to_end = s == data_ptr->_params.getTableAttributesVec().back();
                if(attribute_flags & s->_flag){
                    names  += QString( names.isEmpty()  ? "" : ", "  + name);
                    values += QString( values.isEmpty() ? "" : ", :" + name);
                    if(!name_placeholders.isEmpty())
                        name_placeholders += ", ";
                    name_placeholders += QString(name +" = :"+name);
                    toUpdate.insert(":"+name,data);
                }else if(!primary_key){
                    if(name == "updated_at"){//might be updated by another server
                        continue;
                    }
                    if(!conditions.isEmpty())
                        conditions += " AND ";

                    if(data.isNull()){
                        conditions += QString(name +" IS NULL");
                    } else {
                        conditions += QString(name +" = :"+name);
                        toUpdate.insert(":"+name,data);
                    }
                }
            }

            if(names.isEmpty() || values.isEmpty()){
                return true; // nothing to update
            }

            const QString query_str = std::visit([&]{
                auto info = get_db_Info();
                if(includes_deleting){
                    if(!MODEL::UseSoftDelete()){
                        return QString("DELETE FROM `%1`.`%2` WHERE %4 LIMIT 1")
                        .arg(info.db_name,info.table_name,conditions);
                    }
                }

                if(isInsertProcess){
                    return QString("INSERT INTO `%1`.`%2`( %3 ) VALUES ( %4 )")
                    .arg(info.db_name,info.table_name,names,values);
                }

                return QString("UPDATE `%1`.`%2` SET %3 WHERE %4 LIMIT 1")
                    .arg(info.db_name,info.table_name,name_placeholders,conditions);
            });

            QSqlDatabase default_db;
            if(!MODEL::init_db(default_db)){
                return false;
            }

            QSqlQuery sqlQuery(default_db);
            sqlQuery.prepare(query_str);
            for (auto k_v : toUpdate.asKeyValueRange()) {
                sqlQuery.bindValue(k_v.first , k_v.second);
            }
            if(!sqlQuery.exec()){
                if(!data_ptr->table_attribute_base.increment_failed()){
                    Eloquent::Query::printErrorQuery(sqlQuery,toUpdate);
                }
                return false;
            }

            data_ptr->_params._existInDataBase=QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

            lock_table_attribute_base.relock();
            data_ptr->table_attribute_base._savedStamp    = dataStamp;
            if(dataStamp == data_ptr->table_attribute_base._dataStamp){
               data_ptr->table_attribute_base._attribute_flags = 0;
            }
            return true;
        });

    };

    static Collection all(){
        auto info = get_db_Info();
        return Eloquent::SqlBuilder::all<MODEL>()
            ->from(info.db_name,info.table_name)
            ->get();
    }

protected:
    inline auto primaryKey() const{
        auto this_model = (MODEL *)this;
        return this_model->template safe_data_ptr<TableAttributeImpl*>([&](auto data_ptr) -> TableAttributeImpl* {
            if constexpr (MODEL::Params::hasPrimary()){
                if(data_ptr){
                    return data_ptr->_params.primaryKey();
                }
            }
            return nullptr;
        });
    }
    static auto get_db_Info(){
        QSqlDatabase db;
        if(!db_info && !MODEL::init_db(db)){
            return dummy_db_info.getInfo();
        }
        return db_info.getInfo();
    }
private:

    static DB_INFO db_info;
};

template<typename MODEL ,typename PARAMS>
DB_INFO ModelBaseImplSqlOrm<MODEL,PARAMS>::db_info;



