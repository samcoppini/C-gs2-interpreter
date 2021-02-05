#pragma once

#include "command.hpp"

namespace gs2 {

class Block {
    private:
        std::vector<Command> _commands;

    public:
        static Block parseBytes(const std::vector<uint8_t> &code);

        void add(Command command);

        const std::vector<Command> &getCommands() const;
};

} // namespace gs2
