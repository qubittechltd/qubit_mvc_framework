#ifndef QUBIT_MVC_APP_H
#define QUBIT_MVC_APP_H

#include <QString>
#include <QTimeZone>

struct QUBIT_MVC_APP {
    static const QUBIT_MVC_APP * instance();
    QTimeZone timezone() const volatile;
    QString database_datetime_format() const volatile;
    int job_queue_limit() const volatile;
    int tries() const volatile;
    int retry_after() const volatile;
    int model_cache_time() const volatile;
};





#endif // QUBIT_MVC_APP_H
