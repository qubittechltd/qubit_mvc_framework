#include <QVariant>

#pragma once


class SQL_OPERATOR{
    template<typename FIELD>
    friend class TableAttribute;
protected:
    enum TYPE {
        _NONE = 0,
        _EQUAL=2,
        _NOT_EQUAL = ~_EQUAL,
        _LESS=4,//>
        _GREATER=8,//<
        _LESS_OR_EQUAL = _LESS | _EQUAL, //>=
        _GREATER_OR_EQUAL = _GREATER | _EQUAL,//<=
        _LIKE = 16
    };
public:

    template<typename T>
    static auto EQUAL(const T & v){
        return std::make_tuple(v,_EQUAL);
    }

    template<typename T>
    static auto NOT_EQUAL(const T & v){
        return std::make_tuple(v,_NOT_EQUAL);
    }

    template<typename T>
    static auto LESS(const T & v){
        return std::make_tuple(v,_LESS);
    }

    template<typename T>
    static auto GREATER(const T & v){
        return std::make_tuple(v,_GREATER);
    }

    template<typename T>
    static auto LESS_OR_EQUAL(const T & v){
        return std::make_tuple(v,_LESS_OR_EQUAL);
    }

    template<typename T>
    static auto GREATER_OR_EQUAL(const T & v){
        return std::make_tuple(v,_GREATER_OR_EQUAL);
    }

    template<typename T>
    static auto LIKE(const T & v){
        return std::make_tuple(v,_LIKE);
    }

};


