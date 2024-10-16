#pragma once
#ifndef SYSTEM_SETTINGS_H
#define SYSTEM_SETTINGS_H

#include "QJsonDocument"
#include "QJsonObject"
#include <QByteArray>
#include <QDataStream>
#include <QMetaType>
#include <QSharedPointer>
#include <QtGlobal>
#include "utilities/qrtdatabase.h"
#include "QReadWriteLock"
#include "QAtomicInteger"
#include "QMetaObject"
#include "QMetaProperty"
#include "QReadLocker"


#define system_property(T,V) \
Q_PROPERTY(T V READ get_##V WRITE set_##V NOTIFY notify_##V ) \
                                                            \
    public : T get_##V() {                                  \
        volatile const QReadLocker locker(& V##_lock);      \
        return V;                                           \
}                                                       \
                                                            \
    Q_SIGNAL void notify_##V( T V );    \
                                        \
                                        \
    public : void opt_set_##V(const T & v,bool setDatabase= false) {         \
        if (this->V != v ){                          \
            V##_lock.lockForWrite();                 \
            V = v;                                   \
            V##_lock.unlock();                       \
            if(setDatabase){                         \
                auto & db = QRTDatabase::instance(); \
                auto home= db.getHomeRefHandle();    \
                home.Child("" #V "").SetValue(v);    \
            }                                        \
            emit notify_##V(v);                      \
        }                                            \
    }                                                \
                                                     \
    public : void set_##V(const T & v) {             \
        opt_set_##V(v,true);                         \
    }                                                \
                                                     \
    private: QReadWriteLock V##_lock;                \
    protected: T V;                                  \
    public :                                         \


             namespace SsvUI {
    class Setting;
};

class  SettingParams : public QObject{
    Q_OBJECT
    friend class SsvUI::Setting;
public:
    explicit SettingParams(QObject * parent = nullptr) : QObject(parent) {

    }

    explicit SettingParams(const SettingParams& other,QObject * parent = nullptr) : QObject(parent) {
        auto size = staticMetaObject.propertyCount();
        for (int i = 0; i <  size; ++i){
            auto a = staticMetaObject.property(i).read(&other);
            staticMetaObject.property(i).write(this,a);
        }
        this->stamp = other.stamp;
    }

    qint64  stamp=0;
    system_property(std::string,server_port);
    system_property(std::string,db_username);//
    system_property(std::string,db_password);//
    system_property(std::string,db_hostname);//
    system_property(std::string,db_port);//
    system_property(std::string,db_name);//

    inline static SettingParams & current(){
        static SettingParams s ;
        return s;
    }

    static SettingParams & c(){
        return current();
    }

    bool operator==(const SettingParams& other) const {
        auto size = metaObject()->propertyCount();
        for (int i = 0; i <  size; ++i){
            auto a = other.metaObject()->property(i).read(this);
            auto b = metaObject()->property(i).read(&other);
            if(a != b){
                return false;
            }
        }
        return true;
    }

    bool operator!=(const SettingParams& other) const {
        return !(*this == other);
    }


    SettingParams& operator=(const SettingParams& other) {
        auto size = metaObject()->propertyCount();
        for (int i = 0; i <  size; ++i) {
            auto v = other.metaObject()->property(i).read(&other);
            metaObject()->property(i).write(this,v);
        }
        this->stamp = other.stamp;
        return *this;
    }

};



#endif // SYSTEM_SETTINGS_H
