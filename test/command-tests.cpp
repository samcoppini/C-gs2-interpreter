#include "catch2/catch.hpp"

#include "block.hpp"
#include "command.hpp"
#include "gs2context.hpp"
#include "gs2exception.hpp"
#include "list.hpp"
#include "utils.hpp"

#include <iostream>

gs2::List getResult(const std::string &code, const std::vector<gs2::Value> &initialStack = {}) {
    std::vector<uint8_t> codeBytes{code.begin(), code.end()};
    auto block = gs2::Block::parseBytes(codeBytes);

    gs2::List list;
    for (const auto &val: initialStack) {
        list.add(val);
    }

    gs2::GS2Context gs2{list};
    block.execute(gs2);
    return list;
}

void compareString(const gs2::List &list, const std::string &expected) {
    CHECK(expected.size() == list.size());

    if (expected.size() == list.size()) {
        for (size_t i = 0; i < list.size(); i++) {
            CHECK(list[i].isNumber());

            if (list[i].isNumber()) {
                CHECK(list[i].getNumber() == static_cast<uint8_t>(expected[i]));
            }
        }
    }
}

TEST_CASE("Test 0x00 - nop") {
    gs2::List result;

    CHECK_NOTHROW(result = getResult({"\x00", 1}));
    CHECK(result.size() == 0);
}

TEST_CASE("Test 0x01 - push an unsigned byte") {
    auto result = getResult({"\x01\x42\x01\xFF\x01\x00", 6});

    REQUIRE(result.size() == 3);
    REQUIRE(result[0].isNumber());
    REQUIRE(result[1].isNumber());
    REQUIRE(result[2].isNumber());

    CHECK(result[0].getNumber() == 66);
    CHECK(result[1].getNumber() == 255);
    CHECK(result[2].getNumber() == 0);
}

TEST_CASE("Test 0x02 - push a signed short") {
    auto result = getResult({"\x02\xff\x00\x02\xd6\xff\x02\x00\x00", 9});

    REQUIRE(result.size() == 3);
    REQUIRE(result[0].isNumber());
    REQUIRE(result[1].isNumber());

    CHECK(result[0].getNumber() == 255);
    CHECK(result[1].getNumber() == -42);
    CHECK(result[2].getNumber() == 0);
}

TEST_CASE("Test 0x03 - push a signed long") {
    auto result = getResult("\x03\x13\x6d\xae\x37");

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 0x37AE6D13);

    result = getResult("\x03\xd6\xff\xff\xff");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == -42);
}

TEST_CASE("Test 0x05 - pushing strings") {
    auto result = getResult("Hello, World!\x05");

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    compareString(result[0].getList(), "Hello, World!");

    result = getResult("\x04hey\x07what\x07\x05");

    REQUIRE(result.size() == 3);
    REQUIRE(result[0].isList());
    compareString(result[0].getList(), "hey");
    REQUIRE(result[1].isList());
    compareString(result[1].getList(), "what");
    REQUIRE(result[2].isList());
    compareString(result[2].getList(), "");
}

TEST_CASE("Test 0x06 - pushing a list of strings") {
    auto result = getResult("\x04Hello\x07\x07World\x06");

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    auto pushedList = result[0].getList();
    REQUIRE(pushedList.size() == 3);
    REQUIRE(pushedList[0].isList());
    compareString(pushedList[0].getList(), "Hello");
    REQUIRE(pushedList[1].isList());
    compareString(pushedList[1].getList(), "");
    REQUIRE(pushedList[2].isList());
    compareString(pushedList[2].getList(), "World");

    result = getResult("a\x07" "b\x07" "c\x07" "d\x06\x04" "1\x07" "2\x07" "3\x06");
    REQUIRE(result.size() == 2);
    
    REQUIRE(result[0].isList());
    pushedList = result[0].getList();
    REQUIRE(pushedList.size() == 4);
    REQUIRE(pushedList[0].isList());
    compareString(pushedList[0].getList(), "a");
    REQUIRE(pushedList[1].isList());
    compareString(pushedList[1].getList(), "b");
    REQUIRE(pushedList[2].isList());
    compareString(pushedList[2].getList(), "c");
    REQUIRE(pushedList[3].isList());
    compareString(pushedList[3].getList(), "d");
    
    REQUIRE(result[1].isList());
    pushedList = result[1].getList();
    REQUIRE(pushedList.size() == 3);
    REQUIRE(pushedList[0].isList());
    compareString(pushedList[0].getList(), "1");
    REQUIRE(pushedList[1].isList());
    compareString(pushedList[1].getList(), "2");
    REQUIRE(pushedList[2].isList());
    compareString(pushedList[2].getList(), "3");
}

TEST_CASE("Test 0x07 - push a single character string") {
    auto result = getResult("\x07\xFF\x07\x20");

    REQUIRE(result.size() == 2);
    REQUIRE(result[0].isList());
    compareString(result[0].getList(), "\xFF");
    REQUIRE(result[1].isList());
    compareString(result[1].getList(), "\x20");
}

TEST_CASE("Test 0x0a - pushing a newline to the stack") {
    auto result = getResult("\x0a");

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    compareString(result[0].getList(), "\n");
}

TEST_CASE("Test 0x0b - pushing an empty list") {
    auto result = getResult("\x0b");

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    CHECK(result[0].getList().empty());
}

TEST_CASE("Test 0x0c - pushing an empty block") {
    auto result = getResult("\x0c");

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isBlock());
    CHECK(result[0].getBlock().getCommands().empty());
}

TEST_CASE("Test 0x0d - pushing a space") {
    auto result = getResult("\x0d");

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    compareString(result[0].getList(), " ");
}

TEST_CASE("Test 0x10 through 0x1a - pushing small numbers") {
    for (auto i = 0; i <= 10; i++) {
        char buf[2] = { static_cast<char>(0x10 + i), '\0' };
        auto result = getResult(buf);

        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isNumber());
        CHECK(result[0].getNumber() == i);
    }
}

TEST_CASE("Test 0x1b - push 100") {
    auto result = getResult("\x1b");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 100);
}

TEST_CASE("Test 0x1c - push 1000") {
    auto result = getResult("\x1c");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 1000);
}

TEST_CASE("Test 0x1d - push 16") {
    auto result = getResult("\x1d");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 16);
}

TEST_CASE("Test 0x1e - push 64") {
    auto result = getResult("\x1e");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 64);
}

TEST_CASE("Test 0x1f - push 256") {
    auto result = getResult("\x1f");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 256);
}

TEST_CASE("Test 0x20 - negate / reverse / eval") {
    // Push 10 to the stack, then negate it
    auto result = getResult("\x1a\x20");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == -10);

    // Push the string Hello to the stack, then reverse it
    result = getResult("\x04Hello\x05\x20");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    compareString(result[0].getList(), "olleH");

    // Push a block that pushes 10 to the stack, then execute it
    result = getResult("\x08\x1a\x09\x20");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 10);
}

TEST_CASE("Test 0x2a - double / lines") {
    // Push 1234 to the stack, then double it
    auto result = getResult("\x2a", {1234});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 2468);

    // Split a string on newlines
    result = getResult("\x2a", {gs2::makeList("aaaa\nbbb\n\ncc\nd\n")});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    auto splitLines = result[0].getList();
    REQUIRE(splitLines.size() == 5);
    REQUIRE(splitLines[0].isList());
    compareString(splitLines[0].getList(), "aaaa");
    REQUIRE(splitLines[1].isList());
    compareString(splitLines[1].getList(), "bbb");
    REQUIRE(splitLines[2].isList());
    compareString(splitLines[2].getList(), "");
    REQUIRE(splitLines[3].isList());
    compareString(splitLines[3].getList(), "cc");
    REQUIRE(splitLines[4].isList());
    compareString(splitLines[4].getList(), "d");

    result = getResult("\x2a", {gs2::List()});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    splitLines = result[0].getList();
    REQUIRE(splitLines.size() == 1);
    REQUIRE(splitLines[0].isList());
    compareString(splitLines[0].getList(), "");
}

TEST_CASE("Test 0x30 - add / catenate") {
    SECTION("Error cases") {
        // Can't add empty stack
        CHECK_THROWS_AS(getResult("\x30"), gs2::GS2Exception);

        // Can't add stack with only one value
        CHECK_THROWS_AS(getResult("\x30", {10}), gs2::GS2Exception);

        // Can't add block and int
        CHECK_THROWS_AS(getResult("\x30", {gs2::Block{}, 10}), gs2::GS2Exception);
    }

    SECTION("Successful cases") {
        // Adding two numbers
        auto result = getResult("\x30", {12, 30});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isNumber());
        CHECK(result[0].getNumber() == 42);

        // Adding two lists
        gs2::List list1, list2;
        list1.add(10);
        list2.add(20);
        result = getResult("\x30", {list1, list2});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isList());
        auto resultList = result[0].getList();
        REQUIRE(resultList.size() == 2);
        REQUIRE(resultList[0].isNumber());
        CHECK(resultList[0].getNumber() == 10);
        REQUIRE(resultList[1].isNumber());
        CHECK(resultList[1].getNumber() == 20);

        // Combining two blocks
        gs2::Block block1, block2;
        block1.add(std::vector<uint8_t>{10});
        block2.add(std::vector<uint8_t>{1, 11});
        result = getResult("\x30", {block1, block2});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isBlock());
        auto resultBlock = result[0].getBlock();
        auto resultCommands = resultBlock.getCommands();
        REQUIRE(resultCommands.size() == 2);
        REQUIRE(resultCommands[0].isBytes());
        CHECK(resultCommands[0].getBytes() == std::vector<uint8_t>{10});
        REQUIRE(resultCommands[1].isBytes());
        CHECK(resultCommands[1].getBytes() == std::vector<uint8_t>{1, 11});

        // Adding a number to the end of a list
        result = getResult("\x30", {list1, 11});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isList());
        resultList = result[0].getList();
        REQUIRE(resultList.size() == 2);
        REQUIRE(resultList[0].isNumber());
        CHECK(resultList[0].getNumber() == 10);
        REQUIRE(resultList[1].isNumber());
        CHECK(resultList[1].getNumber() == 11);

        // Adding a number to the beginning of a list
        result = getResult("\x30", {9, list1});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isList());
        resultList = result[0].getList();
        REQUIRE(resultList.size() == 2);
        REQUIRE(resultList[0].isNumber());
        CHECK(resultList[0].getNumber() == 9);
        REQUIRE(resultList[1].isNumber());
        CHECK(resultList[1].getNumber() == 10);

    }
}

TEST_CASE("Test 0x50 - pop") {
    auto result = getResult("\x50", {1, 2, 3});
    REQUIRE(result.size() == 2);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 1);
    REQUIRE(result[1].isNumber());
    CHECK(result[1].getNumber() == 2);

    result = getResult("\x50", {2, 3});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 2);

    result = getResult("\x50", {3});
    CHECK(result.empty());

    // Popping an empty stack should throw an exception
    CHECK_THROWS_AS(getResult("\x50", {}), gs2::GS2Exception);
}

TEST_CASE("Test 0x51 - pop2") {
    auto result = getResult("\x51", {1, 2, 3});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 1);

    result = getResult("\x51", {2, 3});
    CHECK(result.empty());

    // Calling pop2 on a stack with less than 2 values should throw
    CHECK_THROWS_AS(getResult("\x51", {3}), gs2::GS2Exception);
    CHECK_THROWS_AS(getResult("\x51", {}), gs2::GS2Exception);
}

TEST_CASE("Test 0x52 - show") {
    // Converting a number to a string
    auto result = getResult("\x52", {1234});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());

    auto resultList = result[0].getList();
    compareString(resultList, "1234");

    // Converting a string back to a string
    result = getResult("\x52", {gs2::makeList("abcd")});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    resultList = result[0].getList();
    compareString(resultList, "abcd");

    // Trying to convert a block should throw
    CHECK_THROWS_AS(getResult("\x52", {gs2::Block()}), gs2::GS2Exception);
}

TEST_CASE("Test 0x56 - read-num") {
    SECTION("Error cases") {
        // Can't read number off of an empty stack
        CHECK_THROWS_AS(getResult("\x56"), gs2::GS2Exception);

        // Can't read number from a block
        CHECK_THROWS_AS(getResult("\x08 10 \x09\x56"), gs2::GS2Exception);

        // Can't read non-number string
        CHECK_THROWS_AS(getResult("\x04hey\x05\x56"), gs2::GS2Exception);
    }

    SECTION("Successful cases") {
        auto result = getResult("([{-99}])\x05\x56");
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isNumber());
        CHECK(result[0].getNumber() == -99);

        result = getResult("\x01\x30\x56");
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isNumber());
        CHECK(result[0].getNumber() == 0);

        result = getResult("11,-12,13,-14\x05\x56");
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isNumber());
        CHECK(result[0].getNumber() == 11);
    }
}

TEST_CASE("Test 0x57 - read-nums") {
    SECTION("Error cases") {
        // Can't read number off of an empty stack
        CHECK_THROWS_AS(getResult("\x57"), gs2::GS2Exception);

        // Can't read number from a block
        CHECK_THROWS_AS(getResult("\x08 10 \x09\x57"), gs2::GS2Exception);
    }

    SECTION("Successful cases") {
        auto result = getResult("this is not a number\x05\x57");
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isList());
        auto numList = result[0].getList();
        CHECK(numList.size() == 0);

        result = getResult("\x01\x30\x57");
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isList());
        numList = result[0].getList();
        REQUIRE(numList.size() == 1);
        REQUIRE(numList[0].isNumber());
        CHECK(numList[0].getNumber() == 0);

        result = getResult("11,-12,13,-14\x05\x57");
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isList());
        numList = result[0].getList();
        REQUIRE(numList.size() == 4);
        REQUIRE(numList[0].isNumber());
        CHECK(numList[0].getNumber() == 11);
        REQUIRE(numList[1].isNumber());
        CHECK(numList[1].getNumber() == -12);
        REQUIRE(numList[2].isNumber());
        CHECK(numList[2].getNumber() == 13);
        REQUIRE(numList[3].isNumber());
        CHECK(numList[3].getNumber() == -14);
    }
}
