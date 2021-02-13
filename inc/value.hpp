#pragma once

#include "block.hpp"
#include "list.hpp"

#include <boost/multiprecision/cpp_int.hpp>

#include <cstdint>
#include <string>
#include <variant>

namespace gs2 {

class Value {
    public:
        using IntType = boost::multiprecision::cpp_int;

    private:
        std::variant<IntType, List, Block> _data;

    public:
        Value(int64_t num);
        Value(IntType num);
        Value(List list);
        Value(Block block);

        bool operator!=(const Value &rhs) const;

        bool isNumber() const;
        const IntType& getNumber() const;
        IntType& getNumber();

        bool isList() const;
        const List& getList() const;
        List& getList();

        bool isBlock() const;
        const Block& getBlock() const;
        Block& getBlock();

        std::string str(bool nested=false) const;
};

} // namespace gs2
