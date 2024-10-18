#ifndef QUBIT_MVC_APP_H
#define QUBIT_MVC_APP_H

#include <QString>
#include <QTimeZone>

struct QUBIT_MVC_APP {
    static const QUBIT_MVC_APP * instance(){
        return nullptr;
    }
    QTimeZone timezone() const;
    QString database_datetime_format() const;
    int job_queue_limit() const;
    int tries() const;
    int retry_after() const;
    int model_cache_time() const;
};





#endif // QUBIT_MVC_APP_H
