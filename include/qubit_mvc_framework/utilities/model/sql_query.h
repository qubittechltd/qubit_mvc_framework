#ifndef SQL_QUERY_H
#define SQL_QUERY_H
#include <QDateTime>
#include <QString>
#include <QByteArray>
#include <QJsonobject>
#include <string>
#include <QVariant>
#include <QReadWriteLock>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QSqlRecord>
#include <QRegularExpression>
#include <QtConcurrent/QtConcurrent>

namespace Eloquent {

enum SQL_OPERATOR_TYPE {
    NONE = 0,
    EQUAL=2,
    NOT_EQUAL = ~EQUAL,
    LESS=4,//>
    GREATER=8,//<
    LESS_OR_EQUAL = LESS | EQUAL, //>=
    GREATER_OR_EQUAL = GREATER | EQUAL,//<=
    LIKE = 16
};

class SQL_OPERATOR{
    template<typename FIELD>
    friend class TableAttribute;
    typedef Eloquent::SQL_OPERATOR_TYPE TYPE;
public:
    static QString toString(TYPE op) {
        switch (op) {
        case TYPE::EQUAL:
            return "=";
        case TYPE::NOT_EQUAL:
            return "<>";
        case TYPE::LESS:
            return "<";
        case TYPE::GREATER:
            return ">";
        case TYPE::LESS_OR_EQUAL:
            return "<=";
        case TYPE::GREATER_OR_EQUAL:
            return ">=";
        case TYPE::LIKE:
            return "LIKE";
        default:
            return "";
        }
    }
};

template<typename T>
struct is_string_type : std::false_type {};
template<>
struct is_string_type<std::string> : std::true_type {};
template<>
struct is_string_type<QString> : std::true_type {};
template<>
struct is_string_type<QByteArray> : std::true_type {};
template<>
struct is_string_type<const char*> : std::true_type {};
template<>
struct is_string_type<char> : std::true_type {};

template <typename T, typename... Args>
struct are_same_type : std::false_type {};

template <typename T, typename First, typename... Rest>
struct are_same_type<T, First, Rest...>
    : std::conditional_t<std::is_same<T, First>::value, are_same_type<T, Rest...>, std::false_type> {};

template <typename T>
struct are_same_type<T> : std::true_type {};

template<typename T>
struct SHARED_QUERY{
    SHARED_QUERY():ptr(std::make_shared<T>()){}
    SHARED_QUERY(T * t){
        ptr = std::shared_ptr<T>(new T(*t));
    }
    SHARED_QUERY(std::shared_ptr<T> _ptr) : ptr(_ptr) {}
    auto operator->(){
        return ptr.operator->();
    }
private:
    std::shared_ptr<T> ptr;
};

template<typename MODEL>
struct OrmEndGet;
template<typename MODEL>
struct OrmEndFirst;
template<typename MODEL>
struct OrmEndFirstOrFail;

struct Query {

    static void printErrorQuery(const QSqlQuery &query,const QMap<QString,QVariant> & key_value = {}){
        auto qry = query.lastQuery();
        for(auto k_v : key_value.asKeyValueRange()){
            QString value = k_v.second.isNull() ? "null" :
                                "'" + k_v.second.toString() + "'";

            qry = qry.replace(k_v.first,value);
        }
        qWarning() << "ExecutedQuery  :" << qry;
        qWarning() << "LastError      :" << query.lastError();
    }
protected:

    inline Query& append_if_not_empty(const QString& part) {
        if(!queryString.isEmpty())
            return append(part);
        return *this;
    }
    inline Query& append_if_not_ended(const QString& check,const QString& part) {
        if(!queryString.endsWith(check,Qt::CaseInsensitive))
            return append(part);
        return *this;
    }
    inline Query& append_if_not_ended(const QString& part) {
        if(!queryString.endsWith(part,Qt::CaseInsensitive))
            return append(part);
        return *this;
    }
    inline Query& append_if_empty(const QString& part) {
        if(queryString.isEmpty())
            return append(part);
        return *this;
    }
    inline Query& prepend(const QString& part) {
        queryString.prepend(part);
        return *this;
    }
    inline Query& prepend_if(bool condition ,const QString& part) {
        if(condition)
            queryString.prepend(part);
        return *this;
    }
    inline Query& append(const QString& part) {
        queryString += part;
        return *this;
    }
    inline Query& append_if(bool condition ,const QString& part) {
        if(condition)
            queryString += part;
        return *this;
    }
    inline Query& space() {
        return append(" ");
    }
    inline Query& space_if_not_ended() {
        return append_if_not_ended(" ");
    }
    inline Query& chop_if_ended(const QString& check,const QString& part) {
        if(queryString.endsWith(check,Qt::CaseInsensitive))
            chop(part);
        return *this;
    }
    inline Query& chop(const QString& part) {
        if(queryString.endsWith(part,Qt::CaseInsensitive))
            queryString.chop(part.size());
        return *this;
    }
    inline Query& trimmed() {
        queryString=queryString.trimmed();
        return *this;
    }
    virtual inline Query& reset(QString s=""){
        queryString = s;
        return *this;
    }
    const QString & query() const {
        return queryString;
    }

private:
    QString queryString;
    template<class T,class T1>
    friend struct OrmEnd;
    template<typename MODEL> friend struct OrmEndFirst;
    template<typename MODEL> friend struct OrmEndFirstOrFail;
    template<typename MODEL> friend struct OrmEndDelete;
    template<typename MODEL> friend struct LimitCPO;
    template<typename MODEL> friend struct OffsetCPO;
    template<typename MODEL> friend struct HavingCPO;
    template<typename MODEL> friend struct GroupByCPO;
    template<typename MODEL> friend struct OrderByCPOImpl;
    template<typename MODEL> friend struct OrderByCPO;
    template<typename MODEL> friend struct OldestCPO;
    template<typename MODEL> friend struct LatestCPO;
    template<typename MODEL> friend struct WhereInCPO;
    template<typename MODEL> friend struct WhereNotInCPO;
    template<typename MODEL> friend struct WhereBetweenCPO;
    template<typename MODEL> friend struct WhereNotBetweenCPO;
    template<typename MODEL> friend struct WhereNullCPO;
    template<typename MODEL> friend struct WhereNotNullCPO;
    template<typename MODEL> friend struct WhereCPO;
    template<typename MODEL> friend struct OrWhereCPO;
    template<typename MODEL> friend struct FromCPO;
    template<typename MODEL> friend struct SelectAll;
    template<typename MODEL> friend struct SelectCPO;
    template<typename MODEL> friend struct PugCPO;
    template<typename MODEL> friend struct JoinCPO;
    template<typename MODEL> friend struct UpdateCPO;
    template<typename MODEL> friend struct OrmEndTruncateCPO;
    template<typename MODEL> friend struct OrmEndDoesNotExists;
    friend struct ExistsCPO;
    friend struct ExecuteQuery;
    friend struct DoesNotExistsCPO;
};


template<typename T>
struct OrmEndCount : public Query{
    auto operator()() const {
        auto s = query().trimmed();
        static QRegularExpression re("(^SELECT\\s).*?(\\sFROM.*$)",QRegularExpression::CaseInsensitiveOption|QRegularExpression::DotMatchesEverythingOption);
        if(!re.match(s).hasMatch()){
            qWarning()<<"Incorrect count query"<<s;
        }
        QString qryStr = s.replace(re, "\\1COUNT(*)\\2");

        QSqlDatabase db;
        if constexpr(std::is_same<T,Query>::value){
            db = T::default_db;
        }else if(!T::init_db(db)){
            return 0;
        }
        QSqlQuery sqlQuery(qryStr,db);

        if (!sqlQuery.exec()) {
            Query::printErrorQuery(sqlQuery);
            return 0;
        }

        if(sqlQuery.next()) {
            return sqlQuery.value(0).toInt();
        }
        return 0;
    }
};

template<typename T>
struct OrmEndExists : public Query {
    auto operator()() const {
        auto qryStr = query().trimmed();
        qryStr += "LIMIT 1";

        QSqlDatabase db;

        if constexpr(std::is_same<T,Query>::value){
            db = T::default_db;
        }else if(!T::init_db(db)){
            return false;
        }
        QSqlQuery sqlQuery(qryStr,db);

        if (!sqlQuery.exec()) {
            Query::printErrorQuery(sqlQuery);
            return false;
        }

        if (!sqlQuery.next()) {
            return false;
        }
        return true;
    }
};

template<typename T>
struct OrmEndDoesNotExists : public Query {
    auto operator()() const {
        auto v = exist();
        return !v;
    }
    virtual inline Query& reset(QString s=""){
        Query::reset(s);
        exist.reset(s);
        return *this;
    }
private:
    OrmEndExists<T> exist{};
};

struct OrmEndPrint : public Query{
    auto operator()() const {
        qDebug("%s\n", query().toStdString().c_str());
        return this;
    }
};

template<typename T>
struct OrmEndGet : public Query{
    auto operator()() const {
        std::vector<QSqlRecord> records;
        auto vec = typename T::Collection();
        if (query().isEmpty()) {
            qWarning() << "Attempted to execute an empty query.";
            if constexpr(std::is_same<T,Query>::value) return  records;
            else return vec;
        }

        QSqlDatabase db;
        if constexpr(std::is_same<T,Query>::value){
            db = T::default_db;
        }else if(!T::init_db(db)){
            return vec;
        }

        QSqlQuery sqlQuery(query(),db);

        if (!sqlQuery.exec()) {
            Query::printErrorQuery(sqlQuery);
            if constexpr(std::is_same<T,Query>::value) return  records;
            else return vec;
        }

        while (sqlQuery.next()) {
            if constexpr(std::is_same<T,Query>::value){
                records.emplace_back(sqlQuery.record());
            }else{
                auto && params = T::Params::template create<typename T::Params>(sqlQuery.record());

                vec.emplace_back(std::move(T::create(params)));
            }
        }

        if constexpr(std::is_same<T,Query>::value) return records;
        else return vec;

    }

    operator bool() const {
        if (query().isEmpty()) {
            qWarning() << "Attempted to execute an empty query.";
            return false;
        }
        auto qryStr = query();

        QSqlDatabase db;
        if constexpr(std::is_same<T,Query>::value){
            db = T::default_db;
        }else if(!T::init_db(db)){
            return false;
        }
        QSqlQuery sqlQuery(qryStr,db);

        if (!sqlQuery.exec()) {
            Query::printErrorQuery(sqlQuery);
            return false;
        }
        if (sqlQuery.next()) {
            return true;
        }
        return false;
    }
protected:

    auto operator()(std::function<T(const QSqlRecord &)>  func) -> std::vector<T> const {
        std::vector<T> records;
        if (query().isEmpty()) {
            qWarning() << "Attempted to execute an empty query.";
            return records;
        }
        auto qryStr = query();

        QSqlDatabase db;
        if constexpr(std::is_same<T,Query>::value){
            db = T::default_db;
        }else if(!T::init_db(db)){
            return records;
        }
        QSqlQuery sqlQuery(qryStr,db);

        if (!sqlQuery.exec()) {
            Query::printErrorQuery(sqlQuery);
            return records;
        }
        while (sqlQuery.next()) {
            records.emplace_back(func(sqlQuery.record()));
        }
        return records;
    }
private:
    friend class UserModel;

};

template<typename T>
struct OrmEndFirst : public Query {
    auto operator()() const{
        if (query().isEmpty()) {
            qWarning() << "Attempted to execute an empty query.";
            if constexpr(std::is_same<T,Query>::value) return  QSqlRecord();
            else return T::Default();
        }

        auto qryStr = query();

        QSqlDatabase db;
        if constexpr(std::is_same<T,Query>::value){
            db = T::default_db;
        }else if(T::init_db(db)){
            auto map = extract_values();

            if(auto model = T::retriveFromCache(map)){
                return model;
            }
        }else{
            return T::Default();
        }

        QSqlQuery sqlQuery(qryStr,db);

        if (!sqlQuery.exec()) {
            Query::printErrorQuery(sqlQuery);
            if constexpr(std::is_same<T,Query>::value) return  QSqlRecord();
            else return T::Default();
        }

        if (!sqlQuery.next()) {
            if constexpr(std::is_same<T,Query>::value) return  QSqlRecord();
            else return T::Default();
        }

        if constexpr(std::is_same<T,Query>::value){
            return sqlQuery.record();
        }else{
            using MODEL =  T;
            auto params =  MODEL::Params::template create<typename MODEL::Params >(sqlQuery.record());
            return T::create(params);
        }

    }
private:
    QMap<QString, QVariant> extract_values() const{
        QMap<QString, QVariant> conditions;
        auto const qryStr = query();

        QRegularExpression whereRegex("WHERE\\s+(.+)$", QRegularExpression::CaseInsensitiveOption);
        QRegularExpressionMatch match = whereRegex.match(qryStr);

        if (match.hasMatch()) {
            QString whereClause = match.captured(1);

            QStringList conditionList = whereClause.split("AND", Qt::SkipEmptyParts);

            for (const QString& condition : conditionList) {
                // Extract key and value from each condition
                static const QRegularExpression conditionRegex("\\s*`?([\\w_]+)`?\\s*=\\s*'([^']*)'");
                QRegularExpressionMatch condMatch = conditionRegex.match(condition.trimmed());

                if (condMatch.hasMatch()) {
                    QString key = condMatch.captured(1);
                    QString value = condMatch.captured(2);
                    conditions.insert(key, value);
                }
            }
        }
        return conditions;
    }
};

template<typename T>
struct OrmEndFirstOrFail : public Query {
    struct exception : public std::exception {
        const char* what() const noexcept override {
            return "Query returned no results.";
        }
    };
    auto operator()() const {
        auto v = first();
        if(v.isEmpty()){
            qWarning() << "NO Records";
            throw OrmEndFirstOrFail::exception();
        }
        return v;
    }
    virtual inline Query& reset(QString s=""){
        Query::reset(s);
        first.reset(s);
        return *this;
    }

private:
    OrmEndFirst<T> first{};
};

template<typename T>
struct OrmEndDelete : public Query {

    auto operator()() const {
        if constexpr(std::is_same<T,Query>::value){
            if (query().isEmpty()) {
                qWarning() << "Attempted to execute an empty query.";
                if constexpr(std::is_same<T,Query>::value) return  QSqlRecord();
                else return T::Default();
            }

            QSqlDatabase db;
            if constexpr(std::is_same<T,Query>::value){
                db = T::default_db;
            }else if(!T::init_db(db)){
                return T::Default();
            }

            auto qryStr = query();
            if(qryStr.startsWith("SELECT")){
                qryStr = qryStr.replace(0, 6, "DELETE");
            }

            QSqlQuery sqlQuery(qryStr,db);

            if (!sqlQuery.exec()) {
                Query::printErrorQuery(sqlQuery);
                if constexpr(std::is_same<T,Query>::value) return  QSqlRecord();
                else return T::Default();
            }

            if (!sqlQuery.next()) {
                if constexpr(std::is_same<T,Query>::value) return  QSqlRecord();
                else return T::Default();
            }
            return true;
        }else if(auto v = first()){
            v.deleteLater();
            return true;
        }

        return false;
    }
    virtual inline Query& reset(QString s=""){
        Query::reset(s);
        first.reset(s);
        return *this;
    }

private:
    OrmEndFirst<T> first{};
};

template<typename T>
struct OrmEndUpdate : public Query {

    template <typename U = T>//for orm
    typename std::enable_if<!std::is_same<U, Query>::value, int>::type
    operator()(const typename T::Params &params) {
        auto info = T::get_db_Info();

        QString && updateStr = QString("UPDATE `%1`.`%2` SET ").arg(info.db_name,info.table_name);
        for(auto & inf : params.getTableAttributesVec()){
            if(inf->isValid() && !inf->isBaseAtrribute() && !inf->isDefaulted()){
                auto name  = inf->name();
                updateStr += name + " =:" + name;
                updateStr += (inf == params.getTableAttributesVec().back()) ? " " : " , ";
            }
        }

        QString && from = QString("FROM `%1`.`%2` ").arg(info.db_name,info.table_name);
        auto qryStr = query();
        auto conditions = qryStr.mid(qryStr.indexOf(from)+from.size());

        auto new_qryStr = updateStr+conditions;

        QSqlDatabase db;
        if(!T::init_db(db)){
            return 0;
        }

        QSqlQuery sqlQuery(db);

        sqlQuery.prepare(new_qryStr);

        for(auto & inf : params.getTableAttributesVec()){
            if(inf->isValid()){
                sqlQuery.bindValue(":" + inf->name(), inf->value());
            }
        }

        if (!sqlQuery.exec()) {
            qWarning()<<"Query execution failed :"<< sqlQuery.lastError().text().toStdString();
            return 0;
        }

        return sqlQuery.numRowsAffected();

    }

    template <typename U = T>//for sqlbuilder
    typename std::enable_if<std::is_same<U, Query>::value, int>::type
    operator()(const QMap<QString, QVariant> & map) {

        auto qryStr = query();
        if (query().isEmpty()) {
            return 0;
        }

        if (map.isEmpty()) {
            return 0;
        }

        auto info = T::get_db_Info();

        typename T::Params params;
        auto local_vec_attributes = params.getTableAttributesVec();

        QString && updateString = QString("UPDATE `%1`.`%2` SET ").arg(info.db_name,info.table_name);

        bool has_at_least_one_colum = false;
        for (auto itr = map.cbegin(); itr != map.cend(); ++itr) {
            auto vec_itr = std::find_if(local_vec_attributes.cbegin(),local_vec_attributes.cend(),[&itr](auto & attr){
                return itr.key() == attr->name();
            });
            if(vec_itr != local_vec_attributes.end()){
                updateString += QString("%1 = :%1").arg(itr.key());
                updateString += (std::next(itr) != map.cend()) ? ", " : " ";
                has_at_least_one_colum = true;
            }
        }
        if(!has_at_least_one_colum){
            return 0;
        }

        QSqlDatabase db;
        if(!T::init_db(db)){
            return 0;
        }

        QString && from = QString("FROM `%1`.`%2` ").arg(info.db_name,info.table_name);
        auto conditions = qryStr.mid(qryStr.indexOf(from)+from.size());
        auto new_qryStr = updateString+conditions;

        QSqlQuery sqlQuery(db);

        sqlQuery.prepare(new_qryStr);

        for (auto itr = map.cbegin(); itr != map.cend(); ++itr) {
            auto vec_itr = std::find_if(local_vec_attributes.cbegin(),local_vec_attributes.cend(),[&itr](auto & attr){
                return itr.key() == attr->name();
            });
            if(vec_itr != local_vec_attributes.end()){
                sqlQuery.bindValue(":" + itr.key(), itr.value());
            }
        }


        if (!sqlQuery.exec()) {
            qWarning()<<"Query execution failed :"<< sqlQuery.lastError().text().toStdString();
            return 0;
        }

        return sqlQuery.numRowsAffected();
    }
};

template<typename T>
struct OrmEndTruncateCPO : public Query {
    auto operator()(bool disableForeignKeyChecks) const {
        auto info = T::get_db_Info();
        QString qryStr;

        if (disableForeignKeyChecks) {
            qryStr = QString("SET FOREIGN_KEY_CHECKS = 0; "
                             "TRUNCATE TABLE `%1`.`%2`; "
                             "SET FOREIGN_KEY_CHECKS = 1;")
                         .arg(info.db_name, info.table_name);
        } else {
            qryStr = QString("TRUNCATE TABLE `%1`.`%2`")
            .arg(info.db_name, info.table_name);
        }

        QSqlDatabase db;
        if (!T::init_db(db)) {
            qWarning() << "Database initialization failed";
            return false;
        }

        QSqlQuery sqlQuery(db);
        if (!sqlQuery.exec(qryStr)) {
            qWarning() << "Query execution failed:" << sqlQuery.lastError().text().toStdString();
            return false;
        }

        return true;
    }

    bool operator()() const {
        return (*this)(false);
    }

};

template<typename T,typename TYPE_B4_REACHED_END>
struct OrmEnd{
    OrmEndFirst<T> first{};
    OrmEndFirstOrFail<T> firstOrFail{};
    OrmEndGet<T> get{};
    OrmEndCount<T> count{};
    OrmEndExists<T> exists{};
    OrmEndDoesNotExists<T> doesNotExists{};
    OrmEndDelete<T> deleteLater{};
    OrmEndUpdate<T> update{};
    OrmEndTruncateCPO<T> truncate{};
    OrmEndPrint print{};


    void prepare_end(QString query){

        first.reset(query);
        firstOrFail.reset(query);
        get.reset(query);
        print.reset(query);
        count.reset(query);
        exists.reset(query);
        doesNotExists.reset(query);
        deleteLater.reset(query);
        update.reset(query);
        truncate.reset(query);
    }

    auto clone() {
        return SHARED_QUERY<TYPE_B4_REACHED_END>((TYPE_B4_REACHED_END*)this);
    }
};

template<typename MODEL>
struct OffsetCPO : public  OrmEnd<MODEL,OffsetCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;
    template<typename T,typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
    auto operator()(T offset)  {
        space_if_not_ended().append("OFFSET").space().append(QString::number(offset));
        this->prepare_end(query());
        return this;
    }
};

template<typename MODEL>
struct LimitCPO : public  OrmEnd<MODEL,LimitCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;
    template<typename T,typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
    auto operator()(T limit)  {
        space_if_not_ended().append("LIMIT").space().append(QString::number(limit));
        offset.reset(query());
        this->prepare_end(query());
        return this;
    }

    OffsetCPO<MODEL>offset{};
};

template<typename MODEL>
struct HavingCPO : public OrmEnd<MODEL,HavingCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;
    template<typename T>
    auto operator()(T column) -> typename std::enable_if<(is_string_type<T>::value), decltype(this)>::type const {
        /*** implement here ***/
        limit.reset(query());
        this->prepare_end(query());
        return this;
    }
    LimitCPO<MODEL>limit{};
};

template<typename MODEL>
struct GroupByCPO : public OrmEnd<MODEL,GroupByCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;

    template<typename ...Args>
    auto operator()(Args ...columns) -> typename std::enable_if<(is_string_type<std::decay_t<Args>>::value && ...), decltype(this)>::type {
        // Start the "GROUP BY" clause if not already started
        trimmed().space_if_not_ended().append("GROUP BY");
        // Append each column name to the query, separated by commas
        (space().append("`").append(columns).append("`,"), ...);
        chop(","); // Remove the trailing comma
        space();

        // Prepare the end of the query for further chaining
        having.reset(query());
        limit.reset(query());
        this->prepare_end(query());
        return this;
    }
    HavingCPO<MODEL>having{};
    LimitCPO<MODEL>limit{};
};

enum ORDER {
    ASC,
    DESC
};

template<typename MODEL>
struct OrderByCPOImpl : public OrmEnd<MODEL,OrderByCPOImpl<MODEL>>, public Query {
    typedef MODEL ModelBase;
    OrderByCPOImpl(ORDER o = ORDER::ASC):order(o){}

    auto operator()(){
        trimmed()
            .space_if_not_ended()
            .chop("ASC ")
            .chop("DESC ")
            .append_if_not_ended(order == ORDER::ASC ? "ASC " : "DESC ");

        limit.reset(query());
        this->prepare_end(query());
        return this;
    }
    LimitCPO<MODEL> limit{};
private:
    ORDER order;
};

template<typename MODEL>
struct OrderByCPO : public OrmEnd<MODEL,OrderByCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;
    // Handle single column with specified order
    template<typename T>
    auto operator()(T column, ORDER order = ORDER::ASC) -> typename std::enable_if<is_string_type<T>::value, decltype(this)>::type {

        if (!query().contains("ORDER BY", Qt::CaseInsensitive)) {
            space_if_not_ended().append("ORDER BY").space();
        } else {
            append(", "); // Append a comma to add another column
        }

        append("`").append(column).append("`").space()
            .append(order == ORDER::ASC ? "ASC" : "DESC");

        asc.reset(query());
        desc.reset(query());
        limit.reset(query());
        this->prepare_end(query());
        return this;
    }

    // Handle multiple columns using variadic templates
    template<typename... Args>
    auto operator()(ORDER order, Args... args){
        (void)std::initializer_list<int>{ (operator()(order, args), 0)... };
        return this; // Recursively handle the rest
    }

    OrderByCPOImpl<MODEL> asc;
    OrderByCPOImpl<MODEL> desc = OrderByCPOImpl<MODEL>(ORDER::DESC);
    LimitCPO<MODEL> limit{};
private:
    template <typename T>
    void operator()(ORDER order, const T& arg) {
        // Base case: handle the single argument
        operator()(arg,order);
    }
};

template<typename MODEL>
struct LatestCPO : public OrmEnd<MODEL,LatestCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;

    template<typename T = std::string>
    auto operator()(T column="created_at", ORDER order = ORDER::ASC) -> typename std::enable_if<is_string_type<T>::value, decltype(this)>::type {
        space_if_not_ended().append("ORDER BY").space()
            .append("`").append(column).append("`").space()
            .append(order == ORDER::ASC ? "ASC" : "DESC");

        asc.reset(query());
        desc.reset(query());
        limit.reset(query());
        this->prepare_end(query());
        return this;
    }

    // Handle multiple columns using variadic templates
    template<typename T = std::string, typename... Args>
    auto operator()(T column = "created_at", ORDER order = ORDER::ASC, Args... args) -> typename std::enable_if<is_string_type<T>::value, decltype(this)>::type {
        operator()(column, order); // Handle the first column and order
        return operator()(args...); // Recursively handle the rest
    }

    OrderByCPOImpl<MODEL> asc;
    OrderByCPOImpl<MODEL> desc = OrderByCPOImpl<MODEL>(ORDER::DESC);
    LimitCPO<MODEL> limit{};
};

template<typename MODEL>
struct OldestCPO : public OrmEnd<MODEL,OldestCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;

    template<typename T = std::string>
    auto operator()(T column="created_at", ORDER order = ORDER::DESC) -> typename std::enable_if<is_string_type<T>::value, decltype(this)>::type {
        space_if_not_ended().append("ORDER BY").space()
            .append("`").append(column).append("`").space()
            .append(order == ORDER::ASC ? "ASC" : "DESC");

        asc.reset(query());
        desc.reset(query());
        limit.reset(query());
        this->prepare_end(query());
        return this;
    }

    // Handle multiple columns using variadic templates
    template<typename T = std::string, typename... Args>
    auto operator()(T column = "created_at", ORDER order = ORDER::DESC, Args... args) -> typename std::enable_if<is_string_type<T>::value, decltype(this)>::type {
        operator()(column, order); // Handle the first column and order
        return operator()(args...); // Recursively handle the rest
    }

    OrderByCPOImpl<MODEL> asc;
    OrderByCPOImpl<MODEL> desc = OrderByCPOImpl<MODEL>(ORDER::DESC);
    LimitCPO<MODEL> limit{};
};

template<typename MODEL> struct WhereCPO;
template<typename MODEL> struct WhereInCPO;
template<typename MODEL> struct WhereNotInCPO;
template<typename MODEL> struct WhereBetweenCPO;
template<typename MODEL> struct WhereNotBetweenCPO;
template<typename MODEL> struct WhereNullCPO;
template<typename MODEL> struct WhereNotNullCPO;
template<typename MODEL> struct JoinCPO;

template<typename MODEL>
struct OrWhereCPO : public  OrmEnd<MODEL,OrWhereCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;

    template<typename T,typename std::enable_if<is_string_type<T>::value>::type* = nullptr>
    auto operator()(T column ,SQL_OPERATOR_TYPE op,QVariant value)  {
        return operator()(column,SQL_OPERATOR::toString(op),value);
    }

    template<typename T1,typename T2,typename std::enable_if<is_string_type<T1>::value && is_string_type<T2>::value>::type* = nullptr>
    auto operator()(T1 column ,T2 op,QVariant value)  {
        trimmed().space().append_if_not_ended("WHERE ").chop_if_ended("OR WHERE ","WHERE ");
        append("`").append(column).append("`").append(op).append("'").append(value.toString()).append("'");
        orderBy.reset(query());
        groupBy.reset(query());
        limit.reset(query());
        latest.reset(query());
        oldest.reset(query());
        this->prepare_end(query());
        return this;
    }

    template<typename T1,typename std::enable_if<is_string_type<T1>::value>::type* = nullptr>
    auto where(T1 column ,SQL_OPERATOR_TYPE op,QVariant value){
        return where(column,SQL_OPERATOR::toString(op),value);
    }

    template<typename T1,typename T2,typename std::enable_if<is_string_type<T1>::value && is_string_type<T2>::value>::type* = nullptr>
    auto where(T1 column ,T2 op,QVariant value){
        SHARED_QUERY<WhereCPO<MODEL>> ptr = std::make_shared<WhereCPO<MODEL>>();
        ptr->reset(query()).append(" AND WHERE");
        (*ptr.operator->())(column,op,value);
        return ptr;
    }

    template<typename T1,typename std::enable_if<is_string_type<T1>::value>::type* = nullptr>
    auto orWhere(T1 column ,SQL_OPERATOR_TYPE op,QVariant value){
        return orWhere(column,SQL_OPERATOR::toString(op),value);
    }

    template<typename T1,typename T2,typename std::enable_if<is_string_type<T1>::value && is_string_type<T2>::value>::type* = nullptr>
    auto orWhere(T1 column ,T2 op,QVariant value){
        auto ptr = std::make_shared<OrWhereCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" OR WHERE");
        (*ptr.operator->())(column,op,value);
        return ptr;
    }

    template<typename T1>
    auto whereIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereInCPO<MODEL>> ptr = std::make_shared<WhereInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1>
    auto whereNotIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereNotInCPO<MODEL>> ptr = std::make_shared<WhereNotInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1, typename T2, typename T3>
    auto whereBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereBetweenCPO<MODEL>> ptr = std::make_shared<WhereBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1, typename T2, typename T3>
    auto whereNotBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereNotBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereNotBetweenCPO<MODEL>> ptr = std::make_shared<WhereNotBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1>
    auto whereNull(T1 column){
        SHARED_QUERY<WhereNullCPO<MODEL>> ptr = std::make_shared<WhereNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENULL ");
        (*ptr.operator->())(column);
        return ptr;
    }

    template<typename T1>
    auto whereNotNull(T1 column){
        SHARED_QUERY<WhereNotNullCPO<MODEL>> ptr = std::make_shared<WhereNotNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTNULL ");
        (*ptr.operator->())(column);
        return ptr;
    }


    LimitCPO<MODEL> limit{};
    OrderByCPO<MODEL> orderBy{};
    GroupByCPO<MODEL> groupBy{};
    LatestCPO<MODEL> latest{};
    OldestCPO<MODEL> oldest;
};

template<typename MODEL>
struct WhereCPO :public  OrmEnd<MODEL,WhereCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;

    template<typename T,typename std::enable_if<is_string_type<T>::value>::type* = nullptr>
    auto operator()(T column ,SQL_OPERATOR_TYPE op,QVariant value)  {
        auto as_this = (WhereCPO *)this;
        return as_this->operator()(column,SQL_OPERATOR::toString(op),value);
    }

    template<typename T1,typename T2,typename std::enable_if<is_string_type<T1>::value && is_string_type<T2>::value>::type* = nullptr>
    auto operator()(T1 column ,T2 op,QVariant value)  {
        trimmed().space().append_if_not_ended("WHERE ").chop_if_ended("AND WHERE ","WHERE ");
        append("`").append(column).append("`").append(QString(op)).append("'").append(value.toString()).append("'");

        limit.reset(query());
        orderBy.reset(query());
        groupBy.reset(query());
        latest.reset(query());
        oldest.reset(query());
        orWhere.reset(query()).append(" OR WHERE");
        this->prepare_end(query());
        return this;
    }

    template<typename T1,typename std::enable_if<is_string_type<T1>::value>::type* = nullptr>
    auto where(T1 column ,SQL_OPERATOR_TYPE op,QVariant value){
        return where(column,SQL_OPERATOR::toString(op),value);
    }

    template<typename T1,typename T2,typename std::enable_if<is_string_type<T1>::value && is_string_type<T2>::value>::type* = nullptr>
    auto where(T1 column ,T2 op,QVariant value){
        SHARED_QUERY<WhereCPO<MODEL>> ptr = std::make_shared<WhereCPO<MODEL>>();
        ptr->reset(query()).append_if_not_ended("WHERE "," AND WHERE");
        (*ptr.operator->())(column,op,value);
        return ptr;
    }

    template<typename T1>
    auto whereIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereInCPO<MODEL>> ptr = std::make_shared<WhereInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1>
    auto whereNotIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereNotInCPO<MODEL>> ptr = std::make_shared<WhereNotInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1, typename T2, typename T3>
    auto whereBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereBetweenCPO<MODEL>> ptr = std::make_shared<WhereBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1, typename T2, typename T3>
    auto whereNotBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereNotBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereNotBetweenCPO<MODEL>> ptr = std::make_shared<WhereNotBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1>
    auto whereNull(T1 column){
        SHARED_QUERY<WhereNullCPO<MODEL>> ptr = std::make_shared<WhereNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENULL ");
        (*ptr.operator->())(column);
        return ptr;
    }

    template<typename T1>
    auto whereNotNull(T1 column){
        SHARED_QUERY<WhereNotNullCPO<MODEL>> ptr = std::make_shared<WhereNotNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTNULL ");
        (*ptr.operator->())(column);
        return ptr;
    }

    LimitCPO<MODEL> limit{};
    OrderByCPO<MODEL> orderBy{};
    GroupByCPO<MODEL> groupBy{};
    OrWhereCPO<MODEL> orWhere{};
    LatestCPO<MODEL> latest{};
    OldestCPO<MODEL> oldest;
};

template<typename MODEL>
struct WhereInCPO :public  OrmEnd<MODEL,WhereInCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;
    template<typename T1>
    auto operator()(T1 column ,const std::initializer_list<QVariant> & values)  {
        trimmed().space().chop_if_ended(" AND WHEREIN ","WHEREIN ")
            .append_if_not_ended(" AND ","WHERE").space()
            .append("`").append(column).append("`").space().append("IN").space().append("(");

        for (auto & value : values) {
            space().append("'").append(value.toString()).append("',");
        }
        chop(",").space().append(")");

        limit.reset(query());
        orderBy.reset(query());
        groupBy.reset(query());
        latest.reset(query());
        oldest.reset(query());
        this->prepare_end(query());
        return this;
    }

    template<typename T1>
    auto whereIn(T1 column ,const std::initializer_list<QVariant> & values){
        append(" AND WHEREIN ");
        operator()(column,values);
        return this;
    }

    template<typename T1>
    auto whereNotIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereNotInCPO<MODEL>> ptr = std::make_shared<WhereNotInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1, typename T2, typename T3>
    auto whereBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereBetweenCPO<MODEL>> ptr = std::make_shared<WhereBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }



    template<typename T1, typename T2, typename T3>
    auto whereNotBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereNotBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereNotBetweenCPO<MODEL>> ptr = std::make_shared<WhereNotBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1>
    auto whereNull(T1 column){
        SHARED_QUERY<WhereNullCPO<MODEL>> ptr = std::make_shared<WhereNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENULL ");
        (*ptr.operator->())(column);
        return ptr;

    }

    template<typename T1>
    auto whereNotNull(T1 column){
        SHARED_QUERY<WhereNotNullCPO<MODEL>> ptr = std::make_shared<WhereNotNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTNULL ");
        (*ptr.operator->())(column);
        return ptr;
    }

    WhereCPO<MODEL> where{};
    OrWhereCPO<MODEL> OrWhere{};
    LimitCPO<MODEL> limit{};
    OrderByCPO<MODEL> orderBy{};
    GroupByCPO<MODEL> groupBy{};
    LatestCPO<MODEL> latest{};
    OldestCPO<MODEL> oldest;
};

template<typename MODEL>
struct WhereNotInCPO :public  OrmEnd<MODEL,WhereNotInCPO<MODEL>>, public Query {

    typedef MODEL ModelBase;

    template<typename T1>
    auto operator()(T1 column ,const std::initializer_list<QVariant> & values)  {
        trimmed().space().chop_if_ended(" AND WHERENOTIN ","WHERENOTIN ")
            .append_if_not_ended(" AND ","WHERE").space()
            .append("`").append(column).append("`").space().append("NOT IN").space().append("(");

        for (auto & value : values) {
            space().append("'").append(value.toString()).append("',");
        }
        chop(",").space().append(")");

        limit.reset(query());
        orderBy.reset(query());
        this->prepare_end(query());
        return this;
    }

    template<typename T1>
    auto whereNotIn(T1 column ,const std::initializer_list<QVariant> & values){
        append(" AND WHERENOTIN ");
        operator()(column,values);
        return this;
    }

    template<typename T1>
    auto whereIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereInCPO<MODEL>> ptr = std::make_shared<WhereInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1, typename T2, typename T3>
    auto whereBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereBetweenCPO<MODEL>> ptr = std::make_shared<WhereBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1, typename T2, typename T3>
    auto whereNotBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereNotBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereNotBetweenCPO<MODEL>> ptr = std::make_shared<WhereNotBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    WhereCPO<MODEL> where{};
    OrWhereCPO<MODEL> OrWhere{};
    LimitCPO<MODEL> limit{};
    OrderByCPO<MODEL> orderBy{};
};

template<typename MODEL>
struct WhereBetweenCPO : public OrmEnd<MODEL,WhereBetweenCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;

    template<typename T1, typename T2, typename T3>
    auto operator()(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, decltype(this)>::type {
        trimmed().space().chop_if_ended(" AND WHEREBETWEEN ","WHEREBETWEEN ")
            .append_if_not_ended(" AND ","WHERE").space()
            .append("`").append(column).append("` BETWEEN '").append(QVariant(low).toString()).append("' AND '").append(QVariant(high).toString()).append("'");

        limit.reset(query());
        orderBy.reset(query());
        groupBy.reset(query());
        latest.reset(query());
        oldest.reset(query());
        this->prepare_end(query());
        return this;
    }


    template<typename T1, typename T2, typename T3>
    auto whereBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereBetweenCPO>>::type {
        append(" AND WHEREBETWEEN");
        operator()(column, low, high);
        return this;
    }


    template<typename T1>
    auto whereIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereInCPO<MODEL>> ptr = std::make_shared<WhereInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1>
    auto whereNotIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereNotInCPO<MODEL>> ptr = std::make_shared<WhereNotInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1, typename T2, typename T3>
    auto whereNotBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereNotBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereNotBetweenCPO<MODEL>> ptr = std::make_shared<WhereNotBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1>
    auto whereNull(T1 column){
        SHARED_QUERY<WhereNullCPO<MODEL>> ptr = std::make_shared<WhereNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENULL ");
        (*ptr.operator->())(column);
        return ptr;

    }

    template<typename T1>
    auto whereNotNull(T1 column){
        SHARED_QUERY<WhereNotNullCPO<MODEL>> ptr = std::make_shared<WhereNotNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTNULL ");
        (*ptr.operator->())(column);
        return ptr;
    }

    WhereCPO<MODEL>   where{};
    OrWhereCPO<MODEL> OrWhere{};
    LimitCPO<MODEL>   limit{};
    OrderByCPO<MODEL> orderBy{};
    GroupByCPO<MODEL> groupBy{};
    LatestCPO<MODEL> latest{};
    OldestCPO<MODEL> oldest;
};

template<typename MODEL>
struct WhereNotBetweenCPO : public OrmEnd<MODEL,WhereNotBetweenCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;
    template<typename T1, typename T2, typename T3>
    auto operator()(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, decltype(this)>::type {
        trimmed().space().chop_if_ended(" AND WHERENOTBETWEEN ","WHERENOTBETWEEN ")
            .append_if_not_ended(" AND ","WHERE").space()
            .append("`").append(column).append("` NOT BETWEEN '").append(QVariant(low).toString()).append("' AND '").append(QVariant(high).toString()).append("'");

        limit.reset(query());
        orderBy.reset(query());
        groupBy.reset(query());
        latest.reset(query());
        oldest.reset(query());
        this->prepare_end(query());
        return this;
    }


    template<typename T1, typename T2, typename T3>
    auto whereNotBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereNotBetweenCPO<MODEL>>>::type {
        append(" AND WHERENOTBETWEEN ");
        operator()(column, low, high);
        return this;
    }


    template<typename T1>
    auto whereIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereInCPO<MODEL>> ptr = std::make_shared<WhereInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1>
    auto whereNotIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereNotInCPO<MODEL>> ptr = std::make_shared<WhereNotInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1, typename T2, typename T3>
    auto whereBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereBetweenCPO<MODEL>> ptr = std::make_shared<WhereBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1>
    auto whereNull(T1 column){
        SHARED_QUERY<WhereNullCPO<MODEL>> ptr = std::make_shared<WhereNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENULL ");
        (*ptr.operator->())(column);
        return ptr;

    }

    template<typename T1>
    auto whereNotNull(T1 column){
        SHARED_QUERY<WhereNotNullCPO<MODEL>> ptr = std::make_shared<WhereNotNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTNULL ");
        (*ptr.operator->())(column);
        return ptr;
    }

    WhereCPO<MODEL>   where{};
    OrWhereCPO<MODEL> OrWhere{};
    LimitCPO<MODEL>   limit{};
    OrderByCPO<MODEL> orderBy{};
    GroupByCPO<MODEL> groupBy{};
    LatestCPO<MODEL> latest{};
    OldestCPO<MODEL> oldest;
};

template<typename MODEL>
struct WhereNullCPO :public  OrmEnd<MODEL,WhereNullCPO<MODEL>>, public Query {

    typedef MODEL ModelBase;
    template<typename T1>
    auto operator()(T1 column)  {
        trimmed().space().chop_if_ended(" AND WHERENULL ","WHERENULL ")
            .append_if_not_ended(" AND ","WHERE").space()
            .append("`").append(column).append("`").space().append("IS NULL").space();

        limit.reset(query());
        orderBy.reset(query());
        groupBy.reset(query());
        latest.reset(query());
        oldest.reset(query());
        where.reset(query()).append(" AND WHERE");
        orWhere.reset(query()).append(" OR WHERE");
        this->prepare_end(query());
        return this;
    }

    template<typename T1>
    auto whereNull(T1 column){
        append(" AND WHERENULL ");
        operator()(column);
        return this;
    }

    template<typename T1, typename T2, typename T3>
    auto whereBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereBetweenCPO<MODEL>> ptr = std::make_shared<WhereBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1>
    auto whereNotIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereNotInCPO<MODEL>> ptr = std::make_shared<WhereNotInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1, typename T2, typename T3>
    auto whereNotBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereNotBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereNotBetweenCPO<MODEL>> ptr = std::make_shared<WhereNotBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1>
    auto whereNotNull(T1 column){
        SHARED_QUERY<WhereNotNullCPO<MODEL>> ptr = std::make_shared<WhereNotNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTNULL ");
        (*ptr.operator->())(column);
        return ptr;
    }

    WhereCPO<MODEL> where{};
    OrWhereCPO<MODEL> orWhere{};
    LimitCPO<MODEL> limit{};
    OrderByCPO<MODEL> orderBy{};
    GroupByCPO<MODEL> groupBy{};
    LatestCPO<MODEL> latest{};
    OldestCPO<MODEL> oldest;
};

template<typename MODEL>
struct WhereNotNullCPO :public  OrmEnd<MODEL,WhereNotNullCPO<MODEL>>, public Query {
    typedef MODEL ModelBase;
    template<typename T1>
    auto operator()(T1 column)  {
        trimmed().space().chop_if_ended(" AND WHERENOTNULL ","WHERENOTNULL ")
            .append_if_not_ended(" AND ","WHERE").space()
            .append("`").append(column).append("`").space().append("IS NOT NULL").space();

        limit.reset(query());
        orderBy.reset(query());
        groupBy.reset(query());
        latest.reset(query());
        oldest.reset(query());
        where.reset(query());
        this->prepare_end(query());
        return this;
    }

    template<typename T1>
    auto whereNotNull(T1 column){
        append(" AND WHERENOTNULL ");
        operator()(column);
        return this;
    }

    template<typename T1, typename T2, typename T3>
    auto whereBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereBetweenCPO<MODEL>> ptr = std::make_shared<WhereBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHEREBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1>
    auto whereNotIn(T1 column ,const std::initializer_list<QVariant> & values){
        SHARED_QUERY<WhereNotInCPO<MODEL>> ptr = std::make_shared<WhereNotInCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTIN ");
        (*ptr.operator->())(column,values);
        return ptr;
    }

    template<typename T1, typename T2, typename T3>
    auto whereNotBetween(T1 column, T2 low, T3 high) -> typename std::enable_if<is_string_type<T1>::value, SHARED_QUERY<WhereNotBetweenCPO<MODEL>>>::type {
        SHARED_QUERY<WhereNotBetweenCPO<MODEL>> ptr = std::make_shared<WhereNotBetweenCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENOTBETWEEN ");
        (*ptr.operator->())(column, low, high);
        return ptr;
    }

    template<typename T1>
    auto whereNull(T1 column){
        SHARED_QUERY<WhereNullCPO<MODEL>> ptr = std::make_shared<WhereNullCPO<MODEL>>();
        ((Query*)ptr.operator->())->reset(query()).append(" AND WHERENULL ");
        (*ptr.operator->())(column);
        return ptr;

    }

    WhereCPO<MODEL> where{};
    OrWhereCPO<MODEL> OrWhere{};
    LimitCPO<MODEL> limit{};
    OrderByCPO<MODEL> orderBy{};
    GroupByCPO<MODEL> groupBy{};
    LatestCPO<MODEL> latest{};
    OldestCPO<MODEL> oldest;
};

template<typename MODEL>
struct JoinCPO :public  OrmEnd<MODEL,JoinCPO<MODEL>>, public Query{
    typedef MODEL ModelBase;

    template<typename T1,typename T2>
    auto operator()(T1 table ,T2 primary_key, T2 foreighn_key)  {
        trimmed().space().append("JOIN").space().append("`").append(table).append("`");
        space().append("ON ").append(primary_key).space().append("=").space().append(foreighn_key);

        limit.reset(query());
        orderBy.reset(query());
        groupBy.reset(query());
        latest.reset(query());
        oldest.reset(query());
        where.reset(query());
        this->prepare_end(query());
        return this;
    }

    LimitCPO<MODEL> limit{};
    OrderByCPO<MODEL> orderBy{};
    GroupByCPO<MODEL> groupBy{};
    OrWhereCPO<MODEL> orWhere{};
    LatestCPO<MODEL> latest{};
    OldestCPO<MODEL> oldest{};
    WhereCPO<MODEL> where{};
};

template<typename MODEL>
struct FromCPO : public  OrmEnd<MODEL,FromCPO<MODEL>>, public Query {
public:
    typedef MODEL ModelBase;

    template<typename T,typename std::enable_if<is_string_type<T>::value>::type* = nullptr>
    auto operator()(T table)  {
        space_if_not_ended().append("FROM").space()
            .append("`").append(table).append("`").space();

        bool hasSoftDelete =false;
        if constexpr(!std::is_same<MODEL,Query>::value){
            if(MODEL::UseSoftDelete()){
                append("WHERE deleted_at IS NULL").space();
                hasSoftDelete = true;
            }
        }

        limit.reset(query());
        orderBy.reset(query());
        groupBy.reset(query());
        latest.reset(query());
        oldest.reset(query());
        where.reset(query()).append_if(hasSoftDelete,           "AND ");
        whereIn.reset(query()).append_if( hasSoftDelete,        "AND ");
        whereNotIn.reset(query()).append_if( hasSoftDelete,     "AND ");
        whereBetween.reset(query()).append_if( hasSoftDelete,   "AND ");
        whereNotBetween.reset(query()).append_if( hasSoftDelete,"AND ");
        whereNull.reset(query()).append_if( hasSoftDelete,      "AND ");
        whereNotNull.reset(query()).append_if( hasSoftDelete,   "AND ");
        join.reset(query());
        this->prepare_end(query());
        return this;
    }

    template<typename T1,typename T2,typename std::enable_if<is_string_type<T1>::value && is_string_type<T2>::value>::type* = nullptr>
    auto operator()(T1 database,T2 table)  {
        if(QVariant::fromValue(database).toString().isEmpty()){
            return operator()(table);
        }
        space_if_not_ended().append("FROM").space().append("`").append(database)
            .append("`").append(".").append("`").append(table).append("`").space();

        bool hasSoftDelete =false;
        if constexpr(!std::is_same<MODEL,Query>::value){
            if(MODEL::UseSoftDelete()){
                append("WHERE deleted_at IS NULL").space();
                hasSoftDelete = true;
            }
        }

        limit.reset(query());
        orderBy.reset(query());
        groupBy.reset(query());
        latest.reset(query());
        oldest.reset(query());
        where.reset(query()).append_if(hasSoftDelete,           "AND ");
        whereIn.reset(query()).append_if( hasSoftDelete,        "AND ");
        whereNotIn.reset(query()).append_if( hasSoftDelete,     "AND ");
        whereBetween.reset(query()).append_if( hasSoftDelete,   "AND ");
        whereNotBetween.reset(query()).append_if( hasSoftDelete,"AND ");
        whereNull.reset(query()).append_if( hasSoftDelete,      "AND ");
        whereNotNull.reset(query()).append_if( hasSoftDelete,   "AND ");
        join.reset(query());
        this->prepare_end(query());
        return this;
    }

    auto whereBuilder(){
        if constexpr(!std::is_same<MODEL,Query>::value){
            if(MODEL::UseSoftDelete()){
                where.reset(query()).chop_if_ended("AND ","");
            }else{
                where.reset(query()).append_if_not_ended("WHERE ");
            }
        }
        return where.clone();
    }

    //// HavingCPO<MODEL> having{};//Do not use HAVING for items that should be in the WHERE clause. For example, do not write the following:
    LimitCPO<MODEL> limit{};
    OrderByCPO<MODEL> orderBy{};
    GroupByCPO<MODEL> groupBy{};
    LatestCPO<MODEL> latest{};
    OldestCPO<MODEL> oldest;
    WhereCPO<MODEL> where{};
    WhereInCPO<MODEL> whereIn{};
    WhereNotInCPO<MODEL> whereNotIn{};
    WhereBetweenCPO<MODEL> whereBetween{};
    WhereNotBetweenCPO<MODEL> whereNotBetween{};
    WhereNullCPO<MODEL> whereNull{};
    WhereNotNullCPO<MODEL> whereNotNull{};
    JoinCPO<MODEL> join{};
};

template<typename MODEL = Query>
struct SelectAll : public Query{
    auto operator()() {
        trimmed()
         .append("SELECT * ");
        from.reset(query());
        return this;
    }
    FromCPO<MODEL> from{};
};

template<typename MODEL = Query>
struct SelectCPO :  public Query {

    template<typename ...Args>
    auto operator()(Args ...args) -> typename std::enable_if<(is_string_type<std::decay_t<Args>>::value && ...), decltype(this)>::type  {
        trimmed().append_if_not_empty(",").append_if_empty("SELECT");
        (space().append("`").append(args).append("`,"), ...);
        chop(",").space();
        from.reset(query());
        return this;
    }

    template<typename ...Args>
    auto select(Args ...args){
        SHARED_QUERY<SelectCPO<MODEL>> ptr = std::make_shared<SelectCPO<MODEL>>();
        ptr->reset(query());
        (*ptr.operator->())(args ...);
        return ptr;
    }
    FromCPO<MODEL> from{};
};

template<typename MODEL>
struct PugCPO :  public  OrmEnd<MODEL,PugCPO<MODEL>>, public Query{
    template<typename T1,  typename T2, typename std::enable_if<is_string_type<T1>::value && is_string_type<T2>::value>::type* = nullptr>
    auto operator()(T1 table, T2 value)   {
        trimmed().space().append("SELECT").space().append("`").append(value).append("`");
        space().append("FROM").space().append("`").append(table).append("`");
        this->prepare_end(query());
        return this;
    }

    template<typename T1,  typename T2, typename T3, typename std::enable_if<is_string_type<T1>::value && is_string_type<T2>::value>::type* = nullptr>
    auto operator()(T1 database, T2 table, T3 value)  {
        trimmed().space().append("SELECT").space().append("`").append(value).append("`");
        space().append("FROM").space().append("`").append(database).append("`.");
        append("`").append(table).append("`");
        this->prepare_end(query());
        return this;
    }
};

struct ExistsCPO;
struct DoesNotExistsCPO;

struct SqlBuilder{

    template<typename MODEL = Query, typename ...Args>
    static auto select(Args ...args){
        SHARED_QUERY<SelectCPO<MODEL>> ptr = std::make_shared<SelectCPO<MODEL>>();
        (*ptr.operator->())(args ...);
        return ptr;
    }

    template<typename MODEL = Query>
    static auto all(){
        SHARED_QUERY<SelectAll<MODEL>> ptr=std::make_shared<SelectAll<MODEL>>();
        (*ptr.operator->())();
        return ptr;
    }

private:
    SqlBuilder() = default;
    ~SqlBuilder() = default;
    SqlBuilder(const SqlBuilder&) = delete;
    SqlBuilder& operator=(const SqlBuilder&) = delete;
    SqlBuilder(SqlBuilder&&) = delete;
    SqlBuilder& operator=(SqlBuilder&&) = delete;
};


} // End Eloquent

#endif // SQL_QUERY_H
