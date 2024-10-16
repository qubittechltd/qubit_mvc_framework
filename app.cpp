#include "qubit_mvc_framework/config/app.h"


inline QTimeZone QUBIT_MVC_APP::timezone(){
    return QTimeZone::utc();
}
inline QString QUBIT_MVC_APP::database_datetime_format(){
    return "yyyy-MM-dd hh:mm:ss";
}

inline int QUBIT_MVC_APP::job_queue_limit(){
    return 5;
}

inline int QUBIT_MVC_APP::tries(){
    return 5;
}

inline int QUBIT_MVC_APP::retry_after(){
    return 5;
}

inline int QUBIT_MVC_APP::model_cache_time(){
    return 5000;
}
