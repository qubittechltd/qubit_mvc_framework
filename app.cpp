#include "qubit_mvc_framework/config/app.h"


QTimeZone QUBIT_MVC_APP::timezone() const volatile{
    return QTimeZone::utc();
}
QString QUBIT_MVC_APP::database_datetime_format() const volatile{
    return "yyyy-MM-dd hh:mm:ss";
}

int QUBIT_MVC_APP::job_queue_limit() const volatile{
    return 5;
}

int QUBIT_MVC_APP::tries() const volatile{
    return 5;
}

int QUBIT_MVC_APP::retry_after() const volatile{
    return 5;
}

int QUBIT_MVC_APP::model_cache_time() const volatile{
    return 5000;
}
