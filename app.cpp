#include "app.h"

const QTimeZone app::timezone = QTimeZone::utc();
const QString   app::database_datetime_format="yyyy-MM-dd hh:mm:ss" ;
const int       app::job_queue_limit = 5 ;
const int       app::tries = 5 ;
const int       app::retry_after = 5 ;
const int       app::model_cache_time = 5000 ; // 300 milliseconds
