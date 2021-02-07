#pragma once

#include <cstdint>
#include <vector>

namespace gs2 {

class Command;
class GS2Context;

class Block {
    private:
        std::vector<Command> _commands;

    public:
        Block();
        Block(const Block &);
        Block(Block &&);
        Block& operator=(const Block &);
        Block& operator=(Block &&);
        ~Block();

        static Block parseBytes(const std::vector<uint8_t> &code);

        void add(Command command);

        void execute(GS2Context &gs2) const;

        const std::vector<Command> &getCommands() const;
};

} // namespace gs2
