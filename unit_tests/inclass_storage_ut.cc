#include <small_vectors/inclass_storage.h>
#include <small_vectors/basic_string.h>
#include <small_vectors/stream/basic_string.h>

#include <unit_test_core.h>
#include <string>
#include <type_traits>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wglobal-constructors"
#pragma clang diagnostic ignored "-Wexit-time-destructors"
#endif

using namespace boost::ut;
using small_vectors::inclass_store_t;

struct test_trivial_struct
  {
  int content;
  };

static_assert(std::alignment_of_v<test_trivial_struct> == alignof(test_trivial_struct), "Alignment mismatch.");
using namespace std::string_view_literals;

static suite<"inclass_store_trivial_tests"> inclass_store_trivial_tests = []
{
  using inclass_trivial_store
    = inclass_store_t<test_trivial_struct, sizeof(test_trivial_struct), alignof(test_trivial_struct)>;
  "default_constructor"_test = []
  {
    inclass_trivial_store store;
    expect(eq(store->content, 0));
  };
  "parameterized_constructor"_test = []
  {
    inclass_trivial_store store{10};
    expect(eq(10, store->content));
  };
  "copy_constructor"_test = []
  {
    inclass_trivial_store const original{-10};
    inclass_trivial_store const copy{original};
    expect(eq(-10, copy->content));
  };
  "move_constructor"_test = []
  {
    inclass_trivial_store original{0xfffffe};
    inclass_trivial_store const moved{std::move(original)};
    expect(eq(0xfffffe, moved->content));
  };

  "copy_assignment"_test = []
  {
    inclass_trivial_store original{0xfffffe};
    inclass_trivial_store copy;
    copy = original;
    expect(eq(0xfffffe, (*copy).content));
  };

  "move_assignment"_test = []
  {
    inclass_trivial_store original{0xfffffe};
    inclass_trivial_store moved;
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

static suite<"inclass_store_non_trivial_tests"> inclass_store_non_trivial_tests = []
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

static suite<"inclass_store_multi_arg_struct"> inclass_store_multi_arg_struct = []
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
struct forward_struct_t;

struct test_pimpl
  {
  using inclass_foraward_store = small_vectors::inclass_storage_t<forward_struct_t, 48, 8>;
  inclass_foraward_store store;

  // explicit test_pimpl(inclass_foraward_store const & data);
  test_pimpl();
  explicit test_pimpl(forward_struct_t && data);
  explicit test_pimpl(std::string_view s, int i);
  test_pimpl(test_pimpl const &);
  test_pimpl(test_pimpl &&) noexcept;
  ~test_pimpl();
  test_pimpl & operator=(test_pimpl const &);
  test_pimpl & operator=(test_pimpl &&);

  auto operator->() const noexcept -> forward_struct_t const *;
  auto operator->() noexcept -> forward_struct_t *;
  };

struct forward_struct_t
  {
  small_vectors::string content;
  int integral;
  };

static_assert(sizeof(forward_struct_t) == 48);

// test_pimpl::test_pimpl(inclass_foraward_store const & data) : store{data} {}
test_pimpl::test_pimpl() : store{small_vectors::inclass_storage::default_construct<inclass_foraward_store>()} {}

test_pimpl::~test_pimpl() { small_vectors::inclass_storage::destroy(store); }

test_pimpl::test_pimpl(test_pimpl const & other) : store{small_vectors::inclass_storage::copy_construct(other.store)} {}

test_pimpl::test_pimpl(test_pimpl && other) noexcept :
    store{small_vectors::inclass_storage::move_construct(std::move(other.store))}
  {
  }

test_pimpl & test_pimpl::operator=(test_pimpl const & other)
  {
  small_vectors::inclass_storage::copy_assign(store, other.store);
  return *this;
  }

test_pimpl & test_pimpl::operator=(test_pimpl && other)
  {
  small_vectors::inclass_storage::move_assign(store, std::move(other.store));
  return *this;
  }

test_pimpl::test_pimpl(forward_struct_t && data) :
    store{small_vectors::inclass_storage::construct_from<inclass_foraward_store>(std::move(data))}
  {
  }

test_pimpl::test_pimpl(std::string_view s, int i) :
    store{small_vectors::inclass_storage::construct_from<inclass_foraward_store>(
      small_vectors::string{s}, i

    )}
  {
  }

auto test_pimpl::operator->() const noexcept -> forward_struct_t const *
  {
  return small_vectors::inclass_storage::ptr(store);
  }

auto test_pimpl::operator->() noexcept -> forward_struct_t * { return small_vectors::inclass_storage::ptr(store); }

static suite<"inclass_forward__struct"> inclass_forward_struct = []
{
  "default_constructor"_test = []
  {
    test_pimpl store;
    expect(store->content.empty());
  };
  "copy_constructor"_test = []
  {
    test_pimpl const original{test_text, 0x55aaff};
    test_pimpl copy{original};
    expect(eq(test_text, copy->content));
    expect(eq(0x55aaff, copy->integral));
  };
  "move_constructor"_test = []
  {
    test_pimpl original{test_text, 0x55aaff};
    test_pimpl const moved{std::move(original)};
    expect(eq(test_text, moved->content));
    expect(eq(0x55aaff, moved->integral));
  };
  "copy_assignment"_test = []
  {
    test_pimpl original{forward_struct_t{small_vectors::string{test_text}, 0x55aaff}};
    test_pimpl copy;
    copy = original;
    expect(eq(test_text, copy->content));
    expect(eq(0x55aaff, copy->integral));
  };
  "move_assignment"_test = []
  {
    test_pimpl original{forward_struct_t{small_vectors::string{test_text}, 0x55aaff}};
    test_pimpl moved;
    moved = std::move(original);
    expect(eq(test_text, moved->content));
    expect(eq(0x55aaff, moved->integral));
  };
};

int main()
  {
  // Running the tests
  return boost::ut::cfg<>.run();
  }
