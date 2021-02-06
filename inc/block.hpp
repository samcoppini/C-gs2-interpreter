#pragma once

#include "command.hpp"

namespace gs2 {

class GS2Context;

class Block {
    private:
        std::vector<Command> _commands;

    public:
        static Block parseBytes(const std::vector<uint8_t> &code);

        void add(Command command);

        void execute(GS2Context &gs2) const;

        const std::vector<Command> &getCommands() const;
};

} // namespace gs2
