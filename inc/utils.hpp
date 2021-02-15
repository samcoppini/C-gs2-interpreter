#pragma once

#include <string>

namespace gs2 {

class List;
class Value;

List makeList(const std::string &str);

std::string makeString(Value value);

List join(List toJoin, const List &separator);

List split(List toSplit, const List &sep, bool clean = false);

List stepOver(List list, int64_t stepSize);

} // namespace gs2
