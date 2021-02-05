#include "catch2/catch.hpp"

#include "block.hpp"
#include "gs2exception.hpp"

void testParse(const std::string &code, const std::vector<std::string> &expected) {
    std::vector<uint8_t> codeBytes{code.begin(), code.end()};

    auto block = gs2::Block::parseBytes(codeBytes);
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
}
