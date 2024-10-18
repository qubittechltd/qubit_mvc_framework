#ifndef DB_INFO_H
#define DB_INFO_H

#include <QReadWriteLock>
#include <QString>

class DB_INFO{
public:
    struct VAR{
        std::string table_name;
        std::string db_name;
        std::string db_hostname;
        std::string db_username;
        std::string db_password;
        quint64 db_port=0;
    };
    DB_INFO(){}

    DB_INFO(const DB_INFO::VAR & v): var(v),initialized(true){}

    DB_INFO & operator=(const DB_INFO & info) = delete;
    DB_INFO & operator=(DB_INFO && info) = delete;

    void set_var(DB_INFO & info){
        DB_INFO tmp;
        info.dataLock.lockForRead();
        tmp.initialized = info;
        tmp.var = info.var;
        info.dataLock.unlock();

        dataLock.lockForWrite();
        initialized = tmp;
        var = tmp.var;
        dataLock.unlock();

    }

    operator bool() const {
        return initialized;
    }

    DB_INFO::VAR getInfo() {
        dataLock.lockForRead();
        DB_INFO::VAR _v = var;
        dataLock.unlock();

        return std::move(_v);
    }

private:
    VAR var;
    std::atomic_bool initialized = false;
    QReadWriteLock dataLock;
};
#endif // DB_INFO_H
