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

TEST_CASE("Test 0x21 - bnot / head") {
    // Push 10 to the stack, then bitwise negate it
    auto result = getResult("\x1a\x21");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == ~10);

    gs2::List sampleList;
    sampleList.add(10);
    sampleList.add(gs2::Block());
    sampleList.add(gs2::List());

    result = getResult("\x21", {sampleList});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    REQUIRE(result[0].getNumber() == 10);

    // 0x21 cannot handle empty list
    REQUIRE_THROWS_AS(getResult("\x21", {gs2::List{}}), gs2::GS2Exception);

    // 0x21 cannot handle blocks
    REQUIRE_THROWS_AS(getResult("\x21", {gs2::Block()}), gs2::GS2Exception);
}

TEST_CASE("Test 0x22 - not / head") {
    auto result = getResult("\x22", {0});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 1);

    result = getResult("\x22", {1});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 0);

    gs2::List sampleList;
    sampleList.add(10);
    sampleList.add(gs2::Block());
    sampleList.add(gs2::makeList("test"));

    result = getResult("\x22", {sampleList});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    compareString(result[0].getList(), "test");

    // 0x22 cannot handle empty list
    REQUIRE_THROWS_AS(getResult("\x22", {gs2::List{}}), gs2::GS2Exception);

    // 0x22 cannot handle blocks
    REQUIRE_THROWS_AS(getResult("\x22", {gs2::Block()}), gs2::GS2Exception);
}

TEST_CASE("Test 0x23 - abs / init") {
    auto result = getResult("\x23", {-1234});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 1234);

    gs2::List list = gs2::makeList("fool!");
    result = getResult("\x23\x23", {list});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    compareString(result[0].getList(), "foo");

    gs2::List emptyList;
    result = getResult("\x23", {emptyList});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    CHECK(result[0].getList().empty());

    // Blocks are invalid for 0x23
    REQUIRE_THROWS_AS(getResult("\x23", {gs2::Block()}), gs2::GS2Exception);
}

TEST_CASE("Test 0x24 - digits / last") {
    auto result = getResult("\x24", {8590});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    compareString(result[0].getList(), {"\x08\x05\x09\x00", 4});

    result =  getResult("\x24", {gs2::makeList("test!")});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == '!');

    // Last operation should fail on an empty list
    REQUIRE_THROWS_AS(getResult("\x24", {gs2::List{}}), gs2::GS2Exception);

    // Blocks not supported with 0x24 command
    REQUIRE_THROWS_AS(getResult("\x24", {gs2::Block()}), gs2::GS2Exception);
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
    // Note that having 0x30 at the start of a parsed block triggers line mode,
    // so to avoid that a nop (0x00) is inserted before each 0x30

    SECTION("Error cases") {
        // Can't add empty stack
        CHECK_THROWS_AS(getResult({"\x00\x30", 2}), gs2::GS2Exception);

        // Can't add stack with only one value
        CHECK_THROWS_AS(getResult({"\x00\x30", 2}, {10}), gs2::GS2Exception);

        // Can't add block and int
        CHECK_THROWS_AS(getResult({"\x00\x30", 2}, {gs2::Block{}, 10}), gs2::GS2Exception);
    }

    SECTION("Successful cases") {
        // Adding two numbers
        auto result = getResult({"\x00\x30", 2}, {12, 30});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isNumber());
        CHECK(result[0].getNumber() == 42);

        // Adding two lists
        gs2::List list1, list2;
        list1.add(10);
        list2.add(20);
        result = getResult({"\x00\x30", 2}, {list1, list2});
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
        result = getResult({"\x00\x30", 2}, {block1, block2});
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
        result = getResult({"\x00\x30", 2}, {list1, 11});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isList());
        resultList = result[0].getList();
        REQUIRE(resultList.size() == 2);
        REQUIRE(resultList[0].isNumber());
        CHECK(resultList[0].getNumber() == 10);
        REQUIRE(resultList[1].isNumber());
        CHECK(resultList[1].getNumber() == 11);

        // Adding a number to the beginning of a list
        result = getResult({"\x00\x30", 2}, {9, list1});
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

TEST_CASE("Test 0x34 - mod / step / clean-split / map") {
    SECTION("mod") {
        auto result = getResult("\x34", {18715338, 16252});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isNumber());
        CHECK(result[0].getNumber() == 9286);
    }

    SECTION("step") {
        gs2::List list;
        list.add(1);
        list.add(2);
        list.add(3);
        list.add(4);
        list.add(5);

        auto result = getResult("\x34", {list, 2});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isList());
        auto skippedList = result[0].getList();
        REQUIRE(skippedList.size() == 3);
        REQUIRE(skippedList[0].isNumber());
        CHECK(skippedList[0].getNumber() == 1);
        REQUIRE(skippedList[1].isNumber());
        CHECK(skippedList[1].getNumber() == 3);
        REQUIRE(skippedList[2].isNumber());
        CHECK(skippedList[2].getNumber() == 5);

        result = getResult("\x34", {-3, list});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isList());
        skippedList = result[0].getList();
        REQUIRE(skippedList.size() == 2);
        REQUIRE(skippedList[0].isNumber());
        CHECK(skippedList[0].getNumber() == 5);
        REQUIRE(skippedList[1].isNumber());
        CHECK(skippedList[1].getNumber() == 2);

        // Zero is an invalid step size
        CHECK_THROWS_AS(getResult("\x34", {list, 0}), gs2::GS2Exception);
    }

    SECTION("clean-split") {
        gs2::List toSplit = gs2::makeList("blahahahablahblah");
        gs2::List sep = gs2::makeList("ah");

        auto result = getResult("\x34", {toSplit, sep});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isList());

        auto list = result[0].getList();
        REQUIRE(list.size() == 3);
        REQUIRE(list[0].isList());
        compareString(list[0].getList(), "bl");
        REQUIRE(list[1].isList());
        compareString(list[1].getList(), "abl");
        REQUIRE(list[2].isList());
        compareString(list[2].getList(), "bl");
    }

    SECTION("map") {
        auto result = getResult({"\x04\x00\x01\x02\x03\x05\x08\x2a\x09\x34", 10});
        REQUIRE(result.size() == 1);
        REQUIRE(result[0].isList());
        auto mappedList = result[0].getList();
        REQUIRE(mappedList.size() == 4);
        REQUIRE(mappedList[0].isNumber());
        REQUIRE(mappedList[0].getNumber() == 0);
        REQUIRE(mappedList[1].isNumber());
        REQUIRE(mappedList[1].getNumber() == 2);
        REQUIRE(mappedList[2].isNumber());
        REQUIRE(mappedList[2].getNumber() == 4);
        REQUIRE(mappedList[3].isNumber());
        REQUIRE(mappedList[3].getNumber() == 6);
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

TEST_CASE("Test 0x54 - show-lines") {
    gs2::List list;
    list.add(gs2::makeList("Hey"));
    list.add(gs2::makeList("What?"));
    list.add(gs2::makeList(""));
    list.add(gs2::makeList("Blah"));

    auto result = getResult("\x54", {list});

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    compareString(result[0].getList(), "Hey\nWhat?\n\nBlah");

    // Empty stack should throw
    CHECK_THROWS_AS(getResult("\x54"), gs2::GS2Exception);

    // Trying show-lines on a block should throw
    CHECK_THROWS_AS(getResult("\x54", {11}), gs2::GS2Exception);
}

TEST_CASE("Test 0x55 - show-lines") {
    gs2::List list;
    list.add(gs2::makeList("Hey"));
    list.add(gs2::makeList("What?"));
    list.add(gs2::makeList(""));
    list.add(gs2::makeList("Blah"));

    auto result = getResult("\x55", {list});

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    compareString(result[0].getList(), "Hey What?  Blah");

    // Empty stack should throw
    CHECK_THROWS_AS(getResult("\x55"), gs2::GS2Exception);

    // Trying show-lines on a block should throw
    CHECK_THROWS_AS(getResult("\x55", {11}), gs2::GS2Exception);
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

TEST_CASE("Test 0x58 - show-line") {
    // Converting a number to a string
    auto result = getResult("\x58", {1234});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());

    auto resultList = result[0].getList();
    compareString(resultList, "1234\n");

    // Converting a string back to a string
    result = getResult("\x58", {gs2::makeList("abcd")});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    resultList = result[0].getList();
    compareString(resultList, "abcd\n");

    // Trying to convert a block should throw
    CHECK_THROWS_AS(getResult("\x58", {gs2::Block()}), gs2::GS2Exception);
}

TEST_CASE("Test 0x59 - show-space") {
    // Converting a number to a string
    auto result = getResult("\x59", {1234});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());

    auto resultList = result[0].getList();
    compareString(resultList, "1234 ");

    // Converting a string back to a string
    result = getResult("\x59", {gs2::makeList("abcd")});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isList());
    resultList = result[0].getList();
    compareString(resultList, "abcd ");

    // Trying to convert a block should throw
    CHECK_THROWS_AS(getResult("\x59", {gs2::Block()}), gs2::GS2Exception);
}

TEST_CASE("Test 0x64 - sum / even") {
    auto result = getResult("\x64", {111});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 0);

    result = getResult("\x64", {222});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 1);

    result = getResult("\x04\x01\x02\x03\x04\x05\x64");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 10);

    // Trying to sum a list with a non-number should throw
    gs2::List list;
    list.add(list);
    CHECK_THROWS_AS(getResult("\x64", {list}), gs2::GS2Exception);

    // Blocks should throw
    CHECK_THROWS_AS(getResult("\x64", {gs2::Block()}), gs2::GS2Exception);
}

TEST_CASE("Test 0x65 - sum / even") {
    auto result = getResult("\x65", {111});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 1);

    result = getResult("\x65", {222});
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 0);

    result = getResult("\x04\x01\x02\x03\x04\x05\x65");
    REQUIRE(result.size() == 1);
    REQUIRE(result[0].isNumber());
    CHECK(result[0].getNumber() == 24);

    // Trying to multiply a list with a non-number should throw
    gs2::List list;
    list.add(list);
    CHECK_THROWS_AS(getResult("\x65", {list}), gs2::GS2Exception);

    // Blocks should throw
    CHECK_THROWS_AS(getResult("\x65", {gs2::Block()}), gs2::GS2Exception);
}

TEST_CASE("Test 0xb2 - counter") {
    auto result = getResult("\xb2\xb2\xb2\xb2\xb2");
    REQUIRE(result.size() == 5);

    for (int i = 0; i < 5; i++) {
        REQUIRE(result[i].isNumber());
        REQUIRE(result[i].getNumber() == i + 1);
    }
}
