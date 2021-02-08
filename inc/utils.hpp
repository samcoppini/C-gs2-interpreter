#pragma once

#include <string>

namespace gs2 {

class List;
class Value;

std::string makeString(Value value);

List split(List toSplit, const List &sep);

} // namespace gs2
