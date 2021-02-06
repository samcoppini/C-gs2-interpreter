#pragma once

#include "List.hpp"

#include <cstdint>
#include <variant>

namespace gs2 {

class Value {
    private:
        std::variant<int64_t, List> _data;

    public:
        Value(int64_t num);
        Value(List list);

        bool isNumber() const;
        int64_t getNumber() const;

        bool isList() const;
        const List& getList() const; 
        List& getList();
};

} // namespace gs2
