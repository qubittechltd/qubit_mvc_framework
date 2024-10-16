#ifndef APP_H
#define APP_H

#include <QString>
#include <QTimeZone>

struct app{
    static const QTimeZone timezone ;
    static const QString  database_datetime_format ;
    static const int  job_queue_limit;
    static const int  tries;
    static const int  retry_after;
    static const int  model_cache_time;
};

#endif // APP_H
