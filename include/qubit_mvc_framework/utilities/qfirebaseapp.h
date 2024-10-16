#ifndef QFIREBASEAPP_H
#define QFIREBASEAPP_H
#include "qubit_mvc_framework/utilities/google-services.json.h"

class QFirebase : public QObject {
    firebase::App * firebaseApp=nullptr;
    WGOOGLESERVICE wgoogleService;
    Q_OBJECT
public :
    explicit QFirebase(QObject * parent = 0){
        Q_UNUSED(parent)
        firebaseApp = firebase::App::Create(*wgoogleService.options().get());
    }

    static QFirebase & instance(){
        static QFirebase qfb;
        return qfb;
    }

    const firebase::App & handle() const {
        return *firebaseApp;
    }

    firebase::App * handlePtr() const{
        return firebaseApp;
    }

    ~QFirebase(){
        // if(firebaseApp != nullptr)
        //     delete firebaseApp;
    }


};

#endif // QFIREBASEAPP_H
