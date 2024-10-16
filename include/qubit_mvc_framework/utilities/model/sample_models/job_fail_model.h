#ifndef JOB_FAIL_MODEL_H
#define JOB_FAIL_MODEL_H

#include <qubit_mvc_framework/utilities/model/model.h>
#include <qubit_mvc_framework/utilities/model/model_relationships.h>
#include <utilities/system_settings.h>
#include <QSqlQuery>


class JobFailParams : public ParamsBaseImpl<JobFailParams>  {
    Q_GADGET
public:
    PRIMARY_KEY_MODEL_ATTRIBUTE_DEFAULT(QByteArray,  id, QUuid::createUuid().toByteArray(QUuid::WithoutBraces));
    MODEL_ATTRIBUTE(QByteArray, job_id)
    MODEL_ATTRIBUTE_DEFAULT(QString,connection,"database")
    MODEL_ATTRIBUTE_DEFAULT(QString,queue, "Main")
    MODEL_ATTRIBUTE(QByteArray,     payload)
    MODEL_ATTRIBUTE(QString,        exception)
    MODEL_ATTRIBUTE(QDateTime,      failed_at)
    MODEL_ATTRIBUTE_DEFAULT(int,    failed_at_index, 1)
};
Q_DECLARE_METATYPE(JobFailParams)


class JobFailModel : public ModelBase<JobFailModel,JobFailParams>{
    MODEL_INIT(JobFailModel,JobFailParams)
public:
    static bool init_db(QSqlDatabase & default_db){
        return protected_init_table(default_db,DB_INFO({
            .table_name  = "failed_jobs",
            .db_name     = SettingParams::c().get_db_name(),
            .db_hostname = SettingParams::c().get_db_hostname(),
            .db_username = SettingParams::c().get_db_username(),
            .db_password = SettingParams::c().get_db_password(),
            .db_port     = static_cast<quint64>(std::stoi(SettingParams::c().get_db_port()))
        }));
    }
};

#endif // JOB_FAIL_MODEL_H
