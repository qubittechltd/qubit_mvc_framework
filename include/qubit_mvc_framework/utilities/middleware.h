#pragma once
#include "qubit_mvc_framework/utilities/sequences.h"
#include "qubit_mvc_framework/utilities/session.h"
#include <QtHttpServer/QHttpServer>
#include <QtHttpServer/QHttpServerRouterRule>
#include <QMap>
#include <QString>
#include <QMetaObject>
#include <QScopedPointer>
#include <QMetaObject>
#include <type_traits>
#include <tuple>
#include <utility>

class MIDDLEWARE_P : public QObject
{
public:
    enum COMMON {
        NONE=0x00,
        AUTH=0x01,
        WEB=0x02,
        API=0x04
    };
    Q_FLAG(COMMON)

    explicit MIDDLEWARE_P(QHttpServer *server);

    template<typename Rule = QHttpServerRouterRule, typename ... Args>
    bool route(Args && ... args){
        auto server =  (QHttpServer *) parent();
        auto tuple_all_args = std::make_tuple(std::forward<Args>(args)...);
        auto tuple_reduced_args = Sequences::pop_back<1>(std::forward<Args>(args)...);
        auto last_arg = std::get<sizeof ... (args) - 1>(tuple_all_args);
        auto tuple_post_exeception = std::tuple_cat(
            tuple_reduced_args,
            std::make_tuple([last_arg,this](const QHttpServerRequest &request, QHttpServerResponder &&responder){
                post_middleware(request,std::move(responder),last_arg);
            })
        );
        return std::apply([server](auto... args){
            return server->route(std::forward<decltype(args)>(args)...);
        },tuple_post_exeception);
    }

    template<typename Rule = QHttpServerRouterRule, typename ... Args>
    bool route(MIDDLEWARE_P::COMMON middleware,Args && ... args){

        if(!route(std::forward<Args>(args)...))
            return false;

        auto reduced_args = Sequences::pop_back<1>(std::forward<Args>(args)...);

        auto rule = std::apply([](auto... args){
            auto routerHandler = [](const QRegularExpressionMatch &match,const QHttpServerRequest &request,QHttpServerResponder &&responder) {};

            return new QHttpServerRouterRule(std::forward<decltype(args)>(args)...,std::move(routerHandler));

        }, reduced_args);

        if(rule->hasValidMethods()){
            using ViewHandler = typename Sequences::VariadicTypeLast<Args...>::Type;
            using ViewTraits = QHttpServerRouterViewTraits<ViewHandler>;
            createPathRegexp<ViewTraits>(rule,typename ViewTraits::Arguments::Indexes{});
        }

        registered.emplace_back(std::make_pair(rule,middleware));

        return true;
    }

    template<typename ... Args>
    bool route(MIDDLEWARE_P::COMMON middleware,QHttpServerRequest::Method method,Args && ... args){
        auto args_tuple = std::make_tuple(std::forward<Args>(args)...);
        auto extended_tuple = std::tuple_cat(
            std::make_tuple(
                std::get<0>(args_tuple),
                method
            ),
            Sequences::pop_front<1>(std::forward<Args>(args)...)
        );
        return apply([&](auto&&... modified_args) {
            return route(middleware, std::forward<decltype(modified_args)>(modified_args)...);
        }, extended_tuple);
    }

    QHttpServer & server() const;

    // P∧(P⊕Q)
    template<COMMON inclusive ,COMMON exclusive,typename std::enable_if<(inclusive && ((inclusive & ~exclusive) == inclusive)) ,bool>::type = true>
    bool doMiddleWare(const QHttpServerRequest &request) const {
        auto requested_middleware = inclusive;
        for (auto & p : registered) {
            auto registered_middleware = p.second;
            if(requested_middleware && (registered_middleware & requested_middleware)){
                if(!exclusive){
                    QRegularExpressionMatch match;
                    if(p.first->matches(request,&match)){
                        return true;
                    }
                }
                if((registered_middleware & exclusive) != MIDDLEWARE_P::COMMON::NONE){
                    QRegularExpressionMatch match;
                    if(p.first->matches(request,&match)){
                        return true;
                    }
                }
            }
        }
        return false;
    }

    template<const COMMON include>
    bool doMiddleWare(const QHttpServerRequest &request) const {
        return doMiddleWare<include,COMMON::NONE>(request);
    }

private:
    std::vector<std::pair<std::unique_ptr<QHttpServerRouterRule>,MIDDLEWARE_P::COMMON>> registered;

    template<typename ViewTraits , int ... Idx>
    void createPathRegexp(QHttpServerRouterRule * rule,QtPrivate::IndexesList<Idx...>){
        std::initializer_list<QMetaType> metaTypes{ViewTraits::Arguments::template metaType<Idx>()...};
        rule->createPathRegexp(metaTypes,server().router()->converters());
    }

    using RouterHandler = std::function<QHttpServerResponse (const QHttpServerRequest &,Session &session)>;
    void post_middleware(const QHttpServerRequest &request, QHttpServerResponder &&responder,RouterHandler routerHandler);

};

MIDDLEWARE_P::COMMON  operator|(const MIDDLEWARE_P::COMMON  c ,const MIDDLEWARE_P::COMMON  d );
MIDDLEWARE_P::COMMON  operator&(const MIDDLEWARE_P::COMMON  c ,const MIDDLEWARE_P::COMMON  d );

