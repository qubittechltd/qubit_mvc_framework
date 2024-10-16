#ifndef EXTRA_DEFINED_FMT_H
#define EXTRA_DEFINED_FMT_H
#include <QString>
#include <QByteArray>
#include <fmt/core.h>
#include <string>
#include "hikocsp/generator.hpp"

namespace fmt {

template <>
struct formatter<QString> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const QString& str, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", str.toStdString());
    }
};

template <>
struct formatter<QByteArray> {
    // This function parses the format specification (no-op in this case)
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const QByteArray& byteArray, FormatContext& ctx) const {
        return fmt::format_to(ctx.out(), "{}", byteArray.toStdString());
    }
};

}
#endif // EXTRA_DEFINED_FMT_H
