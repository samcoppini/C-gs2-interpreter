#include "catch2/catch.hpp"

#include "gs2exception.hpp"
#include "utils.hpp"
#include "value.hpp"

TEST_CASE("makeString tests") {
    // Test converting a number to a string
    auto result = gs2::makeString(0x42);
    CHECK(result == "\x42");

    gs2::List list;
    list.add('H');
    list.add('e');
    list.add('l');
    list.add('l');
    list.add('o');

    // Test converting a list of numbers to a string
    result = gs2::makeString(list);
    CHECK(result == "Hello");

    list.add(gs2::List{});

    // A list with a non-number inside of it should throw
    CHECK_THROWS_AS(gs2::makeString(list), gs2::GS2Exception);

    // A block cannot be converted to a string
    CHECK_THROWS_AS(gs2::makeString(gs2::Block{}), gs2::GS2Exception);
}
