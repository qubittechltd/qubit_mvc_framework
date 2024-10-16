#ifndef INTEGRITY_H
#define INTEGRITY_H

#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMetaMethod>
#include <QMetaEnum>

#define NONCE_SIZE 32
const char CHAR_NULL_16 [16] = {0};
const char CHAR_NULL_32 [32] = {0};
const char PACKAGE_NAME [] = R_PROJECT_DOMAIN;

#define CAP_SOME_ADS 2
#define CAP_NO_ADS 4
#define CAP_UNLIMITED_TIME 16
#define CAP_FAST_SERVERS   32

class IntegrityEnums : public QObject {
   Q_OBJECT
public:
    enum AppRecognitionVerdict{
        AR_NONE=(0),
        AR_PLAY_RECOGNIZED=(1),
        AR_UNRECOGNIZED_VERSION=(2),
        AR_UNEVALUATED=(4)
    };
    Q_ENUM(AppRecognitionVerdict)

    enum DeviceRecognitionVerdict{
        DR_NONE=(0),
        DR_MEETS_BASIC_INTEGRITY=(8),
        DR_MEETS_DEVICE_INTEGRITY=(16), //basic
        DR_MEETS_STRONG_INTEGRITY=(32),
        DR_MEETS_VIRTUAL_INTEGRITY=(64)
     };
    Q_ENUM(DeviceRecognitionVerdict)

    enum AppLicensingVerdict{
        AL_NONE=(0),
        AL_LICENSED=(128),
        AL_UNLICENSED=(256),
        AL_UNEVALUATED=(512)
    };
    Q_ENUM(AppLicensingVerdict)

    enum IsTesting{
        T_NONE=(0),
        C_TESTING=(1024),
    };
    Q_ENUM(IsTesting)

    enum CommonIntegrity{
        C_NONE=(0),
        C_SIGNATURE_OK=(2048),
    };
    Q_ENUM(CommonIntegrity)
};

struct Integrity {

    Integrity(bool sign_ok =false){
        memset((void*)this,0,sizeof(Integrity));
        signature_ok=sign_ok;
    }

    Integrity(const QByteArray &payload,bool sign_ok =false);

    bool signature_ok =false;
    struct DeviceIntegrity{
        IntegrityEnums::DeviceRecognitionVerdict deviceRecognitionVerdict[4]={IntegrityEnums::DR_NONE,IntegrityEnums::DR_NONE,IntegrityEnums::DR_NONE,IntegrityEnums::DR_NONE};
     };

    struct AccountDetails{
       IntegrityEnums::AppLicensingVerdict appLicensingVerdict =IntegrityEnums::AL_NONE;
    };

    struct TestingDetails {
       IntegrityEnums::IsTesting isTestingResponse =IntegrityEnums::T_NONE;
    };
    struct AppIntegrity{
        IntegrityEnums::AppRecognitionVerdict appRecognitionVerdict =IntegrityEnums::AppRecognitionVerdict::AR_NONE;
        int  versionCode;
        char packageName[256]; //domain name
        char certificateSha256Digest[5][256];
    };
    struct RequestDetails{
        qint64 timestampMillis;
        char nonce[NONCE_SIZE];//session_id
        char requestPackageName[256];
    };
/////////////////////////////////////////////////////////////////////////////////////////////////////
    union{
        const RequestDetails requestDetails = RequestDetails();
        struct {
            qint64 timestampMillis;
            char nonce[NONCE_SIZE];//session_id
            char requestPackageName[256];
        };
    };

    union{
        const AppIntegrity appIntegrity=AppIntegrity();
        struct {
            IntegrityEnums::AppRecognitionVerdict appRecognitionVerdict;
            int  versionCode ;
            char packageName[256]; //domain name
            char certificateSha256Digest[5][256];
         };
    };

    union{
       const DeviceIntegrity deviceIntegrity=DeviceIntegrity();
       struct {
           IntegrityEnums::DeviceRecognitionVerdict deviceRecognitionVerdict[4];
       };
    };

    union{
       const AccountDetails accountDetails=AccountDetails();
       struct {
          IntegrityEnums::AppLicensingVerdict appLicensingVerdict;
       };
    };

    union{
       const TestingDetails testingDetails=TestingDetails();
       struct  {
         bool isTestingResponse;
       };
    };

    Integrity& operator=(const Integrity& other){
        memcpy(this,&other,sizeof(Integrity));
        return *this;
    }
    QByteArray payload;

    int16_t pack() {
        int16_t pack=0;
        pack |= deviceRecognitionVerdict[0];
        pack |= deviceRecognitionVerdict[1];
        pack |= deviceRecognitionVerdict[2];
        pack |= deviceRecognitionVerdict[3];
        pack |= appLicensingVerdict;
        pack |= appRecognitionVerdict;
        pack |= isTestingResponse;
        return pack;
    }
};

inline Integrity::Integrity(const QByteArray &payload, bool sign_ok) : Integrity(sign_ok){
    this->payload = payload;
    QJsonDocument payloadJs = QJsonDocument::fromJson(payload);
    auto pJsObj = payloadJs.object();
    for(auto pJsObjItr = pJsObj.constBegin(); pJsObjItr != pJsObj.constEnd() ; pJsObjItr++){
        const QJsonValue & objValue = *pJsObjItr;
        const QString & key = pJsObjItr.key();
        if(objValue.isObject()){
            const QJsonObject & obj = objValue.toObject();
            bool okEnum;
            if(pJsObjItr.key() == "requestDetails"){
                QByteArray tmp1=obj.value("requestPackageName").toString().toUtf8();
                QByteArray tmp2=obj.value("timestampMillis").toString().toUtf8();
                QByteArray tmp3=QByteArray::fromBase64(obj.value("nonce").toString().toUtf8(),QByteArray::Base64UrlEncoding);
                memcpy(requestPackageName,tmp1,std::min((int)tmp1.size(),(int)sizeof(requestPackageName)));
                timestampMillis = tmp2.toLongLong();
                memcpy(nonce,tmp3,std::min(NONCE_SIZE,(int)tmp3.size()));
            }else if(pJsObjItr.key() == "appIntegrity"){
                QByteArray tmp1="AR_"+obj.value("appRecognitionVerdict").toString("NONE").toUtf8();
                QByteArray tmp2=obj.value("packageName").toString().toUtf8();
                auto tmp3=obj.value("certificateSha256Digest").toArray();
                auto tmp4=obj.value("versionCode").toString();
                auto&& ENUM = QMetaEnum::fromType<IntegrityEnums::AppRecognitionVerdict>();
                appRecognitionVerdict =(IntegrityEnums::AppRecognitionVerdict)ENUM.keyToValue(tmp1,&okEnum);
                if(!okEnum)
                    appRecognitionVerdict=IntegrityEnums::AppRecognitionVerdict::AR_NONE;
                memcpy(packageName,tmp2,std::min((int)(sizeof(packageName)/sizeof(char)),(int)tmp2.size()));
                int i= 0,ss = sizeof(certificateSha256Digest)/sizeof(certificateSha256Digest[0]);
                for(auto tmp:tmp3){
                    auto tmpStr= tmp.toString().toUtf8();
                    memcpy(certificateSha256Digest[i],tmpStr,std::min((int)(sizeof(certificateSha256Digest[i])/sizeof(char)),(int)tmpStr.size()));
                    if(ss <= ++i)
                        break;
                }
                versionCode=tmp4.toInt();
            }else if(pJsObjItr.key() == "deviceIntegrity"){
                auto tmp1=obj.value("deviceRecognitionVerdict").toArray();
                auto&& ENUM = QMetaEnum::fromType<IntegrityEnums::DeviceRecognitionVerdict>();
                int i= 0,ss = sizeof(deviceRecognitionVerdict)/sizeof(deviceRecognitionVerdict[0]);
                for(auto tmp:tmp1){
                    auto tmpStr= "DR_"+tmp.toString().toUtf8();
                    deviceRecognitionVerdict[i] =(IntegrityEnums::DeviceRecognitionVerdict)ENUM.keyToValue(tmpStr,&okEnum);
                    if(!okEnum)
                        deviceRecognitionVerdict[i]=IntegrityEnums::DeviceRecognitionVerdict::DR_NONE;
                    if(ss <= ++i)
                        break;
                }
            }else if(pJsObjItr.key() == "accountDetails"){
                QByteArray tmp1="AL_"+obj.value("appLicensingVerdict").toString("NONE").toUtf8();
                auto&& ENUM = QMetaEnum::fromType<IntegrityEnums::AppLicensingVerdict>();
                appLicensingVerdict =(IntegrityEnums::AppLicensingVerdict)ENUM.keyToValue(tmp1,&okEnum);
                if(!okEnum)
                    appLicensingVerdict=IntegrityEnums::AppLicensingVerdict::AL_NONE;
            }else if(pJsObjItr.key() == "testingDetails"){
                if(obj.value("isTestingResponse").toBool()){
                    isTestingResponse= IntegrityEnums::IsTesting::C_TESTING;
                }
            }
        }
    }
}

#endif // INTEGRITY_H
