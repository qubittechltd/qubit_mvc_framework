#ifndef QUBIT_MVC_APP_H
#define QUBIT_MVC_APP_H

#include <QString>
#include <QTimeZone>

struct QUBIT_MVC_APP {
    static QUBIT_MVC_APP instance();
    QTimeZone timezone();
    QString database_datetime_format();
    int job_queue_limit();
    int tries();
    int retry_after();
    int model_cache_time();
};





#endif // QUBIT_MVC_APP_H
