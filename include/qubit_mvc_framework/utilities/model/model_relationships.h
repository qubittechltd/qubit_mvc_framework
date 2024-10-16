#pragma once
#ifndef MODEL_RELATIONSHIPS_H
#define MODEL_RELATIONSHIPS_H

#include "qubit_mvc_framework/utilities/model/table_attribute_impl.h"
#include <QString>

template <typename M2>
class HasOne {
    using M2_KEY_FUNC =  std::function<TableAttributeImpl* (typename M2::Params*)>;
public:

    template <typename M1,typename M1_KEY_FUNC =  std::function<TableAttributeImpl* (typename M1::Params*)>>
    static M2 value(M1 * m1_ptr, const M1_KEY_FUNC & m1_foreign_key_func,const M2_KEY_FUNC & m2_key_func){
        auto * m1_params = m1_ptr->params();
        auto m1_id =  std::invoke(m1_foreign_key_func, m1_params)->value();
        auto o = M2::template safeEntries<M2>([&m1_id, &m2_key_func](auto & vec){
            for(auto & o : vec){
                if(o){
                    auto m2_id = m2_key_func(&o->_params)->value();
                    if(m2_id == m1_id){
                        return M2(o);
                    }
                }
            }
            return M2::Default();
        });
        if(o){
            return o;
        }

        typename M2::Params params;
        params.init();
        // auto m2_key_name = m2_key_func(&params)->_name;
        auto m2_key_name = m2_key_func(&params)->name();
        auto shOptPtr = M2::where(m2_key_name,"=",m1_id)->first();
        return shOptPtr;
    }
};

template <typename M2>
class BelongsTo : public HasOne<M2>{};

template <typename M2>
class HasMany {
    using M2_KEY_FUNC =  std::function<TableAttributeImpl* (typename M2::Params*)>;
public:

    template <typename M1,typename M1_KEY_FUNC =  std::function<TableAttributeImpl* (typename M1::Params*)>>
    static typename M2::Collection value(M1 * m1_ptr, const M1_KEY_FUNC & m1_foreign_key_func,const M2_KEY_FUNC & m2_key_func){
        auto * m1_params = m1_ptr->params();
        auto m1_id =  std::invoke(m1_foreign_key_func, m1_params)->value();
        auto cache_vec = M2::template safeEntries<typename M2::Collection>([&m1_id, &m2_key_func](auto & vec){
            typename  M2::Collection _vec;
            for(auto o : vec){
                if(o){
                    auto m2_id = m2_key_func(&o->_params)->value();
                    if(m2_id == m1_id){
                        _vec.emplace_back(M2(o));
                    }
                }
            }
            return _vec;
        });

        typename M2::Params params;

        params.initializer();

        auto m2_key_name = m2_key_func(&params)->_name;

        typename M2::Collection db_vec = M2::where(m2_key_name,"=",m1_id)->get();

        auto big_vec = db_vec.size() > cache_vec.size() ? &db_vec : &cache_vec;

        auto small_vec = big_vec == &db_vec ? &cache_vec : &db_vec;

        big_vec->reserve(big_vec->size() + small_vec->size());

        auto small_end = small_vec->end();

        auto big_end   = big_vec->end();

        big_end = std::remove_if(big_vec->begin() ,big_end,[&](auto & b_optPtr){
            if(!b_optPtr){
                return true;
            }

            if((small_end - small_vec->begin()) <= 0){
                return false;
            }

            auto it = std::find_if(small_vec->begin() ,small_end,[&b_optPtr](auto & s_optPtr){
                if(!s_optPtr){
                    return true;
                }
                if(b_optPtr == s_optPtr){
                    return true;
                }
                return false;
            });

            if (it != small_vec->end()) {
                if(small_vec == &cache_vec){//prirotize cached values rather than values from db
                    std::swap(*it,b_optPtr);
                }

                std::iter_swap(it, small_end - 1);
                small_end = small_end-1;
            }

            return false;
        });

        if(big_end != big_vec->end()){
            big_vec->erase(big_vec->begin(), big_end);
        }

        if(small_end != small_vec->begin()){
            big_vec->insert(big_vec->end(),small_vec->begin(),small_end);
        }

        return *big_vec;
    }
};

template <typename M2>
class BelongsToMany : public HasMany<M2>{
};

#endif // MODEL_RELATIONSHIPS_H
