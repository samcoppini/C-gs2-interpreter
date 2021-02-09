#pragma once

#include <string>

namespace gs2 {

class List;
class Value;

List makeList(const std::string &str);

std::string makeString(Value value);

List split(List toSplit, const List &sep);

} // namespace gs2
