#ifndef QRTDATABASE_H
#define QRTDATABASE_H

#include <QObject>
#include <QDebug>
#include <QEventLoop>
#include <QVariantMap>
#include <firebase/database.h>
#include <firebase/app.h>
#include <QTimer>
#include "qubit_mvc_framework/utilities/utilities.h"

using namespace firebase::database ;

class QChildListener : public QObject,  ChildListener
{
    Q_OBJECT
public:
    enum EVENTS{
        NONE         = 1,
        ChildAdded   = 2,
        ChildChanged = 4,
        ChildMoved   = 8,
        ChildRemoved = 16,
        Cancelled    = 32
    };
    Q_ENUM(EVENTS)
signals:
    void activated(QChildListener::EVENTS); //only connect to queued connecttion
    void activatedWithData(QVariant,QChildListener::EVENTS); //only connect to queued connecttion
private:
    QEventLoop * eventLoop=nullptr;
    EVENTS event;
    DatabaseReference   _obj = DatabaseReference();
    DataSnapshot        _snapShot = DataSnapshot();
    DatabaseReference   _databaseReference;
    Error _error = Error::kErrorNone ;
    QString _errorStr;
    bool hold=true;
    void OnChildAdded(const DataSnapshot& snapshot,const char* previous_sibling_key){
        if(!eventLoop)
            return;
        QEventLoopLocker unlocker(eventLoop); Q_UNUSED(unlocker) Q_UNUSED(previous_sibling_key)
        _snapShot = snapshot;
        _obj=snapshot.GetReference();
        _error=Error::kErrorNone;
        hold=false;
        emit activated(event = ChildAdded );
        emit activatedWithData(Utilities::FireBase::toQVariant(snapshot.value()),event = ChildAdded );

    }
    void OnChildChanged(const DataSnapshot& snapshot,const char* previous_sibling_key) {
        if(!eventLoop)
            return;
        QEventLoopLocker unlocker(eventLoop); Q_UNUSED(unlocker) Q_UNUSED(previous_sibling_key)
        _snapShot = snapshot;
        _obj=snapshot.GetReference();
        _error=Error::kErrorNone;
        hold=false;
        emit activated(event = ChildChanged );
        emit activatedWithData(Utilities::FireBase::toQVariant(snapshot.value()),event = ChildChanged );
    }
    void OnChildMoved(const DataSnapshot& snapshot,const char* previous_sibling_key) {
        if(!eventLoop)
            return;
        QEventLoopLocker unlocker(eventLoop); Q_UNUSED(unlocker) Q_UNUSED(previous_sibling_key)
        _snapShot = snapshot;
        _obj=snapshot.GetReference();
        _error=Error::kErrorNone;
        hold=false;
        emit activated(event = ChildMoved );
        emit activatedWithData(Utilities::FireBase::toQVariant(snapshot.value()),event = ChildMoved );
    }
    void OnChildRemoved(const DataSnapshot& snapshot) {
        if(!eventLoop)
            return;
        QEventLoopLocker unlocker(eventLoop); Q_UNUSED(unlocker) Q_UNUSED(snapshot)
        _snapShot = snapshot;
        _obj = snapshot.GetReference();
        _error=Error::kErrorNone;
        hold=false;
        emit activated(event = ChildRemoved );
        emit activatedWithData(Utilities::FireBase::toQVariant(snapshot.value()),event = ChildRemoved );
    }
    void OnCancelled(const Error& error, const char* error_message) {
        if(!eventLoop)
            return;
        QEventLoopLocker unlocker(eventLoop); Q_UNUSED(unlocker) Q_UNUSED(error_message)
        _snapShot = DataSnapshot();
        _obj=DatabaseReference();
        _error=error;
        hold=false;
        emit activated(event = Cancelled );
        emit activatedWithData(Utilities::FireBase::toQVariant(_snapShot.value()),event = Cancelled );
    }
public:
    explicit QChildListener( DatabaseReference  databaseReference,  QObject * parent= 0): QObject(parent),_databaseReference(databaseReference){
        _databaseReference.AddChildListener(this);
        eventLoop = new QEventLoop(this);
    }
    ~QChildListener(){
        auto tmp = eventLoop;
        eventLoop=nullptr;
        if(tmp->isRunning())
           tmp->exit(0);
        tmp->deleteLater();
        _databaseReference.RemoveChildListener(this);
    }

    DatabaseReference getReference()  noexcept(false) {
        if(hold && eventLoop)
            eventLoop->exec();
#ifdef QT_DEBUG
        if(_error != Error::kErrorNone )
            QT_THROW(_error);
#endif
        return _obj;
    }

    DataSnapshot getSnapShot()  noexcept(false) {
        if(hold && eventLoop)
            eventLoop->exec();
#ifdef QT_DEBUG
        if(_error != Error::kErrorNone )
            throw _error;
#endif
        return _snapShot;
    }

    Error error(){
        return _error;
    }

    QString errorStr(){
        return _errorStr;
    }

};

class QValueListener : public QObject,  ValueListener {
    Q_OBJECT
public:
    enum EVENTS{
        NONE         = 1,
        ValueChanged = 2,
        Cancelled    = 4
    };
    Q_ENUM(EVENTS)
signals:
    void activated(QValueListener::EVENTS); //only connect to queued connecttion
    void activatedWithData(QVariant,QValueListener::EVENTS); //only connect to queued connecttion
private:
    QEventLoop * eventLoop=nullptr;
    EVENTS event;
    DatabaseReference   _obj = DatabaseReference();
    DataSnapshot        _snapShot = DataSnapshot();
    DatabaseReference   _databaseReference;
    Error _error = Error::kErrorNone ;
    QString _errorStr;
    bool hold=true;
    void OnValueChanged(const DataSnapshot& snapshot) {
        if(!eventLoop)
            return;
        QEventLoopLocker unlocker(eventLoop); Q_UNUSED(unlocker)
        _snapShot = snapshot;
        _obj=snapshot.GetReference();
        _error=Error::kErrorNone;
        hold=false;
        emit activated(event = ValueChanged );
        emit activatedWithData(Utilities::FireBase::toQVariant(snapshot.value()),event = ValueChanged );
    }
    void OnCancelled(const Error& error, const char* error_message) {
        if(!eventLoop)
            return;
        QEventLoopLocker unlocker(eventLoop); Q_UNUSED(unlocker) Q_UNUSED(error_message)
        _snapShot = DataSnapshot();
        _obj=DatabaseReference();
        _error=error;
        hold=false;
        emit activated(event = Cancelled );
    }
public:
    explicit QValueListener( DatabaseReference  databaseReference,  QObject * parent= 0): QObject(parent),_databaseReference(databaseReference){
        _databaseReference.AddValueListener(this);
        eventLoop = new QEventLoop(this);
    }
    ~QValueListener(){
        auto tmp = eventLoop;
        eventLoop=nullptr;
        if(tmp->isRunning())
           tmp->exit(0);
        tmp->deleteLater();
        _databaseReference.RemoveValueListener(this);
    }

    DatabaseReference getReference()  noexcept(false) {
        if(hold && eventLoop)
            eventLoop->exec();
#ifdef QT_DEBUG
        if(_error != Error::kErrorNone )
            QT_THROW(_error);
#endif
        return _obj;
    }

    DataSnapshot getSnapShot()  noexcept(false) {
        if(hold && eventLoop)
            eventLoop->exec();
#ifdef QT_DEBUG
        if(_error != Error::kErrorNone )
            throw _error;
#endif
        return _snapShot;
    }

    Error error(){
        return _error;
    }

    QString errorStr(){
        return _errorStr;
    }

};

class ValueListenerSync : public ValueListener ,public QObject{
    QEventLoop *eventLoop=nullptr;
    firebase::Variant _obj;
    Error _error = Error::kErrorNone ;
    DatabaseReference  _databaseReference;
    QString _errorStr;
    bool hold=true;
    firebase::Future<DataSnapshot> dataSnapshot;


    void OnValueChanged(const DataSnapshot& snapshot) {
        QEventLoopLocker unlocker(eventLoop); Q_UNUSED(unlocker)
        _obj=snapshot.value();
        _error=Error::kErrorNone;
        hold=false;
    }

    void OnCancelled(const Error& error, const char* error_message){
        QEventLoopLocker unlocker(eventLoop); Q_UNUSED(unlocker)
        _obj=firebase::Variant();
        _error=error;
        _errorStr =QString(error_message);
        hold=false;
    }

public:
    explicit ValueListenerSync(DatabaseReference & databaseReference,QObject * parent= 0): QObject(parent), _databaseReference(databaseReference){
        _databaseReference.AddValueListener(this);
         eventLoop = new QEventLoop(this);
    }

    ~ValueListenerSync(){
         _databaseReference.RemoveValueListener(this);
    }

    firebase::Variant get() noexcept(false){
        if(hold) eventLoop->exec();
#ifdef QT_DEBUG
        if(_error != Error::kErrorNone ) throw _error;
#endif
        return _obj;
    }

    Error error(){
        return _error;
    }

    QString errorStr(){
        return _errorStr;
    }
};


class QRTDatabase : public QObject
{
    Q_OBJECT
public:
    explicit QRTDatabase(firebase::App * firebaseApp,QByteArray id, QByteArray db_path, QObject *parent = nullptr);

    ~QRTDatabase();

    static QRTDatabase & instance(firebase::App * firebaseApp = nullptr,QByteArray id="", QByteArray db_path="", QObject *parent = nullptr){
        static QRTDatabase db( firebaseApp,id,db_path, parent);
        return db;
    }

    QSharedPointer<Database> getHandle(){ return firebaseDB; }

    DatabaseReference  getHomeRefHandle() { return homeRef; }

    bool isValid(){return m_valid;}

signals :
    void onConfigurationChanged();

private:
   const QByteArray rt_db;
   firebase::InitResult init_result_out;
   QSharedPointer<Database> firebaseDB;
   DatabaseReference homeRef;
   QByteArray path;
   bool m_valid = false;
};

#endif // QRTDATABASE_H
