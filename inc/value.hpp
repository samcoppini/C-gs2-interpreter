#pragma once

#include "block.hpp"
#include "list.hpp"

#include <cstdint>
#include <string>
#include <variant>

namespace gs2 {

class Value {
    private:
        std::variant<int64_t, List, Block> _data;

    public:
        Value(int64_t num);
        Value(List list);
        Value(Block block);

        bool operator!=(const Value &rhs) const;

        bool isNumber() const;
        int64_t getNumber() const;

        bool isList() const;
        const List& getList() const;
        List& getList();

        bool isBlock() const;
        const Block& getBlock() const;
        Block& getBlock();

        std::string str(bool nested=false) const;
};

} // namespace gs2
