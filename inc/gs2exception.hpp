#pragma once

#include <stdexcept>

namespace gs2 {

class GS2Exception: public std::runtime_error {
    public:
        GS2Exception(const std::string &msg): runtime_error(msg) {}
};

} // namespace gs2
