#include <qubit_mvc_framework/utilities/queues.h>

void Queue::process_jobs(){

    auto now = QDateTime::currentDateTimeUtc();
    auto now_str = now.toString(QUBIT_MVC_APP::instance()->database_datetime_format());

    auto updated_rows = JobModel::whereNull("reserved_at")
                            ->orWhere("available_at",">=",now_str)
                            ->orderBy("created_at")
                            ->limit(QUBIT_MVC_APP::instance()->job_queue_limit())
                            ->update({
                                .reserved_at = now
                            });

    JobModel::Collection jobs;

    if(updated_rows){
        jobs = JobModel::where("reserved_at",'=',now_str)
        ->orWhere("available_at",">=",now_str)
            ->where("queue",">=",_name)
            ->orderBy("created_at")
            ->limit(std::min(updated_rows,QUBIT_MVC_APP::instance()->job_queue_limit()))
            ->get();
    }

    for(auto & job : jobs){

        QByteArray serializedData = job->payload;
        QJsonDocument doc = QJsonDocument::fromJson(serializedData);
        QJsonObject data = doc.object();
        if (!data.isEmpty()) {
            QString className = data["className"].toString();
            if(!className.isEmpty()){
                auto meta = QMetaType::fromName(className.toUtf8());
                if(meta.isValid()){
                    void * jobVoidPtr = meta.create();
                    if(auto jobInstance = static_cast<JobImpl*>(jobVoidPtr)){
                        jobInstance->set_data(data);
                        try {
                            jobInstance->handle();
                            job.deleteLater();
                            delete jobInstance;
                        } catch (std::exception & e) {
                            qDebug("Exception occured while executing job name : %s , id : %s",e.what(),job->id().toStdString().c_str());

                            job->attempts = job->attempts() + 1;

                            if(job->attempts() < (QUBIT_MVC_APP::instance()->tries() + 1)){

                                job->reserved_at = QDateTime();
                                job->available_at = QDateTime::currentDateTimeUtc().addMSecs(2 ^ job->attempts());
                                job.flush();

                            }else{

                                QByteArray id = job->id;
                                QByteArray payload = job->payload;
                                job.deleteLater();

                                JobFailModel failed_job = JobFailModel::create(JobFailModel::Params{
                                    .job_id     = id,
                                    .payload    = payload,
                                    .exception  = e.what(),
                                    .failed_at  = QDateTime::currentDateTime()
                                });
                                failed_job.flush();
                            }
                        }
                    }
                }

            }

        }
    }
}
