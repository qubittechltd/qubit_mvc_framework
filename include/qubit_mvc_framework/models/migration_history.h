#pragma once
#ifndef JOB_MODEL_H
#define JOB_MODEL_H
#include <qubit_mvc_framework/utilities/model/model.h>
#include <qubit_mvc_framework/utilities/model/model_relationships.h>
#include <qubit_mvc_framework/utilities/system_settings.h>
#include <QSqlQuery>

class JobParams : public ParamsBaseImpl<JobParams>  {
    Q_GADGET
public:
    PRIMARY_KEY_MODEL_ATTRIBUTE_DEFAULT(QByteArray,    id,QUuid::createUuid().toByteArray(QUuid::WithoutBraces));
    MODEL_ATTRIBUTE_DEFAULT(QString,queue,    "Main");
    MODEL_ATTRIBUTE(QByteArray,     payload)
    MODEL_ATTRIBUTE_DEFAULT(int,    attempts,   0 );
    MODEL_ATTRIBUTE(QDateTime,      reserved_at)
    MODEL_ATTRIBUTE(QDateTime,      available_at)

};
Q_DECLARE_METATYPE(JobParams)


class JobModel : public ModelBase<JobModel,JobParams>{
    MODEL_INIT(JobModel,JobParams)
public:
    static bool init_db(QSqlDatabase & default_db){
        return protected_init_table(default_db,DB_INFO({
            .table_name  = "jobs",
            .db_name     = SettingParams::c().get_db_name(),
            .db_hostname = SettingParams::c().get_db_hostname(),
            .db_username = SettingParams::c().get_db_username(),
            .db_password = SettingParams::c().get_db_password(),
            .db_port     = static_cast<quint64>(std::stoi(SettingParams::c().get_db_port()))
        }));
    }

};
#endif // JOB_MODEL_H
