#ifndef MIGRATOR_H
#define MIGRATOR_H
#include <QDebug>
#include <typeinfo>

template<typename M , typename P>
struct Migrator : public P{
    Migrator(){
        qDebug("Migrator<%s,%s> created",typeid(M).name(),typeid(P).name());
        M::migrate_up();
    }
};

template<typename M , typename P>
struct DeMigrator : public P{
    DeMigrator(){
        qDebug("DeMigrator<%s,%s> created",typeid(M).name(),typeid(P).name());
        M::migrate_down();
    }
};
#endif // MIGRATOR_H
