#ifndef QUBIT_MVC_APP_H
#define QUBIT_MVC_APP_H

#include <QString>
#include <QTimeZone>

struct QUBIT_MVC_APP{
    static QTimeZone timezone();
    static QString database_datetime_format();
    static int job_queue_limit();
    static int tries();
    static int retry_after();
    static int model_cache_time();
};





#endif // QUBIT_MVC_APP_H
