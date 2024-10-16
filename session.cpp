#include "qubit_mvc_framework/utilities/session.h"
#include "QCoreApplication"
#include "qubit_mvc_framework/utilities/secure.h"
#include "qubit_mvc_framework/utilities/common_p.h"
#include <QThread>
#include <QUuid>

std::vector<Session*> sessions;//multithreading
bool operator == (const Session *c ,const QByteArray &token){
    return c->token == token;
}
Session::Session(const QByteArray &t,const QDeadlineTimer & rememberMe, QObject *parent)
    : token(t),remember_me(rememberMe),QObject{parent}, _user(UserModel::Default())
{
     ::sessions.push_back(this);
}

Session::~Session()
{
    auto itr = std::find(::sessions.begin(),::sessions.end(),this);
    if(itr != ::sessions.end()){
        ::sessions.erase(itr);
    }
}

const Session * QHttpServerRequest::session() const{
   return Session::user_session(*this,QDeadlineTimer(std::chrono::days(REMEMEMBER_ME_DAYS)));
}

QByteArray generateTokenFromPPK(){
    auto uuidB64 = QUuid::createUuid().toByteArray().toBase64(QByteArray::OmitTrailingEquals|QByteArray::Base64UrlEncoding);
    QByteArray token;
    if(!Secure::instance().signBytes(std::ref(token),uuidB64)){
       return "";
    }
    return token;
}
bool isSessionFromThisInstance(const QByteArray &token){
    //if the app crashes or unexceptly shutdown, all session are invalidated
    if(!Secure::instance().verifySignature(token)){
        return true;
    }
    return false;
}
bool hasAuthTokenBeenTemped(const QByteArray &token){
    if(!Secure::instance().verifySignature(token)){
        return true;
    }
    return false;
}

bool token_has_exipired(const QByteArray &token){
    return false;
}

void Session::set_user(UserModel & u){
    _user = u;
}

std::tuple<Session *, bool> Session::user_session_t(const QHttpServerRequest &request, const QDeadlineTimer &rememberMe)
{
    if(QThread::currentThread() != QCoreApplication::instance()->thread()){
        qWarning()<<"Session::user_session is called in non mainthread";
    }

    QByteArray user_token = std::visit([&] -> QByteArray{//
        for (const auto & p : request.headers()) {
            if(p.first == "Cookie"){
                for (auto &name_value : p.second.split(';')) {
                    name_value = name_value.trimmed();
                    const QByteArray session("session=");
                    if(name_value.startsWith(session)){
                        return name_value.last(name_value.size() - session.size());
                    }
                }
            }
        }
        return "";
    });

    bool skip_search = std::visit([&user_token] -> bool{
        if(user_token.isEmpty())
            return true;
        if(token_has_exipired(user_token))
            return true;
        if(isSessionFromThisInstance(user_token))
            return true;
        return false;
    });

    auto itr = std::find_if((skip_search ? sessions.end(): sessions.begin()),sessions.end(),[&user_token](Session * session){
        if(session->token == user_token){
            session->hits++;
            return true;
        }else if(session->remember_me.hasExpired()){
            session->deleteLater();
        }
        return false;
    });

    bool session_found(itr == sessions.end());
    Session * session = session_found ?
                            new Session(generateTokenFromPPK(),rememberMe) :
                                *itr;

    return std::make_tuple(session,session_found);
}

UserModel Session::user() const {
    return _user;
}
void Session::logout(){
    _user.reset();

}

QByteArray Session::getCookie(){
    auto expirationDateTime = QDateTime::currentDateTime()+std::chrono::milliseconds(remember_me.remainingTime());
    QString expires = "expires=" + expirationDateTime.toString(Qt::RFC2822Date);

    QString path = "path=/";
    QString domain = "" PROJECT_DOMAIN "";
    QString httpOnly = "HttpOnly";
    // QString secure = "secure";
    // QString SameSite="SameSite=None";
    return QString("session=%1; %2; %3; %4; %5;")
                .arg(token,expires,path,domain,httpOnly).toLocal8Bit();
}

std::string Session::prev_page(){
    return history().size() ? "/" : history().back();
}



