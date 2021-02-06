#include "catch2/catch.hpp"

#include "block.hpp"
#include "gs2context.hpp"
#include "list.hpp"

#include <iostream>

gs2::List getResult(const std::string &code) {
    std::vector<uint8_t> codeBytes{code.begin(), code.end()};
    auto block = gs2::Block::parseBytes(codeBytes);

    gs2::List list;
    gs2::GS2Context gs2{std::cout, list};
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
