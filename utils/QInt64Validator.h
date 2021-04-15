#ifndef QINT64VALIDATOR_H
#define QINT64VALIDATOR_H

#include <QValidator>
#include <cstdint>

class QInt64Validator : public QValidator {
    Q_OBJECT
public:
    using value_type = std::int64_t;

public:
    explicit QInt64Validator(QObject *parent = nullptr)
        : QValidator(parent) {}

    QInt64Validator(value_type minimum, value_type maximum, QObject *parent = nullptr)
        : QValidator(parent), minimum_(minimum), maximum_(maximum) {}

    ~QInt64Validator() override = default;

public:
    value_type bottom() const {
        return minimum_;
    }

    value_type top() const {
        return maximum_;
    }

    QValidator::State validate(QString &input, int &pos) const override {
        Q_UNUSED(pos)

        if (input.isEmpty()) {
            return QValidator::Acceptable;
        }

        if (input == "-") {
            return QValidator::Intermediate;
        }

        bool ok;
        const value_type temp = input.toLongLong(&ok);
        if (!ok) {
            return QValidator::Invalid;
        }

        return (temp >= bottom() && temp <= top()) ? QValidator::Acceptable : QValidator::Invalid;
    }

    void setRange(value_type bottom, value_type top){
        setBottom(bottom);
        setTop(top);
    }

    void setBottom(value_type bottom){
        minimum_ = bottom;
    }

    void setTop(value_type top){
        maximum_ = top;
    }

private:
    value_type minimum_ = 0;
    value_type maximum_ = 0;
};


#endif // QINT64VALIDATOR_H
