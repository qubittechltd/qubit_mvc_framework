#ifndef SANITIZER_H
#define SANITIZER_H

#include <QObject>
#include <QString>
#include <QRegularExpression>

class Sanitizer : public QObject
{
    Q_OBJECT
public:
    explicit Sanitizer(QObject *parent = nullptr);

    static bool isContainsOnlyString(const QString &input) {
        // Regular expression pattern to match only letters from 'a' to 'z' and 'A' to 'Z'

        static const QRegularExpression re("^[a-zA-Z]*$");
        QRegularExpressionMatch match = re.match(input);
        return match.hasMatch();
    }

    static bool isValidFullName(const QString &fullName) {
        // Regular expression pattern to match full name

        static const QRegularExpression re("^[a-zA-Z]+( [a-zA-Z]+)+$");
        QRegularExpressionMatch match = re.match(fullName);
        return match.hasMatch();
    }

    static bool isValidEmail(const QString &email) {
        // Regular expression pattern to match email address

        static const QRegularExpression regex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        return regex.match(email).hasMatch();
    }

    static bool isValidUserName(const QString &userName) {
        // Regular expression pattern to match user name

        static const QRegularExpression regex("^[a-zA-Z][a-zA-Z0-9_]*$");
        return regex.match(userName).hasMatch();
    }

    static bool isValidDate(const QString &date) {
        // Regular expression pattern to match date in YYYY-MM-DD or MM-DD-YYYY format

        static const QRegularExpression regex("^\\d{4}-\\d{2}-\\d{2}$|^\\d{2}-\\d{2}-\\d{4}$");
        return regex.match(date).hasMatch();
    }

    static bool isStrongPassword(const QString &password) {
        // Regular expression pattern to match strong password

        static const QRegularExpression regex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,}$");
        return regex.match(password).hasMatch();
    }

signals:
};

#endif // SANITIZER_H
