#ifndef SESSION_H
#define SESSION_H

#include <QDeadlineTimer>
#include <QtHttpServer/QHttpServerRequest>
#include <QObject>
#include <concepts>

#include "qubit_mvc_framework/models/user_model_impl.h"
#include "qubit_mvc_framework/utilities/common_p.h"
#include "qubit_mvc_framework/utilities/controller.h"

class ControllerDeletedException : std::exception{};
class ControllerTimeOutException : std::exception{};
class Session;
template<typename T,typename = std::enable_if_t<std::is_base_of_v<Controller, T>>>
class ControllerSingleTon {
public:

    ControllerSingleTon(T *&& cc):c(std::move(cc)){
        if(!(m_locked = cc->lock.tryLockForWrite(CONTROLLER_TIMEOUT_MSEC))){
           throw ControllerTimeOutException();
        }
        QObject::connect(cc,&QObject::destroyed,std::bind(&ControllerSingleTon::controllerDeleted,this));
    }
    T * operator->(){
        if(c) return c;
        qWarning("Controller was deleted");
        throw ControllerDeletedException();
    }
    ~ControllerSingleTon(){
        if(m_locked){
            m_locked = false;
            operator->()->lock.unlock();
        }
    }
private:
    void controllerDeleted(){
        c = nullptr;
    }
private:
    Q_DISABLE_COPY_MOVE(ControllerSingleTon)
    bool m_locked=false;
    T * c;
};


class Session : public QObject{
    Q_OBJECT
    friend class LoginController;
    friend class RegisterController;
    friend class ResetController;
protected:


    template<typename UserModel>
    void set_user(UserModel & u){
        auto user = new UserModel();
        *user = u;
        _user.reset(user);
    }
    // template<typename ... Args>
    // const auto & history(Args && ... args,bool save_or_all=false){
    //     std::tuple<Args ...> tuple = std::make_tuple(args...);
    //     static QVector<std::tuple<Args ...>> history;
    //     history.emplaceBack(tuple);
    //     return history;
    // }

public:

    static const Session * user_session(const QHttpServerRequest &request,const QDeadlineTimer & rememberMe = QDeadlineTimer(std::chrono::days(REMEMEMBER_ME_DAYS))){
        auto t = user_session_t(request,rememberMe);
        return std::get<0>(t);
    }


    static std::tuple<Session*, bool> user_session_t(const QHttpServerRequest &request,const QDeadlineTimer & rememberMe = QDeadlineTimer(std::chrono::days(REMEMEMBER_ME_DAYS)));

    template<typename UserModel = UserModelImpl>
    UserModel user() const {
        if(!_user){
            return UserModel::Default();
        }
        if constexpr ( std::same_as<UserModel,UserModelImpl> ){
            return *_user.get();
        }else{
            auto user = std::static_pointer_cast<UserModel>(_user);
            return *user.get();
        }
    }

    void logout();

    const QByteArray token;

    QByteArray getCookie();

    const auto & history(const std::string &s=std::string()) const{
        static std::vector<std::string> history;
        if(!s.empty()){
            history.emplace_back(s);
        }
        return history;
    }

    std::string prev_page();

    template<typename T>
    auto instanceOf1() -> typename std::enable_if_t<(std::is_base_of_v<Controller,T>),ControllerSingleTon<T>>{
        for(auto & obj : children()){
            if(auto p = dynamic_cast<T*>(obj)){
                return  std::move((T*)obj);
            }
        }
        return new T(this);
    }

    template<typename T>
    auto instanceOf() -> typename std::enable_if_t<(std::is_base_of_v<Controller,T>),ControllerSingleTon<T>>{
        auto itr = std::find_if(children().begin(),children().end(),[](QObject * obj){
            static const auto & id = typeid(T);
            return (id == typeid(*static_cast<T*>(obj)));
        });
        if(itr == children().end()){
            return new T(this);
        }
        return static_cast<T*>(*itr);
    }

signals:
private:
    explicit Session(const QByteArray &t,const QDeadlineTimer & rememberMe , QObject *parent=nullptr);
    ~Session();
    qint64 hits = 0;
    const QDeadlineTimer remember_me;
    std::shared_ptr<UserModelImpl> _user = nullptr;
};

#endif // SESSION_H
