#pragma once

#define MODEL_ATTRIBUTE_IMPL(TYPE,VARIABLE,IS_UNIQUE)                             \
    public : Q_PROPERTY(TableAttributeImpl* VARIABLE                                \
                READ get_##VARIABLE##_attribute                                     \
                WRITE set_##VARIABLE##_attribute                                    \
                NOTIFY VARIABLE##_changed                                           \
                STORED IS_UNIQUE                                                  \
            )                                                                       \
                                                                                    \
    public : static const char * get_##VARIABLE##_name()  { return "" #VARIABLE; }   \
                                                                                    \
    public : auto get_##VARIABLE()  { return VARIABLE.value(); }               \
                                                                                    \
    public : TableAttributeImpl * get_##VARIABLE##_attribute()  { return &VARIABLE; }\
                                                                                    \
    protected : void set_##VARIABLE##_attribute(TableAttributeImpl * value) {       \
        auto _value = (TableAttribute<TYPE> *) value;                               \
        if (_value && *_value != VARIABLE()){                                       \
            VARIABLE = _value->value();                                                     \
            emit VARIABLE##_changed(*_value);                                       \
        }                                                                           \
    }                                                                               \
                                                                                    \
    public : void set_##VARIABLE(TYPE value) {                                      \
        if (value != VARIABLE()){                                                   \
            VARIABLE = value;                                                       \
            emit VARIABLE##_changed(value);                                         \
        }                                                                           \
    }                                                                               \
                                                                                    \
    signals: void VARIABLE##_changed(TYPE value){};                                 \
                                                                                    \
    public : TableAttribute<TYPE>  VARIABLE



#define MODEL_ATTRIBUTE_DEFAULT(T,V,D)          MODEL_ATTRIBUTE_IMPL(T,V,false) = (DEFAULT(T((D))));

#define MODEL_ATTRIBUTE(T,V)                    MODEL_ATTRIBUTE_IMPL(T,V,false);

#define UNIQUE_MODEL_ATTRIBUTE_DEFAULT(T,V,D)   MODEL_ATTRIBUTE_IMPL(T,V,true) = (DEFAULT(T((D))));

#define UNIQUE_MODEL_ATTRIBUTE(T,V)             MODEL_ATTRIBUTE_IMPL(T,V,true);

#define PRIMARY_KEY_MODEL_ATTRIBUTE_DEFAULT(T,V,D)   \
    public : static constexpr bool hasPrimary(){ return true;}  \
    public : TableAttributeImpl * primaryKey() { return &V;}  \
    UNIQUE_MODEL_ATTRIBUTE_DEFAULT(T,V,D);

#define PRIMARY_KEY_MODEL_ATTRIBUTE(T,V)   \
    public : static constexpr bool hasPrimary(){ return true;}  \
    public : TableAttributeImpl *  primaryKey() { return &V;}  \
    UNIQUE_MODEL_ATTRIBUTE(T,V,true);

#define MODEL_INIT(MODEL,PARAMS)            \
    friend class ModelBase<MODEL,PARAMS>;   \
    friend struct Eloquent::OrmEndFirst<MODEL>;\
    template<typename MODEL>   friend class HasOne;    \
    template<typename MODEL>   friend class HasMany;   \
    protected:                              \
    MODEL () : ModelBase<MODEL,PARAMS>(){}; \
    MODEL (std::shared_ptr<ModelBaseData<PARAMS>> sh_p) : ModelBase<MODEL,PARAMS>(sh_p) {} \
    MODEL (const PARAMS &p) : ModelBase<MODEL,PARAMS>(std::move(p)) {} \
 \

