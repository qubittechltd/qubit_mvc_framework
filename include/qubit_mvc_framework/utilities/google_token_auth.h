#ifndef GOOGLE_TOKEN_AUTH_H
#define GOOGLE_TOKEN_AUTH_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QEventLoop>
#include <QTimer>
#include <QString>
#include <QDebug>
#include <QSslKey>
#include <QSslCertificate>
#include <QReadWriteLock>
#include <QTimer>
#include <QElapsedTimer>
#include <QDeadlineTimer>
#define SSV_MANAGER_SET_TRANSFER_TIMEOUT 30'000

struct CERT_KEY{
    QString    kid; //key id
    QByteArray pem;//algorithim
    QByteArray base64;
    QSslCertificate    cert;
    bool operator==(const CERT_KEY& other) const;
    CERT_KEY();
    ~CERT_KEY();
    qint64 aliveTimeSec() const;
private:
    QElapsedTimer elaspsedTimer;
};

struct User{
    QString email;
    QString given_name;
    QString family_name;

    bool isEmpty() const {
        return email.isEmpty() && given_name.isEmpty() && family_name.isEmpty();
    }
};

QElapsedTimer  timeSinceLastKeyUpdate;
QDeadlineTimer retryExponentialBackingTimer(0);
QReadWriteLock key_lock;
std::vector<CERT_KEY> cert_keys;
int key_cache_secs = 43'200;

User  retrieveGoogleTokenInfo(const QByteArray& id_token) {

    enum EVENTS{
        ERROR = -2,
        TIMEOUT = -1,
        SUCCESS = 0
    };

    int duration = 30'000;
    QByteArray tag;

    QNetworkAccessManager manager;  // Network access manager
    manager.setAutoDeleteReplies(true);

    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1_2);

    QNetworkRequest request(QUrl("https://oauth2.googleapis.com/tokeninfo"));
    request.setSslConfiguration(config);
    request.setHeader(QNetworkRequest::UserAgentHeader,"PostmanRuntime/7.42.0");
    request.setRawHeader("Accept","*/*");
    request.setRawHeader("Content-Type","application/json");

    QTimer timer;
    timer.setTimerType(Qt::VeryCoarseTimer);
    timer.setSingleShot(true);
    timer.setInterval(duration);


    int trials=0;
    User user;
    QNetworkReply * reply;
    QSharedPointer<QEventLoop> event;

AGAIN:

    if(trials){
        int exponential = (2 * std::pow(2,trials)) * 1'000;
        qDebug()<<"Retrying exponential pause" << exponential;
        QEventLoop loop;
        QTimer::singleShot(exponential,&loop,&QEventLoop::quit);
        if(loop.exec())
            goto ENDING;
        qDebug()<<"Retrying resumed";
    }
    event = QSharedPointer<QEventLoop>::create();
    if(duration){
        timer.callOnTimeout(event.data(),[=]() {
            if(!tag.isEmpty())
                qDebug()<<"ACQUIRING TIMEOUT (TAG : "+tag+" TRIALS : "+QString::number(trials)+")";
            if(event->isRunning())
                event->exit(EVENTS::TIMEOUT);
        },Qt::DirectConnection);
        timer.start();
    }

    reply = manager.post(request,
                         "{\"id_token\" : \""+id_token+"\" }"
                         );


    QObject::connect(reply, &QNetworkReply::finished,event.data(),[ reply,event,&user,&trials](){
        QEventLoopLocker unlocker(event.data()) ;

        QByteArray response_data = reply->readAll();

        QJsonDocument jsonResponse = QJsonDocument::fromJson(response_data);

        if (jsonResponse.isNull()) {
            if(trials > 2){
                qDebug() << "Failed to parse JSON response";
            }
            qDebug()<<"Error parsing json, retry";
            event->exit(EVENTS::ERROR);
            return;

        } else {
            QJsonObject jsonObject = jsonResponse.object();
            user.email       = jsonObject["email"].toString();
            user.given_name  = jsonObject["given_name"].toString();
            user.family_name = jsonObject["family_name"].toString();

        }
    });


ENDING:
    switch (event->exec()) {
    case EVENTS::TIMEOUT: break;
    case EVENTS::ERROR:{
        trials++;
        if(trials < 3){
            qDebug()<<"Retrying Again";
            goto AGAIN;
        }
        break;
    }
    default:
        timer.stop();
        if(!tag.isEmpty())
            qDebug()<<"ACQUIRING (TAG : "+tag+" )";
        break;
    }

    return user;

}

User  decryptGoogleTokenInfo(const QByteArray& id_token){
    return User();
}

bool pullCertKey()
{
    QEventLoop loop;
    bool success=false;
    QNetworkAccessManager manager(&loop);
    manager.setAutoDeleteReplies(true);
    manager.setTransferTimeout(SSV_MANAGER_SET_TRANSFER_TIMEOUT);
    QNetworkRequest request(QUrl("https://www.googleapis.com/oauth2/v1/certs"));
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1_2);
    request.setSslConfiguration(config);
    request.setHeader(QNetworkRequest::UserAgentHeader,"PostmanRuntime/7.42.0");
    request.setRawHeader("Accept","*/*");
    request.setRawHeader("Content-Type","application/json");

    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished,&loop,[&,reply](){
        QEventLoopLocker l(&loop);

        auto data = reply->readAll().trimmed();
        auto dataJsonDoc =QJsonDocument::fromJson(data);
        if(dataJsonDoc.isObject()){
            auto keysJson =dataJsonDoc.object();
            for (auto key_value_tr = keysJson.begin(); key_value_tr != keysJson.end(); ++key_value_tr) {

                CERT_KEY cert_key;
                cert_key.kid     = key_value_tr.key() ;
                cert_key.pem     = key_value_tr.value().toString().toUtf8();

                cert_key.cert = QSslCertificate(cert_key.pem,QSsl::Pem);
                if (cert_key.cert.isNull()){
                    qDebug("cannot initiate QSslKey EC");
                    continue;
                }
                if(key_lock.tryLockForWrite()){
                    bool found=false;
                    auto itr = cert_keys.begin();
                    while (itr != cert_keys.end()){
                        auto & stored_key = *itr;
                        if(cert_key == stored_key){
                            found=true;
                            break;
                        }
                        if(stored_key.aliveTimeSec() > key_cache_secs){
                            itr=cert_keys.erase(itr);
                            continue;
                        }
                        itr++;
                    }
                    if(!found){
                        cert_keys.push_back(cert_key);
                        qDebug("Aquired Key: %s",  cert_key.kid.toStdString().c_str());
                    }
                    key_lock.unlock();
                }else{
                    loop.exit(-1);
                    return;
                }

            }

        }
        timeSinceLastKeyUpdate.restart();
    });
    success = loop.exec() == 0;
    return success;
}


CERT_KEY::CERT_KEY(){
    kid=(0);
    elaspsedTimer.start();
}

CERT_KEY::~CERT_KEY(){
}

qint64 CERT_KEY::aliveTimeSec() const {
    return elaspsedTimer.elapsed()/1000;
}

bool CERT_KEY::operator==(const CERT_KEY &other) const {
    return kid==other.kid && pem==other.pem && base64==other.base64;
}
#endif // GOOGLE_TOKEN_AUTH_H
