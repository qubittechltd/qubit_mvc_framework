#include "qubit_mvc_framework/utilities/middleware.h"
#include "qubit_mvc_framework/utilities/cachefiles.h"
#include "qubit_mvc_framework/utilities/common_p.h"
#include "qubit_mvc_framework/utilities/redirect.h"
#include <QtHttpServer/QHttpServer>
#include <QtHttpServer/QHttpServerRouterRule>
#include <qubit_mvc_framework/utilities/session.h>

QByteArray NOT_FOUND();

void urlHandler(QObject *server,const QString &path,QHttpServerResponder &responder){
    try{
        try {
            auto & c = CacheFiles::load_file(server,path);
            QHttpServerResponse response(c.params().data);
            if(path.endsWith(".css")){
                response.addHeader("Content-Type","text/css");
            }else if(path.endsWith(".js")){
                response.addHeader("Content-Type","text/javascript");
            }
            responder.sendResponse(response);
        }catch (MVC_FILE_STREAM_REQUIRED & e) {
            responder.sendResponse(QHttpServerResponse::fromFile(e.path));
        }catch (MVC_FILE_NOT_FOUND & e) {
            try{
                auto & c = CacheFiles::load_file(server,"/errors/error_404.html");
                QHttpServerResponse response(c.params().data);
                responder.sendResponse(response);
            }catch(const std::exception & e){
                QHttpServerResponse response(
                    NOT_FOUND(),
                    QHttpServerResponse::StatusCode::NotFound
                );
                // response.addHeader("Content-Type","text/javascript");
                responder.sendResponse(response);
            }
        }
        catch (MVC_FILE_PERMISSION_FAILED & e) {
            urlHandler(server,"/errors/error_403.html",responder);
        }
        catch (MVC_FILE_READ_ERROR & e) {
            urlHandler(server,"/errors/error_500.html",responder);
        }
        catch (MVC_SESSION_EXPIRED & e) {
            responder.sendResponse(QHttpServerResponse(redirect("/login"),QHttpServerResponder::StatusCode::MovedPermanently));
        }
        catch (MVC_USER_UNAUTHORIZED & e) {
            responder.sendResponse(QHttpServerResponse(redirect("/login"),QHttpServerResponder::StatusCode::MovedPermanently));
        }
    }catch(std::exception &e){
        qDebug("system_error %s",e.what());
    }
}

void MIDDLEWARE_P::post_middleware(const QHttpServerRequest &request, QHttpServerResponder &&responder,RouterHandler routerHandler){
    auto server =  (QHttpServer *) parent();
    try{
        bool session_found;
        auto session = Session::user_session_t(request);

        auto response = routerHandler(request,*std::get<0>(session));
        ///after controller // mid _ middleware
        if(!std::get<1>(session)){
            response.addHeaders({
                {"Set-Cookie",std::get<0>(session)->getCookie()},
            });
        }
        responder.sendResponse(response);
        //after response  // post middleware
        //histroy taking

    }catch (ControllerDeletedException & e) {
        urlHandler(server,"/errors/error_500.html",responder);
    }catch(ControllerTimeOutException & e){
        urlHandler(server,"/errors/error_408.html",responder);
    }catch(std::exception & e){
        qDebug("return stacktrace web page std::exception %s",e.what());
    }
}

MIDDLEWARE_P::MIDDLEWARE_P(QHttpServer *server) : QObject(server){

    server->setMissingHandler([server](const QHttpServerRequest &request,QHttpServerResponder &&responder){
        auto path = request.url().path();
        urlHandler(server,path,responder);
    });

    //WEB
    server->beforeController([server, this](bool &handled,const QHttpServerRequest &request,QHttpServerResponder &responder){
        if(!doMiddleWare<MIDDLEWARE_P::WEB>(request)){
            return true;
        }
        auto session = request.session();
        session->history(request.url().path().toStdString());
        Q_UNUSED(session)
        return true;
    });

}

QHttpServer & MIDDLEWARE_P::server() const{
    return *static_cast<QHttpServer*>(parent());
}

MIDDLEWARE_P::COMMON operator|(const MIDDLEWARE_P::COMMON c, const MIDDLEWARE_P::COMMON d){
    auto a = (qintptr)c;
    auto b = (qintptr)c;
    return MIDDLEWARE_P::COMMON(a | b);
}

MIDDLEWARE_P::COMMON operator&(const MIDDLEWARE_P::COMMON c, const MIDDLEWARE_P::COMMON d){
    auto a = (qintptr)c;
    auto b = (qintptr)d;
    return MIDDLEWARE_P::COMMON(a & b);
}

QByteArray NOT_FOUND(){
    QByteArray data=""
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
"<html>\r\n"
"   <head>\r\n"
"       <title>404 Not Found</title>\r\n"
"   </head>\r\n"
"   <body>\r\n"
"       <h1>404 Not Found</h1>\r\n"
"   </body>\r\n"
"</html>\r\n";
    return data;
}

