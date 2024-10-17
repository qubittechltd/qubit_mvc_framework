#include "qubit_mvc_framework/config/app.h"


inline QTimeZone QUBIT_MVC_APP::timezone() const{
    return QTimeZone::utc();
}
inline QString QUBIT_MVC_APP::database_datetime_format() const{
    return "yyyy-MM-dd hh:mm:ss";
}

inline int QUBIT_MVC_APP::job_queue_limit() const{
    return 5;
}

inline int QUBIT_MVC_APP::tries() const{
    return 5;
}

inline int QUBIT_MVC_APP::retry_after() const{
    return 5;
}

inline int QUBIT_MVC_APP::model_cache_time() const{
    return 5000;
}
