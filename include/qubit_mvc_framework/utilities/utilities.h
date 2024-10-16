#ifndef UTILITIES_H
#define UTILITIES_H

#include <QCoreApplication>
#include <QByteArray>
#include <QJsonObject>
#include <QDateTime>
#include <QThreadPool>
#include <QTimer>

#include <QByteArray>
#include <QMultiMap>
#include <QtGlobal>
#include <QtConcurrentRun>
#include <zlib.h>
#include <firebase/database.h>
#include <firebase/app.h>

#define GZIP_WINDOWS_BIT 15 + 16
#define GZIP_CHUNK_SIZE 32 * 1024
#ifndef SSIZE_T_DEFINED
#ifdef _WIN64
typedef __int64 ssize_t;
#endif
#define SSIZE_T_DEFINED
#endif
#ifdef QT_DEBUG
     #define ONLY_DEBUG(x)    qDebug(x)
#else
     #define ONLY_DEBUG(x)    QNoDebug()
#endif

struct Utilities{

    template<class T>
    static void cpy_reset(const std::function<void(std::string)> &dst_func,const T &src,ssize_t dummy = 0){
        QVariant q(src);
        dst_func(q.toString().toStdString());
    }

    static void cpy_reset(std::string &dst,const QString &src,ssize_t dummy = 0){
        dst = src.toStdString();
    }

    static void cpy_reset(char dst [],const QString &src,ssize_t dst_size){
        memset(dst,0,dst_size);
        if(src.size() >= dst_size || src.isEmpty()){
            return;
        }
        memcpy(dst,src.toStdString().c_str(),src.size());
    }

    template<class T>
    static void cpy_reset(std::string &dst,T src,ssize_t dummy = 0){
        QVariant q(src);
        dst = q.toString().toStdString();
    }

    template<class T>
    static void cpy_reset(char dst[],T src,ssize_t dst_size){
        memset(dst,0,dst_size);
        QVariant q(src);
        auto ba = q.toByteArray();
        if(dst_size >= ba.size()){
            memcpy(dst,ba.data(),ba.size());
        }
    }

    template<class T>
    static T char_read(const char src []){
        return QVariant(src).value<T>();
    }

    template<class T>
    static void cpy_reset(T & dst,T src){
        dst=src;
    }

    template <class Key, class T>
    static T mapFind(const Key & key,const T & Default,const QMultiMap<Key, T> * map){
        auto i = map->cbegin();
        while (i != map->cend()) {
            auto k = i.key();
            if(k == key){
                return i.value();
            }
            ++i;
        }
        return Default;
    };

    template <class Key, class T>
    static typename QMultiMap<Key, T>::ConstIterator mapFind(const Key & key,const QMultiMap<Key, T> * map){
        auto i = map->cbegin();
        while (i != map->cend()) {
            auto k = i.key();
            if(k == key){
                return i;
            }
            ++i;
        }
        return i;
    };

    static QJsonObject mapToJsonObject(const QMultiMap<QVariant,QVariant> * map, bool *ok = nullptr ) {
        QJsonObject jsonObject;
        if(ok) *ok = true;
        for (auto it = map->constBegin(); it != map->constEnd(); ++it) {
            const QVariant& v_key = it.key();
            const QVariant& value = it.value();
            if(!v_key.canConvert<QString>()){
                if(ok) *ok = false;
                return jsonObject;
            }

            switch (value.metaType().id()) {
            case QMetaType::Type::Bool:
            case QMetaType::Type::LongLong:
            case QMetaType::Type::Double:
            case QMetaType::Type::Nullptr:
            case QMetaType::Type::QString: {
                jsonObject[v_key.toString()] = value.toJsonValue();
                break;
            }
            case QMetaType::Type::QVariantList:  {
                jsonObject[v_key.toString()] = value.toJsonValue();
                break;
            }
            default:
            case QMetaType::Type::QVariantMap:  {
                auto obj =mapToJsonObject(static_cast<const QMultiMap<QVariant,QVariant> *>(value.data()),ok);
                if(!ok) return jsonObject;
                jsonObject[v_key.toString()] = obj;
                break;
            }
            }
        }
        return jsonObject;
    }

    static bool gzipDecompress(QByteArray input, QByteArray &output){
        // Prepare output
        output.clear();

        // Is there something to do?
        if(input.length() > 0)
        {
            // Prepare inflater status
            z_stream strm;
            strm.zalloc = Z_NULL;
            strm.zfree = Z_NULL;
            strm.opaque = Z_NULL;
            strm.avail_in = 0;
            strm.next_in = Z_NULL;

            // Initialize inflater
            int ret = inflateInit2(&strm, GZIP_WINDOWS_BIT);

            if (ret != Z_OK)
                return(false);

            // Extract pointer to input data
            char *input_data = input.data();
            int input_data_left = input.length();

            // Decompress data until available
            do {
                // Determine current chunk size
                int chunk_size = qMin(GZIP_CHUNK_SIZE, input_data_left);

                // Check for termination
                if(chunk_size <= 0)
                    break;

                // Set inflater references
                strm.next_in = (unsigned char*)input_data;
                strm.avail_in = chunk_size;

                // Update interval variables
                input_data += chunk_size;
                input_data_left -= chunk_size;

                // Inflate chunk and cumulate output
                do {

                    // Declare vars
                    char out[GZIP_CHUNK_SIZE];

                    // Set inflater references
                    strm.next_out = (unsigned char*)out;
                    strm.avail_out = GZIP_CHUNK_SIZE;

                    // Try to inflate chunk
                    ret = inflate(&strm, Z_NO_FLUSH);

                    switch (ret) {
                    case Z_NEED_DICT:
                        ret = Z_DATA_ERROR;
                    case Z_DATA_ERROR:
                    case Z_MEM_ERROR:
                    case Z_STREAM_ERROR:
                        // Clean-up
                        inflateEnd(&strm);

                        // Return
                        return(false);
                    }

                    // Determine decompressed size
                    int have = (GZIP_CHUNK_SIZE - strm.avail_out);

                    // Cumulate result
                    if(have > 0)
                        output.append((char*)out, have);

                } while (strm.avail_out == 0);

            } while (ret != Z_STREAM_END);

            // Clean-up
            inflateEnd(&strm);

            // Return
            return (ret == Z_STREAM_END);
        }
        else
            return(true);
    }

    class FireBase{

    public:

        FireBase(){
            firebase_threadPool.setObjectName("FIREBASE_THREAD_POOL");
            firebase_threadPool.setMaxThreadCount(20);
            firebase_threadPool.setExpiryTimeout(60);
        }

        ~FireBase(){
        }

        template<typename T>
        static T OnCompletionSync(firebase::Future<T>  fds,int duration = 10'000,QString tag=""){
            using namespace firebase;
            QSharedPointer<QEventLoop> event = QSharedPointer<QEventLoop>::create();
            T ds;
            QTimer timer;
            int trials=0;
        AGAIN:  if(duration){
                timer.setTimerType(Qt::VeryCoarseTimer);
                timer.setSingleShot(true);
                timer.setInterval(duration);
                timer.callOnTimeout(event.data(),[=]() {
                        if(!tag.isEmpty())
                            ONLY_DEBUG()<<"ACQUIRING TIMEOUT (TAG : "+tag+" TRIALS : "+QString::number(trials)+")";
                        if(event->isRunning())
                            event->exit(-1);
                    },Qt::DirectConnection);
                timer.start();
            }

            if(!trials || fds.status() != FutureStatus::kFutureStatusPending){
                auto futureRun = QtConcurrent::run(&firebase_threadPool,[&]{
                    fds.OnCompletion([=]( const firebase::Future<T> & fsnap ){  //In most cases, the callback will be running in a different thread,
                        Q_UNUSED(fsnap)
                        if(event->isRunning())
                            QMetaObject::invokeMethod(event.data(), "exit", Qt::QueuedConnection);
                    });
                });
            }
            if(!tag.isEmpty())
                ONLY_DEBUG()<<"ACQUIRING (TAG : "+tag+" )";

            if(event->exec() == -1){
                timer.disconnect();
                if(trials <5){
                    trials++;
                    goto AGAIN;
                }
            }else{
                timer.stop();
            }

            QScopeGuard guard([&]{ fds.Release(); });//WARNING: Future with handle 38506136 still exists though its backing API 0x30807470 is being deleted. Please call Future::Release() before deleting the backing API
            T * snap =(T *) fds.result();
            if(snap != nullptr){
                if(!tag.isEmpty())
                    ONLY_DEBUG()<<"ACQUIRED (TAG : "+tag+" )";
                return *snap;
            }
            return ds;
        }

        static QVariant OnVariantCompletionSync(firebase::database::DatabaseReference  ref,QByteArray key,int duration = 3'000,QVariant defaultValue=QVariant(),QString tag=""){
            firebase::database::DataSnapshot snap;
            if(key.isEmpty()){
                key = QByteArray(ref.GetParent().key());
                snap =OnCompletionSync(ref.GetValue(),duration,tag);
            }else{
                std::string s(key.constData(),key.size());
                if(tag.isEmpty())
                    tag = key;
                snap =OnCompletionSync(ref.Child(s).GetValue(),duration,tag);
            }

            bool defaulted;
            QVariant variant = toQVariant(snap.value(),&defaulted,defaultValue);
            if(defaulted) {
                QByteArray info("Defaulted KEY("+key+")  : VALUE("+variant.toByteArray()+")\0");
                qDebug("    %s", info.data());
            }
            return variant;
        }

        static QVariant OnVariantCompletionSync(firebase::database::DatabaseReference ref,QByteArray key,QString tag=""){
            firebase::database::DataSnapshot snap;
            if(key.isEmpty()){
                key = QByteArray(ref.key());
                snap =OnCompletionSync(ref.GetValue(),3'000,tag);
            }else{
                snap =OnCompletionSync(ref.Child(key.toStdString()).GetValue(),3'000,tag);
            }
            bool defaulted;
            QVariant variant = toQVariant(snap.value(),&defaulted);
            if(defaulted) {
                QByteArray info("Defaulted KEY("+key+")  : VALUE("+variant.toByteArray()+")\0");
                qDebug("    %s", info.data());
            }
            return variant;
        }

        static QVariant toQVariant(firebase::Variant variant, bool *defaulted =nullptr, QVariant defaultValue=QVariant());

        static firebase::Variant fromQJsonValue(QJsonValue value, bool *defaulted = nullptr, firebase::Variant defaultValue = firebase::Variant());

    private:
        static QThreadPool firebase_threadPool;
    };
};



#endif // UTILITIES_H
