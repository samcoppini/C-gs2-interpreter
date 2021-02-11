#include "catch2/catch.hpp"

#include "block.hpp"
#include "command.hpp"
#include "gs2exception.hpp"

gs2::Block parseBlock(const std::string &code) {
    std::vector<uint8_t> codeBytes{code.begin(), code.end()};
    return gs2::Block::parseBytes(codeBytes);
}

void testParse(const std::string &code, const std::vector<std::string> &expected) {
    auto block = parseBlock(code);
    auto &cmds = block.getCommands();

    REQUIRE(expected.size() == cmds.size());

    for (size_t i = 0; i < expected.size(); i++) {
        std::vector<uint8_t> expectedBytes(expected[i].begin(), expected[i].end());
        CHECK(expectedBytes == cmds[i].getBytes());
    }
}

void testFail(const std::string &code) {
    std::vector<uint8_t> codeBytes{code.begin(), code.end()};
    CHECK_THROWS_AS(gs2::Block::parseBytes(codeBytes), gs2::GS2Exception);
}

TEST_CASE("Testing code parsing") {
    // Normal case
    testParse("Hello there", {"H", "e", "l", "l", "o", " ", "t", "h", "e", "r", "e"});

    // Push unsigned byte
    testParse("\x01\xee\x01\xdd", {"\x01\xee", "\x01\xdd"});

    // Push signed short
    testParse("\x02\xee\x01\x02\x11\x22", {"\x02\xee\x01", "\x02\x11\x22"});

    // Push signed long
    testParse("\x03\x01\x02\x03\x04\xbb", {"\x03\x01\x02\x03\x04", "\xbb"});

    // Push one-character strings
    testParse("\x07\x32\x07\x42", {"\x07\x32", "\x07\x42"});

    // Push strings
    testParse("\x04hello there\x05\x04\x05", {"\x04hello there\x05", "\x04\x05"});
    testParse("\x04gamma\x9b\x04knife\x9c\x04rattlesnake\x9d",
              {"\x04gamma\x9b", "\x04knife\x9c", "\x04rattlesnake\x9d"});
    testParse("\x04???\x9e\x04!!!\x9f", {"\x04???\x9e", "\x04!!!\x9f"});

    // Push strings (with implicit beginnings)
    testParse("who\x05", {"\x04who\x05"});
    testParse("what\x06k", {"\x04what\x06", "k"});
    testParse("where\x9b??", {"\x04where\x9b", "?", "?"});
    testParse("why\x9c!!!", {"\x04why\x9c", "!", "!", "!"});
    testParse("how\x9d", {"\x04how\x9d"});
    testParse("which\x9e", {"\x04which\x9e"});
    testParse("\x01\x02\x03\x9f", {"\x04\x01\x02\x03\x9f"});

    // Now, some things which should fail to parse
    testFail("\x01");
    testFail("\x02?");
    testFail("\x03???");
    testFail("\x04 blah blah blah");
    testFail("\x07");
    testFail("\x09");
}

TEST_CASE("Testing different modes") {
    // Line mode
    auto commands = parseBlock("\x30Hey\x05?").getCommands();

    REQUIRE(commands.size() == 4);
    REQUIRE(commands[0].isBytes());
    CHECK(commands[0].getBytes() == std::vector<uint8_t>{ 0x2a });
    REQUIRE(commands[2].isBytes());
    CHECK(commands[2].getBytes() == std::vector<uint8_t>{ 0x34 });
    REQUIRE(commands[3].isBytes());
    CHECK(commands[3].getBytes() == std::vector<uint8_t>{ 0x54 });
    REQUIRE(commands[1].isBlock());
    auto blockCommands = commands[1].getBlock().getCommands();
    REQUIRE(blockCommands.size() == 2);
    REQUIRE(blockCommands[0].isBytes());
    CHECK(blockCommands[0].getBytes() == std::vector<uint8_t>{ 0x04, 'H', 'e', 'y', 0x05 });
    REQUIRE(blockCommands[1].isBytes());
    CHECK(blockCommands[1].getBytes() == std::vector<uint8_t>{ '?' });

}

TEST_CASE("Testing block parses") {
    // Simple block case
    auto commands = parseBlock("\x08hey\x09").getCommands();

    REQUIRE(commands.size() == 2);
    REQUIRE(commands[1].isBytes());
    REQUIRE(commands[1].getBytes() == std::vector<uint8_t>{ 0 });
    REQUIRE(commands[0].isBlock());
    commands = commands[0].getBlock().getCommands();
    REQUIRE(commands.size() == 3);
    REQUIRE(commands[0].isBytes());
    CHECK(commands[0].getBytes() == std::vector<uint8_t>{ 'h' });
    REQUIRE(commands[1].isBytes());
    CHECK(commands[1].getBytes() == std::vector<uint8_t>{ 'e' });
    REQUIRE(commands[2].isBytes());
    CHECK(commands[2].getBytes() == std::vector<uint8_t>{ 'y' });

    // Nested blocks
    commands = parseBlock("\x08\x08\x08\x08").getCommands();
    for (int i = 0; i < 4; i++) {
        REQUIRE(commands.size() == 2);
        REQUIRE(commands[1].isBytes());
        CHECK(commands[1].getBytes() == std::vector<uint8_t>{ 0 });

        REQUIRE(commands[0].isBlock());
        if (i == 3) {
            REQUIRE(commands[0].getBlock().getCommands().empty());
        }
        else {
            auto innerBlock = commands[0].getBlock().getCommands();
            commands = innerBlock;
        }
    }

    // Filter block
    commands = parseBlock("\xff?!").getCommands();

    REQUIRE(commands.size() == 2);
    REQUIRE(commands[1].isBytes());
    CHECK(commands[1].getBytes() == std::vector<uint8_t>{ 0x35 });
    REQUIRE(commands[0].isBlock());
    auto innerBlock = commands[0].getBlock().getCommands();
    REQUIRE(innerBlock.size() == 2);
    REQUIRE(innerBlock[0].isBytes());
    CHECK(innerBlock[0].getBytes() == std::vector<uint8_t>{ '?' });
    REQUIRE(innerBlock[1].isBytes());
    CHECK(innerBlock[1].getBytes() == std::vector<uint8_t>{ '!' });

    // Map block
    commands = parseBlock("\xfe?!").getCommands();

    REQUIRE(commands.size() == 2);
    REQUIRE(commands[1].isBytes());
    CHECK(commands[1].getBytes() == std::vector<uint8_t>{ 0x34 });
    REQUIRE(commands[0].isBlock());
    innerBlock = commands[0].getBlock().getCommands();
    REQUIRE(innerBlock.size() == 2);
    REQUIRE(innerBlock[0].isBytes());
    CHECK(innerBlock[0].getBytes() == std::vector<uint8_t>{ '?' });
    REQUIRE(innerBlock[1].isBytes());
    CHECK(innerBlock[1].getBytes() == std::vector<uint8_t>{ '!' });
}
