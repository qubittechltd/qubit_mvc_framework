#ifndef GOOGLESERVICES_JSON
#define GOOGLESERVICES_JSON
#include "QByteArray"
#include "QObject"
#include "firebase/app.h"

#define GOOGLESERVICES_JSON_FILE "{\"project_info\":{\"project_number\":\"831319296905\",\"firebase_url\":\"https://simple-mvc-firebase-default-rtdb.firebaseio.com\",\"project_id\":\"simple-mvc-firebase\",\"storage_bucket\":\"qubit-server-templete.appspot.com\"},\"client\":[{\"client_info\":{\"mobilesdk_app_id\":\"1:3284445413:android:4eb767cf52fdf6a75a8c0c\",\"android_client_info\":{\"package_name\":\"com.qubit.server.templete\"}},\"oauth_client\":[{\"client_id\":\"3284445413-58b5v2frsp7l2e4a71k02pfm7vu4kkfo.apps.googleusercontent.com\",\"client_type\":3}],\"api_key\":[{\"current_key\":\"AIzaSyC1FpR_zZBzkQvd7LF5q6DszhE-ogzl3P4\"}],\"services\":{\"appinvite_service\":{\"other_platform_oauth_client\":[{\"client_id\":\"3284445413-58b5v2frsp7l2e4a71k02pfm7vu4kkfo.apps.googleusercontent.com\",\"client_type\":3}]}}}],\"configuration_version\":\"1\"}"



class WGOOGLESERVICE : public QObject {
    std::shared_ptr<firebase::AppOptions>  _options= std::make_shared<firebase::AppOptions>();
    Q_OBJECT
public :
    explicit WGOOGLESERVICE(QObject * parent = 0): QObject(parent){
        firebase::AppOptions::LoadFromJsonConfig(json().data(),_options.get());
        Q_ASSERT_X(_options,"firebase::AppOptions can't be null","WGOOGLESERVICE");
    }
    // virtual ~WGOOGLESERVICE();

    const QByteArray json () const { return QByteArray(GOOGLESERVICES_JSON_FILE); }

    const  std::shared_ptr<firebase::AppOptions> options (){
        return _options;
    }

};


#endif // GOOGLESERVICES_JSON

