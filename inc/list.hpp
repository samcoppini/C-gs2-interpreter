#pragma once

#include <vector>

namespace gs2 {

class Value;

class List {
    private:
        std::vector<Value> _values;

    public:
        List();
        ~List();
        
        List(const List &) = default;
        List(List &&) = default;
        List& operator=(const List &) = default;
        List& operator=(List &&) = default;

        bool operator!=(const List &rhs) const;

        void add(Value val);
        void concat(const List &);
        void insert(std::vector<Value>::iterator it, Value value);
        Value pop();
        void clear();
        void reverse();

        std::vector<Value>::iterator begin();
        std::vector<Value>::iterator end();
        std::vector<Value>::const_iterator begin() const;
        std::vector<Value>::const_iterator end() const;

        Value& operator[](size_t index);
        const Value& operator[](size_t index) const;

        Value &back();
        const Value &back() const;
        size_t size() const;
        bool empty() const;
};

} // namespace gs2
