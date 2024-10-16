#include "qubit_mvc_framework/utilities/utilities.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QThreadPool>

QThreadPool Utilities::FireBase::firebase_threadPool;

#ifndef DummytoQVariant
#define DummytoQVariant
bool operator<( const QVariant& v1, const QVariant& v2 ){
    return false;
}
#endif

QVariant Utilities::FireBase::toQVariant(firebase::Variant variant, bool *defaulted, QVariant defaultValue){
    if(defaulted)
        *defaulted=false;
    auto type = variant.type();
    switch (type) {
    case firebase::Variant::kTypeInt64:             return   QVariant::fromValue<qint64>(variant.int64_value());
    case firebase::Variant::kTypeDouble:            return   QVariant::fromValue<double>(variant.double_value());
    case firebase::Variant::kTypeBool:              return   QVariant::fromValue<bool>(variant.bool_value());
    case firebase::Variant::kTypeStaticString:
    case firebase::Variant::kTypeMutableString:     return   QVariant::fromValue<QString>(QString::fromStdString(variant.string_value()));;
    case firebase::Variant::kTypeVector: {
        QVariantList list;
        for(auto & val : variant.vector())
            list.append(toQVariant(val));
        return list;
    }
    case firebase::Variant::kTypeMap:{
        QMultiMap<QVariant, QVariant> map;
        for (auto it=variant.map().begin(); it!=variant.map().end(); ++it){
            map.insert(toQVariant(it->first),toQVariant(it->second));
        }
        return QVariant::fromValue<QMultiMap<QVariant, QVariant>>(map);
    }
    case firebase::Variant::kTypeStaticBlob:
    case firebase::Variant::kTypeMutableBlob: {
        QByteArray data(variant.blob_size(),Qt::Uninitialized);
        memcpy(data.data(),variant.blob_data(),variant.blob_size());
        return QVariant::fromValue(data);
    }
    case firebase::Variant::kTypeNull:
    default:
        if(defaulted)
            *defaulted=true;
        return defaultValue;
    }

}


firebase::Variant Utilities::FireBase::fromQJsonValue(QJsonValue value, bool *defaulted, firebase::Variant defaultValue){
    if(defaulted)
        *defaulted=false;
    auto type = value.type();
    switch (type) {
    case QJsonValue::Null   : return firebase::Variant();
    case QJsonValue::Bool   : return firebase::Variant(value.toBool());
    case QJsonValue::Double : {
        bool ok;
        QVariant val       = value;
        int64_t valueInt    = val.toString().toLongLong(&ok);
        if(ok) return firebase::Variant(valueInt);
        else return firebase::Variant( val.toDouble());
    };
    case QJsonValue::String : return firebase::Variant(value.toString().toStdString());
    case QJsonValue::Array  : {
        std::vector<firebase::Variant> list;
        auto arr = value.toArray();
        for(auto  val : arr)
            list.push_back(fromQJsonValue(val));
        return list;
    }
    case QJsonValue::Object :{
        std::map<firebase::Variant, firebase::Variant> map;
        auto obj = value.toObject();
        for (auto it=obj.begin(); it!=obj.end(); ++it){
            map.insert(std::make_pair(it.key().toStdString(),fromQJsonValue(it.value())));
        }
        return map;
    }
    default:
        if(defaulted)
            *defaulted=true;
        return defaultValue;
    }

}
