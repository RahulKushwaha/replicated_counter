
//
// Created by Rahul  Kushwaha on 4/3/23.
//

#include <variant>
#include "TableRow.h"

namespace rk::projects::mydb{

    using ColumnValue = std::variant<std::int64_t, std::string>;

    std::string toString(const ColumnValue &rowValue) {
      return std::visit([](auto &&arg) {
        using a = std::decay_t<decltype(arg)>;
        if constexpr(std::is_same_v < a, std::string > )
        return arg;
        else if constexpr(std::is_same_v < a, std::int64_t > )
        return std::to_string(arg);
      }, rowValue);
    }

}