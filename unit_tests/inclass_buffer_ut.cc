#include <small_vectors/inclass_buffer.hpp>

#include <boost/ut.hpp>
#include <string>
#include <type_traits>

using namespace boost::ut;
using small_vectors::inclass_store_t;

struct test_trivial_struct
  {
  int content;
  };

static_assert(std::alignment_of_v<test_trivial_struct> == alignof(test_trivial_struct), "Alignment mismatch.");
using namespace std::string_view_literals;

suite<"inclass_store_trivial_tests"> inclass_store_trivial_tests = []
{
  using inclass_string_store
    = inclass_store_t<test_trivial_struct, sizeof(test_trivial_struct), alignof(test_trivial_struct)>;
  "default_constructor"_test = []
  {
    inclass_string_store store;
    expect(eq(store->content, 0));
  };
  "parameterized_constructor"_test = []
  {
    inclass_string_store store{10};
    expect(eq(10, store->content));
  };
  "copy_constructor"_test = []
  {
    inclass_string_store const original{-10};
    inclass_string_store const copy{original};
    expect(eq(-10, copy->content));
  };
  "move_constructor"_test = []
  {
    inclass_string_store original{0xfffffe};
    inclass_string_store const moved{std::move(original)};
    expect(eq(0xfffffe, moved->content));
  };

  "copy_assignment"_test = []
  {
    inclass_string_store original{0xfffffe};
    inclass_string_store copy;
    copy = original;
    expect(eq(0xfffffe, (*copy).content));
  };

  "move_assignment"_test = []
  {
    inclass_string_store original{0xfffffe};
    inclass_string_store moved;
    moved = std::move(original);
    expect(eq(0xfffffe, (*moved).content));
  };
};
constexpr auto test_text
  = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Duis sit amet dictum neque."
    " Aliquam erat volutpat. Vivamus bibendum pretium eros, eu porta libero dictum ut."
    " Vivamus feugiat nisi elit, quis finibus risus pellentesque non. Cras accumsan felis quis dolor malesuada,"
    " eu consequat velit malesuada."sv;

struct test_non_trivial_struct
  {
  std::string content;
  };

suite<"inclass_store_non_trivial_tests"> inclass_store_non_trivial_tests = []
{
  using inclass_string_store
    = inclass_store_t<test_non_trivial_struct, sizeof(test_non_trivial_struct), alignof(test_non_trivial_struct)>;

  "default_constructor"_test = []
  {
    inclass_string_store store;
    expect(store->content.empty());
  };

  "parameterized_constructor"_test = []
  {
    inclass_string_store store{std::string{test_text}};
    expect(eq(test_text, (*store).content));
  };

  "copy_constructor"_test = []
  {
    inclass_string_store const original{std::string{test_text}};
    inclass_string_store copy{original};
    expect(eq(test_text, (*copy).content));
  };

  "move_constructor"_test = []
  {
    inclass_string_store original{std::string{test_text}};
    inclass_string_store const moved{std::move(original)};
    expect(eq(test_text, (*moved).content));
  };

  "copy_assignment"_test = []
  {
    inclass_string_store original{std::string{test_text}};
    inclass_string_store copy;
    copy = original;
    expect(eq(test_text, (*copy).content));
  };

  "move_assignment"_test = []
  {
    inclass_string_store original{std::string{test_text}};
    inclass_string_store moved;
    moved = std::move(original);
    expect(eq(test_text, (*moved).content));
  };
};

struct test_multi_arg_struct
  {
  std::string content;
  int integral;
  };

suite<"inclass_store_multi_arg_struct"> inclass_store_multi_arg_struct = []
{
  using inclass_string_store = inclass_store_t<test_multi_arg_struct>;

  "default_constructor"_test = []
  {
    inclass_string_store store;
    expect(store->content.empty());
  };

  "parameterized_constructor"_test = []
  {
    inclass_string_store store{std::string{test_text}, 0x1f55aafe};
    expect(eq(test_text, (*store).content));
    expect(eq(0x1f55aafe, store->integral));
  };
};

int main()
  {
  // Running the tests
  return boost::ut::cfg<>.run();
  }
