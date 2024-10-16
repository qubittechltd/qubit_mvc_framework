#ifndef WATCHER_H
#define WATCHER_H
#include "qubit_mvc_framework/utilities/common_p.h"
#include <QDir>
#include <QFileSystemWatcher>

void recursive_add(QFileSystemWatcher &watcher,QString directory){
    // Add the directory and its subdirectories to the watcher
    QDir dir(directory);
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);
    QFileInfoList entries = dir.entryInfoList();
    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            if(!watcher.directories().contains(entry.absoluteDir().absolutePath())){
                watcher.addPath(entry.absoluteFilePath());
                ONLY_DEBUG()<<"watching public directory"<<entry.absoluteDir().absolutePath();
            }
            recursive_add(watcher,entry.absoluteFilePath());
        } else if(!watcher.files().contains(entry.absoluteFilePath())){
            watcher.addPath(entry.absoluteFilePath());
            ONLY_DEBUG()<<"watching public file"<<entry.absoluteFilePath();
        }
    }
};

void watchPublicFolders(QFileSystemWatcher &watcher,QStringList directories,std::function<void(bool isFile,const QString &path)> cb){
    QObject::connect(&watcher, &QFileSystemWatcher::directoryChanged, [&,cb](const QString &path) {
        qDebug() << "Directory changed:" << path;
        recursive_add(watcher,path);
        // cb(false,path);
    });
    QObject::connect(&watcher, &QFileSystemWatcher::fileChanged, [&,cb](const QString &path) {
        qDebug() << "File changed:" << path;
        recursive_add(watcher,path);
        cb(true,path);
    });

    foreach (auto directory, directories) {
        recursive_add(watcher,directory);
    }
}

#endif // WATCHER_H
