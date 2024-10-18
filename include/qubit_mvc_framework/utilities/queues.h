#ifndef QUEUES_H
#define QUEUES_H

#include <QObject>
#include <QSharedPointer>
#include <QDebug>
#include <QQueue>
#include <QSharedPointer>
#include <QThread>
#include <QReadWriteLock>
#include <QReadLocker>
#include <QTimer>
#include <QDateTime>
#include <qubit_mvc_framework/models/job_fail_model.h>
#include <qubit_mvc_framework/models/job_model.h>
#include <qubit_mvc_framework/config/app.h>

class JobImpl {
    friend class Queue;
protected:
    QJsonObject _data;
    void set_data(const QJsonObject & data){  _data=data; }
public:
    virtual ~JobImpl() = default;
    virtual void handle() = 0;
    virtual QByteArray serialize() const = 0;
};




template<typename T >
class Job  : public  JobImpl {
    // A template that checks for the presence of a constructor
    template <typename TT, typename = void>
    struct has_default_constructor : std::false_type {};

    template <typename TT>
    struct has_default_constructor<TT, std::void_t<decltype(TT())>> : std::true_type {};


protected:
    Job(){}

    explicit Job(QByteArray data) : JobImpl(){

        static_assert(has_default_constructor<T>::value, "Error:  DerivedClassName(){} constructor must be implemented.");

        qRegisterMetaType<T>(typeid(T).name());

        set_data(QJsonDocument::fromJson(data).object());
    }

public:

    template<typename T1 , typename = std::enable_if_t<std::is_base_of_v<Job<T1>,T>>>
    void dispatch(const T1 & job){
        dispatchDelay(job,0);
    }

    template<typename T1 , typename = std::enable_if_t<std::is_base_of_v<Job<T1>,T>>>
    void dispatchDelay(const T1 & job, int delayInSeconds) {

        QDateTime execute_at;
        if(delayInSeconds > 0){
            auto now = QDateTime::currentDateTime();
            execute_at = now.addSecs(delayInSeconds);
        }

        QJsonObject payload;
        payload["className"] = typeid(T).name();
        payload["data"]  =  QString(job.serialize());

        JobModel new_job = JobModel::create(JobModel::Params{
            .payload = QJsonDocument(payload).toJson(QJsonDocument::Compact),
            .available_at =  execute_at
        });

        new_job.flush();
    }

};

class Queue : public QObject
{
    Q_OBJECT

public:

    static Queue& instance() {
        static Queue instance("Main");
        return instance;
    }

public slots:

    void startProcessing(){
        qDebug() << "QueueWorker startProcessing()";
        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Queue::process_jobs);
        timer->start(5000); // Run every 5 seconds
    }

private slots:
    void process_jobs();
protected:
    explicit Queue(QString name,QObject *parent = nullptr) : QObject(parent), _name(name) {}
    QString _name;
};

#endif // QUEUES_H
