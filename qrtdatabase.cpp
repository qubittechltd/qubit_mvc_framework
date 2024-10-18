#include "qubit_mvc_framework/utilities/qrtdatabase.h"
#include "QCoreApplication"
#include "QEventLoop"
#include "QUrl"
#include "firebase/variant.h"

QRTDatabase::QRTDatabase(firebase::App * firebaseApp, QByteArray id, QByteArray db_path, QObject *parent) :
QObject(parent) ,
    path(id) , rt_db(db_path) {
    firebaseDB = QSharedPointer<Database>(firebase::database::Database::GetInstance(firebaseApp,rt_db,&init_result_out));
    firebaseDB->GoOnline();
    DatabaseReference rootRef = firebaseDB->GetReference();

    qDebug()<<"App Database Ok!";
    homeRef = rootRef.Child("debug");

    if(!homeRef.is_valid()){
        qDebug()<<"Failed to load";
        return;
    }

    homeRef.SetKeepSynchronized(true);

    // DataSnapshot snap = ICommon::OnCompletionSync(homeRef.Child("version").GetValue(),30'000,"Initialize_APP_VERSION");
    // if(snap.is_valid()){
    //     qDebug() << "APP VERSION :  " << snap.value().int64_value();
    // }

    m_valid=true;
    qDebug()<<"Database Finished Setup";
}

QRTDatabase::~QRTDatabase(){
   homeRef.SetKeepSynchronized(false);
}


