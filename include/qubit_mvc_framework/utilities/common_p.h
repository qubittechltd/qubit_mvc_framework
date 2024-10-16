#ifndef COMMON_P_H
#define COMMON_P_H
#include <QString>
#include <exception>

#define FILE_CACHE_TIMER (10*60*1000)
#define FILE_MAX_BYTE_CAN_CACHE (5*1024*1024)
#define FILE_CACHE_MAX_SIZE (1024)
#define REMEMEMBER_ME_DAYS (14)
#define CONTROLLER_TIMEOUT_MSEC (30'000)

#ifdef QT_DEBUG
#define ONLY_DEBUG(x)    qDebug(x)
#else
#define ONLY_DEBUG(x)    QNoDebug()
#endif

#define FLOW_GUARD(var_name,func)                                               \
if(!(var_name)) {                                                           \
    if constexpr(std::is_void_v<std::invoke_result_t<decltype(func)>>) {    \
        /*func(); return;*/                                                 \
        return func();                                                      \
    } else {                                                                \
        return func();                                                      \
    }                                                                       \
}                                                                           \

class  MVC_FILE_NOT_FOUND           : public std::exception{};
class  MVC_FILE_READ_ERROR          : public std::exception{};
struct MVC_FILE_STREAM_REQUIRED     : public std::exception{ QString path;};
class  MVC_FILE_PERMISSION_FAILED   : public std::exception{};
class  MVC_SESSION_EXPIRED          : public std::exception{};
class  MVC_SESSION_TOKEN_TEMPED     : public std::exception{};
class  MVC_AUTHENTICATION_FAILED    : public std::exception{};
class  MVC_USER_UNAUTHORIZED        : public std::exception{};
#endif // COMMON_P_H
