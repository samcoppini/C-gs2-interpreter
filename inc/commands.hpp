#pragma once

#include <string>

namespace gs2 {

class GS2Context;
class Value;

std::string makeString(Value value);

void catenate(GS2Context &);

void negate(GS2Context &);

void readNum(GS2Context &);

void readNums(GS2Context &);

} // namespace gs2
