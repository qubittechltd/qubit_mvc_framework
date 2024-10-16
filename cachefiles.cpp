#include "qubit_mvc_framework/utilities/cachefiles.h"
#include "qubit_mvc_framework/utilities/common_p.h"
#include "qubit_mvc_framework/utilities/watcher.h"
#include <QDebug>
#include <QThread>

std::vector<CacheFiles *> files;
QFileSystemWatcher watcher;
bool operator==(const CacheFiles *c ,const QString &other_path){
    return c->params().path == other_path;
}
const CacheFiles & CacheFiles::load_file(QObject *context,const QString &filePath){

    auto func = [](bool isFile,const QString &path){
        for(auto & f : ::files){
            if(isFile && (f->p.realPath==path)){
                f->deleteLater();
                break;
            }else if(f->p.realPath.startsWith(path)){
                f->deleteLater();
            }
        }
    };
    static bool pragma_once(true);
    if(pragma_once){
        pragma_once=false;
#ifdef QT_DEBUG
        watchPublicFolders(::watcher,{DEBUG_PUBLIC_PATH,"/var/www/html"},func);
#else
        watchPublicFolders(watcher,{"/var/www/html"},func);
#endif
    }

    if(context->thread() != QThread::currentThread()){
        qWarning()<<"Context doesn't belong to this thread";
    }
    auto itr = std::find(::files.begin(),::files.end(),filePath);
    if(itr == ::files.end()){
        bool found_in_debug=false, found =false;
#ifdef QT_DEBUG
        if(!(found_in_debug=::watcher.files().contains(DEBUG_PUBLIC_PATH + filePath)) &&
           !(found=::watcher.files().contains("/var/www/html" + filePath))){
#else
        if(!(found=::watcher.files().contains("/var/www/html" + filePath))){
#endif
            throw MVC_FILE_NOT_FOUND();
        }
        auto realPath = found_in_debug ? DEBUG_PUBLIC_PATH + filePath : "/var/www/html" + filePath;
        QFile file(realPath);

        if(!file.open(QIODevice::ReadOnly)){
            if(file.error() == QFileDevice::PermissionsError){
                throw MVC_FILE_PERMISSION_FAILED();
            }
            throw MVC_FILE_READ_ERROR();
        }

        PARAMS p;
        p.path = filePath;
        p.realPath = realPath;
        if(file.size() > FILE_MAX_BYTE_CAN_CACHE || ::files.size() > FILE_CACHE_MAX_SIZE ){
            MVC_FILE_STREAM_REQUIRED e;
            e.path = realPath;
            throw e;
        }

        p.data=file.readAll();
        file.close();
        auto c =new CacheFiles(context,p);
        return *c;
    }
    return  **itr;
}

CacheFiles::CacheFiles(QObject *parent,const PARAMS &pp)
    : p(std::move(pp)) , QObject{parent}
{
    timer.callOnTimeout(this,[this]{
        QObject::deleteLater();
    });
    timer.setSingleShot(true);
    timer.start(FILE_CACHE_TIMER);
    ::files.push_back(this);
}

CacheFiles::~CacheFiles(){
    auto itr = std::find(::files.begin(),::files.end(),this);
    if(itr != ::files.end()){
        ::files.erase(itr);
    }
}
