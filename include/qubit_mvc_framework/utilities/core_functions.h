#pragma once
#ifndef CORE_FUNCTIONS_H
#define CORE_FUNCTIONS_H

#include "QtCore/qdir.h"
#include "utilities/qfirebaseapp.h"
#include "utilities/qrtdatabase.h"
#include "qubit_mvc_framework/utilities/utilities.h"
#include <QtHttpServer/QHttpServer>
#include <QtHttpServer/QHttpServerResponse>
#include <models/user_model.h>
#include <utilities/cachefiles.h>
#include <middleware/middleware.h>
#include <routes/routes.h>
#include <QSettings>
#include <QTcpServer>
// #include <db_migrations/migration.h>

bool start_server(QHttpServer & server){
    QObject::disconnect(&SettingParams::c(),&SettingParams::notify_server_port,nullptr,nullptr);

    static auto server_port_func = [&](std::string port,QHttpServer &server){
        for (auto * s : server.servers()) {
            for (QTcpServer * s : server.servers()) {
                if(s->isListening()){
                    s->close();
                }
            }
        }

        bool ok= false;
        auto server_port = QVariant(port.c_str()).toInt(&ok);
        if(!ok || server_port==0){
            qCritical("Failed to listen on port : %s", port.c_str());
            return false;
        }

        if(!server.listen(QHostAddress::Any,server_port)){
            qCritical("Failed to listen on port: %i",server_port);
            return false;
        }
        qDebug("listening at port: %i", server_port );

        return true;
    };

    QObject::connect(&SettingParams::c(),&SettingParams::notify_server_port,&server,[&](std::string port){
        server_port_func(port,server);
    });

    if(!server_port_func(SettingParams::c().get_server_port(),server)){
        return false;
    }

    return true;
}


void resetSettings(const int  duration){
    QSettings settings(QDir::currentPath() + "/config.ini", QSettings::IniFormat);

    // initial value
    QString firebase_link = settings.value("Firebase/Link").toString();
    int server_port = settings.value("HttpServer/port", 4444).toInt();
    QString db_name = settings.value("Database/name", "simple_mvc").toString();
    QString db_hostname = settings.value("Database/hostname", "localhost").toString();
    QString db_username = settings.value("Database/username", "root").toString();
    QString db_password = settings.value("Database/password", "password").toString();
    int db_port = settings.value("Database/port", 3306).toInt();

    if(firebase_link.isEmpty()){
        qDebug() << "config.ini does not contain firebase link";
    }

    auto & db = QRTDatabase::instance( QFirebase::instance().handlePtr(), QByteArray(), firebase_link.toUtf8());
    if(!db.isValid()){
        qDebug()<< "STATUS=Failed pulling configurations from database\n";
    }

    auto home = db.getHomeRefHandle();

    Utilities::cpy_reset([](std::string v){
        SettingParams::current().opt_set_server_port(v);
    }, Utilities::FireBase::OnVariantCompletionSync(home,"server_port",duration,server_port).toString());

    Utilities::cpy_reset([](std::string v){
        SettingParams::current().opt_set_db_username(v);
    }, Utilities::FireBase::OnVariantCompletionSync(home,"db_username",duration,db_username).toString());

    Utilities::cpy_reset([](std::string v){
        SettingParams::current().opt_set_db_password(v);
    }, Utilities::FireBase::OnVariantCompletionSync(home,"db_password",duration,db_password).toString());

    Utilities::cpy_reset([](std::string v){
        SettingParams::current().opt_set_db_hostname(v);
    }, Utilities::FireBase::OnVariantCompletionSync(home,"db_hostname",duration,db_hostname).toString());

    Utilities::cpy_reset([](std::string v){
        SettingParams::current().opt_set_db_port(v);
    }, Utilities::FireBase::OnVariantCompletionSync(home,"db_port",duration,db_port).toString());

    Utilities::cpy_reset([](std::string v){
        SettingParams::current().opt_set_db_name(v);
    }, Utilities::FireBase::OnVariantCompletionSync(home,"db_name",duration,db_name).toString());

    SettingParams::current().stamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
}

#endif // CORE_FUNCTIONS_H
