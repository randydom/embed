#pragma once

#include "resource.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

class Saver {
private:
	fs::path root{};
	std::vector<std::string> filenames;
	std::ofstream resource_holder_hpp;
	std::ofstream resource_hpp;
	std::ofstream span_hpp;
	bool verbose = true;

	const std::string subfolder_name = "embedded_resources";

	constexpr static std::array resource_holder_begin_text{
		"namespace {",
		"class ResourceHolder {",
		"private:",
	};
	constexpr static std::array resource_holder_text {
		"",
		"public:",
		"\tauto Gather(const std::string& file) const {",
		"\t\tauto it = std::find_if(resources.begin(), resources.end(), [file](const auto& lhs) {",
		"\t\t\treturn lhs.GetPath() == file;",
		"\t\t});",
		"\t\tif (it == resources.end())",
		"\t\t\tthrow std::runtime_error(\"Unable to detect resource with name \" + file);",
		"\t\t",
		"\t\treturn it->GetArray();",
		"\t}",
		"",
		"\tauto Gather(const char* file) const {",
		"\t\treturn Gather(std::string(file));",
		"\t}",
		"",
		"\tauto ListFiles() const {",
		"\t\tstd::vector<std::string> dst{};",
		"\t\tdst.reserve(resources.size());",
		"\t\tfor (auto&el : resources)",
		"\t\t\tdst.push_back(el.GetPath());",
		"",
		"\t\treturn dst;",
		"\t}",
		"",
		"\tauto FindByFilename(const std::string& file) const {",
		"\t\tstd::vector<Resource> dst{};",
		"\t\tdst.reserve(resources.size());",
		"\t\tstd::copy_if(resources.begin(), resources.end(), std::back_inserter(dst), [file](const auto& item) {",
		"\t\t\tauto path = item.GetPath();",
		"\t\t\tauto last_forward = path.find_last_of('\\\\');",
		"\t\t\tauto last_inverse = path.find_last_of('/');",
		"\t\t\t",
		"\t\t\tif (last_forward != std::string::npos)",
		"\t\t\t\tpath = path.substr(last_forward + 1);",
		"\t\t\telse if (last_inverse != std::string::npos)",
		"\t\t\t\tpath = path.substr(last_inverse + 1);",
		"\t\t\treturn path == file;",
		"\t\t});",
		"\t\t",
		"\t\treturn dst;",
		"\t}",
		"",
		"\tauto FindByFilename(const char* file) const {",
		"\t\treturn FindByFilename(std::string(file));",
		"\t}",
		"",
		"\tauto operator()(const std::string& file) const {",
		"\t\treturn Gather(file);",
		"\t}",
		"",
		"\tauto operator()(const char* file) const {",
		"\t\treturn Gather(std::string(file));",
		"\t}",
		"};",
		"}",
	};
	constexpr static std::array resource_text{
		"#pragma once",
		"",
		"#include <algorithm>",
		"#include <array>",
		"#include <cstdint>",
		"#include <functional>",
		"#include \"span.hpp\"",
		"#include <tuple>",
		"#include <vector>",
		"",
		"class Resource {",
		"public:",
		"\ttemplate <typename T>",
		"\tusing span = tcb::span<T>;",
		"\tusing EmbeddedData = span<const std::uint8_t>;",
		"",
		"private:",
		"\tconst EmbeddedData arr_view;",
		"\tconst std::string path;",
		"",
		"public:",
		"\tResource() = default;",
		"\ttemplate <typename Container>",
		"\tResource(const Container& arr_, std::string path_) : arr_view(arr_), path(std::move(path_)) {}",
		"",
		"\tauto GetArray() const {",
		"\t\treturn arr_view;",
		"\t}",
		"",
		"\tauto& GetPath() const {",
		"\t\treturn path;",
		"\t}",
		"};",
	};
	constexpr static std::array span_text{
		"/*",
		"This is an implementation of std::span from P0122R7",
		"http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0122r7.pdf",
		"*/",
		"",
		"//\t\t  Copyright Tristan Brindle 2018.",
		"// Distributed under the Boost Software License, Version 1.0.",
		"//\t(See accompanying file ../../LICENSE_1_0.txt or copy at",
		"//\t\t  https://www.boost.org/LICENSE_1_0.txt)",
		"",
		"#ifndef TCB_SPAN_HPP_INCLUDED",
		"#define TCB_SPAN_HPP_INCLUDED",
		"",
		"#include <array>",
		"#include <cstddef>",
		"#include <type_traits>",
		"",
		"#ifndef TCB_SPAN_NO_EXCEPTIONS",
		"// Attempt to discover whether we're being compiled with exception support",
		"#if !(defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND))",
		"#define TCB_SPAN_NO_EXCEPTIONS",
		"#endif",
		"#endif",
		"",
		"#ifndef TCB_SPAN_NO_EXCEPTIONS",
		"#include <cstdio>",
		"#include <stdexcept>",
		"#endif",
		"",
		"// Various feature test macros",
		"",
		"#ifndef TCB_SPAN_NAMESPACE_NAME",
		"#define TCB_SPAN_NAMESPACE_NAME tcb",
		"#endif",
		"",
		"#ifdef TCB_SPAN_STD_COMPLIANT_MODE",
		"#define TCB_SPAN_NO_DEPRECATION_WARNINGS",
		"#endif",
		"",
		"#ifndef TCB_SPAN_NO_DEPRECATION_WARNINGS",
		"#define TCB_SPAN_DEPRECATED_FOR(msg) [[deprecated(msg)]]",
		"#else",
		"#define TCB_SPAN_DEPRECATED_FOR(msg)",
		"#endif",
		"",
		"#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)",
		"#define TCB_SPAN_HAVE_CPP17",
		"#endif",
		"",
		"#if __cplusplus >= 201402L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L)",
		"#define TCB_SPAN_HAVE_CPP14",
		"#endif",
		"",
		"namespace TCB_SPAN_NAMESPACE_NAME {",
		"",
		"// Establish default contract checking behavior",
		"#if !defined(TCB_SPAN_THROW_ON_CONTRACT_VIOLATION) &&\t\t\t\t\t\t  \\",
		"\t!defined(TCB_SPAN_TERMINATE_ON_CONTRACT_VIOLATION) &&\t\t\t\t\t  \\",
		"\t!defined(TCB_SPAN_NO_CONTRACT_CHECKING)",
		"#if defined(NDEBUG) || !defined(TCB_SPAN_HAVE_CPP14)",
		"#define TCB_SPAN_NO_CONTRACT_CHECKING",
		"#else",
		"#define TCB_SPAN_TERMINATE_ON_CONTRACT_VIOLATION",
		"#endif",
		"#endif",
		"",
		"#if defined(TCB_SPAN_THROW_ON_CONTRACT_VIOLATION)",
		"struct contract_violation_error : std::logic_error {",
		"\texplicit contract_violation_error(const char* msg) : std::logic_error(msg)",
		"\t{}",
		"};",
		"",
		"inline void contract_violation(const char* msg)",
		"{",
		"\tthrow contract_violation_error(msg);",
		"}",
		"",
		"#elif defined(TCB_SPAN_TERMINATE_ON_CONTRACT_VIOLATION)",
		"[[noreturn]] inline void contract_violation(const char* /*unused*/)",
		"{",
		"\tstd::terminate();",
		"}",
		"#endif",
		"",
		"#if !defined(TCB_SPAN_NO_CONTRACT_CHECKING)",
		"#define TCB_SPAN_STRINGIFY(cond) #cond",
		"#define TCB_SPAN_EXPECT(cond)\t\t\t\t\t\t\t\t\t\t\t\t  \\",
		"\tcond ? (void) 0 : contract_violation(\"Expected \" TCB_SPAN_STRINGIFY(cond))",
		"#else",
		"#define TCB_SPAN_EXPECT(cond)",
		"#endif",
		"",
		"#if defined(TCB_SPAN_HAVE_CPP17) || defined(__cpp_inline_variables)",
		"#define TCB_SPAN_INLINE_VAR inline",
		"#else",
		"#define TCB_SPAN_INLINE_VAR",
		"#endif",
		"",
		"#if defined(TCB_SPAN_HAVE_CPP14) ||\t\t\t\t\t\t\t\t\t\t\t\t \\",
		"\t(defined(__cpp_constexpr) && __cpp_constexpr >= 201304)",
		"#define TCB_SPAN_CONSTEXPR14 constexpr",
		"#else",
		"#define TCB_SPAN_CONSTEXPR14",
		"#endif",
		"",
		"#if defined(TCB_SPAN_NO_CONTRACT_CHECKING)",
		"#define TCB_SPAN_CONSTEXPR11 constexpr",
		"#else",
		"#define TCB_SPAN_CONSTEXPR11 TCB_SPAN_CONSTEXPR14",
		"#endif",
		"",
		"#if defined(TCB_SPAN_HAVE_CPP17) || defined(__cpp_deduction_guides)",
		"#define TCB_SPAN_HAVE_DEDUCTION_GUIDES",
		"#endif",
		"",
		"#if defined(TCB_SPAN_HAVE_CPP17) || defined(__cpp_lib_byte)",
		"#define TCB_SPAN_HAVE_STD_BYTE",
		"#endif",
		"",
		"#if defined(TCB_SPAN_HAVE_CPP17) || defined(__cpp_lib_array_constexpr)",
		"#define TCB_SPAN_HAVE_CONSTEXPR_STD_ARRAY_ETC",
		"#endif",
		"",
		"#if defined(TCB_SPAN_HAVE_CONSTEXPR_STD_ARRAY_ETC)",
		"#define TCB_SPAN_ARRAY_CONSTEXPR constexpr",
		"#else",
		"#define TCB_SPAN_ARRAY_CONSTEXPR",
		"#endif",
		"",
		"#ifdef TCB_SPAN_HAVE_STD_BYTE",
		"using byte = std::byte;",
		"#else",
		"using byte = unsigned char;",
		"#endif",
		"",
		"TCB_SPAN_INLINE_VAR constexpr std::ptrdiff_t dynamic_extent = -1;",
		"",
		"template <typename ElementType, std::ptrdiff_t Extent = dynamic_extent>",
		"class span;",
		"",
		"namespace detail {",
		"",
		"template <typename E, std::ptrdiff_t S>",
		"struct span_storage {",
		"\tconstexpr span_storage() noexcept = default;",
		"",
		"\tconstexpr span_storage(E* ptr, std::ptrdiff_t /*unused*/) noexcept",
		"\t\t: ptr(ptr)",
		"\t{}",
		"",
		"\tE* ptr = nullptr;",
		"\tstatic constexpr std::ptrdiff_t size = S;",
		"};",
		"",
		"template <typename E>",
		"struct span_storage<E, dynamic_extent> {",
		"\tconstexpr span_storage() noexcept = default;",
		"",
		"\tconstexpr span_storage(E* ptr, std::ptrdiff_t size) noexcept",
		"\t\t: ptr(ptr), size(size)",
		"\t{}",
		"",
		"\tE* ptr = nullptr;",
		"\tstd::ptrdiff_t size = 0;",
		"};",
		"",
		"// Reimplementation of C++17 std::size() and std::data()",
		"#if defined(TCB_SPAN_HAVE_CPP17) ||\t\t\t\t\t\t\t\t\t\t\t\\",
		"\tdefined(__cpp_lib_nonmember_container_access)",
		"using std::data;",
		"using std::size;",
		"#else",
		"template <class C>",
		"constexpr auto size(const C& c) -> decltype(c.size())",
		"{",
		"\treturn c.size();",
		"}",
		"",
		"template <class T, std::size_t N>",
		"constexpr std::size_t size(const T (&)[N]) noexcept",
		"{",
		"\treturn N;",
		"}",
		"",
		"template <class C>",
		"constexpr auto data(C& c) -> decltype(c.data())",
		"{",
		"\treturn c.data();",
		"}",
		"",
		"template <class C>",
		"constexpr auto data(const C& c) -> decltype(c.data())",
		"{",
		"\treturn c.data();",
		"}",
		"",
		"template <class T, std::size_t N>",
		"constexpr T* data(T (&array)[N]) noexcept",
		"{",
		"\treturn array;",
		"}",
		"",
		"template <class E>",
		"constexpr const E* data(std::initializer_list<E> il) noexcept",
		"{",
		"\treturn il.begin();",
		"}",
		"#endif // TCB_SPAN_HAVE_CPP17",
		"",
		"#if defined(TCB_SPAN_HAVE_CPP17) || defined(__cpp_lib_void_t)",
		"using std::void_t;",
		"#else",
		"template <typename...>",
		"using void_t = void;",
		"#endif",
		"",
		"template <typename T>",
		"using uncvref_t =",
		"\ttypename std::remove_cv<typename std::remove_reference<T>::type>::type;",
		"",
		"template <typename>",
		"struct is_span : std::false_type {};",
		"",
		"template <typename T, std::ptrdiff_t S>",
		"struct is_span<span<T, S>> : std::true_type {};",
		"",
		"template <typename>",
		"struct is_std_array : std::false_type {};",
		"",
		"template <typename T, std::size_t N>",
		"struct is_std_array<std::array<T, N>> : std::true_type {};",
		"",
		"template <typename, typename = void>",
		"struct has_size_and_data : std::false_type {};",
		"",
		"template <typename T>",
		"struct has_size_and_data<T, void_t<decltype(detail::size(std::declval<T>())),",
		"\t\t\t\t\t\t\t\t   decltype(detail::data(std::declval<T>()))>>",
		"\t: std::true_type {};",
		"",
		"template <typename C, typename U = uncvref_t<C>>",
		"struct is_container {",
		"\tstatic constexpr bool value =",
		"\t\t!is_span<U>::value && !is_std_array<U>::value &&",
		"\t\t!std::is_array<U>::value && has_size_and_data<C>::value;",
		"};",
		"",
		"template <typename T>",
		"using remove_pointer_t = typename std::remove_pointer<T>::type;",
		"",
		"template <typename, typename, typename = void>",
		"struct is_container_element_type_compatible : std::false_type {};",
		"",
		"template <typename T, typename E>",
		"struct is_container_element_type_compatible<",
		"\tT, E, void_t<decltype(detail::data(std::declval<T>()))>>",
		"\t: std::is_convertible<",
		"\t\t  remove_pointer_t<decltype(detail::data(std::declval<T>()))> (*)[],",
		"\t\t  E (*)[]> {};",
		"",
		"template <typename, typename = size_t>",
		"struct is_complete : std::false_type {};",
		"",
		"template <typename T>",
		"struct is_complete<T, decltype(sizeof(T))> : std::true_type {};",
		"",
		"} // namespace detail",
		"",
		"template <typename ElementType, std::ptrdiff_t Extent>",
		"class span {",
		"\tstatic_assert(Extent == dynamic_extent || Extent >= 0,",
		"\t\t\t\t  \"A span must have an extent greater than or equal to zero, \"",
		"\t\t\t\t  \" or a dynamic extent\");",
		"\tstatic_assert(std::is_object<ElementType>::value,",
		"\t\t\t\t  \"A span's ElementType must be an object type (not a \"",
		"\t\t\t\t  \"reference type or void)\");",
		"\tstatic_assert(detail::is_complete<ElementType>::value,",
		"\t\t\t\t  \"A span's ElementType must be a complete type (not a forward \"",
		"\t\t\t\t  \"declaration)\");",
		"\tstatic_assert(!std::is_abstract<ElementType>::value,",
		"\t\t\t\t  \"A span's ElementType cannot be an abstract class type\");",
		"",
		"\tusing storage_type = detail::span_storage<ElementType, Extent>;",
		"",
		"public:",
		"\t// constants and types",
		"\tusing element_type = ElementType;",
		"\tusing value_type = typename std::remove_cv<ElementType>::type;",
		"\tusing index_type = std::ptrdiff_t;",
		"\tusing difference_type = std::ptrdiff_t;",
		"\tusing pointer = ElementType*;",
		"\tusing reference = ElementType&;",
		"\tusing iterator = pointer;",
		"\tusing const_iterator = const ElementType*;",
		"\tusing reverse_iterator = std::reverse_iterator<iterator>;",
		"\tusing const_reverse_iterator = std::reverse_iterator<const_iterator>;",
		"",
		"\tstatic constexpr index_type extent = Extent;",
		"",
		"\t// [span.cons], span constructors, copy, assignment, and destructor",
		"\ttemplate <std::ptrdiff_t E = Extent,",
		"\t\t\t  typename std::enable_if<E <= 0, int>::type = 0>",
		"\tconstexpr span() noexcept",
		"\t{}",
		"",
		"\tTCB_SPAN_CONSTEXPR11 span(pointer ptr, index_type count)",
		"\t\t: storage_(ptr, count)",
		"\t{",
		"\t\tTCB_SPAN_EXPECT(extent == dynamic_extent || count == extent);",
		"\t}",
		"",
		"\tTCB_SPAN_CONSTEXPR11 span(pointer first_elem, pointer last_elem)",
		"\t\t: storage_(first_elem, last_elem - first_elem)",
		"\t{",
		"\t\tTCB_SPAN_EXPECT(extent == dynamic_extent ||",
		"\t\t\t\t\t\tlast_elem - first_elem == extent);",
		"\t}",
		"",
		"\ttemplate <",
		"\t\tstd::size_t N, std::ptrdiff_t E = Extent,",
		"\t\ttypename std::enable_if<",
		"\t\t\t(E == dynamic_extent || static_cast<std::ptrdiff_t>(N) == E) &&",
		"\t\t\t\tdetail::is_container_element_type_compatible<",
		"\t\t\t\t\telement_type (&)[N], ElementType>::value,",
		"\t\t\tint>::type = 0>",
		"\tconstexpr span(element_type (&arr)[N]) noexcept : storage_(arr, N)",
		"\t{}",
		"",
		"\ttemplate <",
		"\t\tstd::size_t N, std::ptrdiff_t E = Extent,",
		"\t\ttypename std::enable_if<",
		"\t\t\t(E == dynamic_extent || static_cast<std::ptrdiff_t>(N) == E) &&",
		"\t\t\t\tdetail::is_container_element_type_compatible<",
		"\t\t\t\t\tstd::array<value_type, N>&, ElementType>::value,",
		"\t\t\tint>::type = 0>",
		"\tTCB_SPAN_ARRAY_CONSTEXPR span(std::array<value_type, N>& arr) noexcept",
		"\t\t: storage_(arr.data(), N)",
		"\t{}",
		"",
		"\ttemplate <",
		"\t\tstd::size_t N, std::ptrdiff_t E = Extent,",
		"\t\ttypename std::enable_if<",
		"\t\t\t(E == dynamic_extent || static_cast<std::ptrdiff_t>(N) == E) &&",
		"\t\t\t\tdetail::is_container_element_type_compatible<",
		"\t\t\t\t\tconst std::array<value_type, N>&, ElementType>::value,",
		"\t\t\tint>::type = 0>",
		"\tTCB_SPAN_ARRAY_CONSTEXPR span(const std::array<value_type, N>& arr) noexcept",
		"\t\t: storage_(arr.data(), N)",
		"\t{}",
		"",
		"\ttemplate <typename Container,",
		"\t\t\t  typename std::enable_if<",
		"\t\t\t\t  detail::is_container<Container>::value &&",
		"\t\t\t\t\t  detail::is_container_element_type_compatible<",
		"\t\t\t\t\t\t  Container&, ElementType>::value,",
		"\t\t\t\t  int>::type = 0>",
		"\tTCB_SPAN_CONSTEXPR11 span(Container& cont)",
		"\t\t: storage_(detail::data(cont), detail::size(cont))",
		"\t{",
		"\t\tTCB_SPAN_EXPECT(extent == dynamic_extent ||",
		"\t\t\t\t\t\tstatic_cast<std::ptrdiff_t>(detail::size(cont)) ==",
		"\t\t\t\t\t\t\textent);",
		"\t}",
		"",
		"\ttemplate <typename Container,",
		"\t\t\t  typename std::enable_if<",
		"\t\t\t\t  detail::is_container<Container>::value &&",
		"\t\t\t\t\t  detail::is_container_element_type_compatible<",
		"\t\t\t\t\t\t  const Container&, ElementType>::value,",
		"\t\t\t\t  int>::type = 0>",
		"\tTCB_SPAN_CONSTEXPR11 span(const Container& cont)",
		"\t\t: storage_(detail::data(cont), detail::size(cont))",
		"\t{",
		"\t\tTCB_SPAN_EXPECT(extent == dynamic_extent ||",
		"\t\t\t\t\t\tstatic_cast<std::ptrdiff_t>(detail::size(cont)) ==",
		"\t\t\t\t\t\t\textent);",
		"\t}",
		"",
		"\tconstexpr span(const span& other) noexcept = default;",
		"",
		"\ttemplate <typename OtherElementType, std::ptrdiff_t OtherExtent,",
		"\t\t\t  typename std::enable_if<",
		"\t\t\t\t  (Extent == OtherExtent || Extent == dynamic_extent) &&",
		"\t\t\t\t\t  std::is_convertible<OtherElementType (*)[],",
		"\t\t\t\t\t\t\t\t\t\t  ElementType (*)[]>::value,",
		"\t\t\t\t  int>::type = 0>",
		"\tconstexpr span(const span<OtherElementType, OtherExtent>& other) noexcept",
		"\t\t: storage_(other.data(), other.size())",
		"\t{}",
		"",
		"\t~span() noexcept = default;",
		"",
		"\tTCB_SPAN_CONSTEXPR14 span& operator=(const span& other) noexcept = default;",
		"",
		"\t// [span.sub], span subviews",
		"\ttemplate <std::ptrdiff_t Count>",
		"\tTCB_SPAN_CONSTEXPR11 span<element_type, Count> first() const",
		"\t{",
		"\t\tTCB_SPAN_EXPECT(Count >= 0 && Count <= size());",
		"\t\treturn {data(), Count};",
		"\t}",
		"",
		"\ttemplate <std::ptrdiff_t Count>",
		"\tTCB_SPAN_CONSTEXPR11 span<element_type, Count> last() const",
		"\t{",
		"\t\tTCB_SPAN_EXPECT(Count >= 0 && Count <= size());",
		"\t\treturn {data() + (size() - Count), Count};",
		"\t}",
		"",
		"\ttemplate <std::ptrdiff_t Offset, std::ptrdiff_t Count = dynamic_extent>",
		"\tusing subspan_return_t =",
		"\t\tspan<ElementType, Count != dynamic_extent",
		"\t\t\t\t\t\t\t  ? Count",
		"\t\t\t\t\t\t\t  : (Extent != dynamic_extent ? Extent - Offset",
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t  : dynamic_extent)>;",
		"",
		"\ttemplate <std::ptrdiff_t Offset, std::ptrdiff_t Count = dynamic_extent>",
		"\tTCB_SPAN_CONSTEXPR11 subspan_return_t<Offset, Count> subspan() const",
		"\t{",
		"\t\tTCB_SPAN_EXPECT((Offset >= 0 && Offset <= size()) &&",
		"\t\t\t\t\t\t(Count == dynamic_extent ||",
		"\t\t\t\t\t\t (Count >= 0 && Offset + Count <= size())));",
		"\t\treturn {data() + Offset,",
		"\t\t\t\tCount != dynamic_extent",
		"\t\t\t\t\t? Count",
		"\t\t\t\t\t: (Extent != dynamic_extent ? Extent - Offset",
		"\t\t\t\t\t\t\t\t\t\t\t\t: size() - Offset)};",
		"\t}",
		"",
		"\tTCB_SPAN_CONSTEXPR11 span<element_type, dynamic_extent>",
		"\tfirst(index_type count) const",
		"\t{",
		"\t\tTCB_SPAN_EXPECT(count >= 0 && count <= size());",
		"\t\treturn {data(), count};",
		"\t}",
		"",
		"\tTCB_SPAN_CONSTEXPR11 span<element_type, dynamic_extent>",
		"\tlast(index_type count) const",
		"\t{",
		"\t\tTCB_SPAN_EXPECT(count >= 0 && count <= size());",
		"\t\treturn {data() + (size() - count), count};",
		"\t}",
		"",
		"\tTCB_SPAN_CONSTEXPR11 span<element_type, dynamic_extent>",
		"\tsubspan(index_type offset, index_type count = dynamic_extent) const",
		"\t{",
		"\t\tTCB_SPAN_EXPECT((offset >= 0 && offset <= size()) &&",
		"\t\t\t\t\t\t(count == dynamic_extent ||",
		"\t\t\t\t\t\t (count >= 0 && offset + count <= size())));",
		"\t\treturn {data() + offset,",
		"\t\t\t\tcount == dynamic_extent ? size() - offset : count};",
		"\t}",
		"",
		"\t// [span.obs], span observers",
		"\tconstexpr index_type size() const noexcept { return storage_.size; }",
		"",
		"\tconstexpr index_type size_bytes() const noexcept",
		"\t{",
		"\t\treturn size() * sizeof(element_type);",
		"\t}",
		"",
		"\tconstexpr bool empty() const noexcept { return size() == 0; }",
		"",
		"\t// [span.elem], span element access",
		"\tTCB_SPAN_CONSTEXPR11 reference operator[](index_type idx) const",
		"\t{",
		"\t\tTCB_SPAN_EXPECT(idx >= 0 && idx < size());",
		"\t\treturn *(data() + idx);",
		"\t}",
		"",
		"\t/* Extension: not in P0122 */",
		"#ifndef TCB_SPAN_STD_COMPLIANT_MODE",
		"\tTCB_SPAN_CONSTEXPR14 reference at(index_type idx) const",
		"\t{",
		"#ifndef TCB_SPAN_NO_EXCEPTIONS",
		"\t\tif (idx < 0 || idx >= size()) {",
		"\t\t\tchar msgbuf[64] = {",
		"\t\t\t\t0,",
		"\t\t\t};",
		"\t\t\tstd::snprintf(msgbuf, sizeof(msgbuf),",
		"\t\t\t\t\t\t  \"Index %td is out of range for span of size %td\", idx,",
		"\t\t\t\t\t\t  size());",
		"\t\t\tthrow std::out_of_range{msgbuf};",
		"\t\t}",
		"#endif // TCB_SPAN_NO_EXCEPTIONS",
		"\t\treturn this->operator[](idx);",
		"\t}",
		"",
		"\tTCB_SPAN_CONSTEXPR11 reference front() const",
		"\t{",
		"\t\tTCB_SPAN_EXPECT(!empty());",
		"\t\treturn *data();",
		"\t}",
		"",
		"\tTCB_SPAN_CONSTEXPR11 reference back() const",
		"\t{",
		"\t\tTCB_SPAN_EXPECT(!empty());",
		"\t\treturn *(data() + (size() - 1));",
		"\t}",
		"",
		"#endif // TCB_SPAN_STD_COMPLIANT_MODE",
		"",
		"#ifndef TCB_SPAN_NO_FUNCTION_CALL_OPERATOR",
		"\tTCB_SPAN_DEPRECATED_FOR(\"Use operator[] instead\")",
		"\tconstexpr reference operator()(index_type idx) const",
		"\t{",
		"\t\treturn this->operator[](idx);",
		"\t}",
		"#endif // TCB_SPAN_NO_FUNCTION_CALL_OPERATOR",
		"",
		"\tconstexpr pointer data() const noexcept { return storage_.ptr; }",
		"",
		"\t// [span.iterators], span iterator support",
		"\tconstexpr iterator begin() const noexcept { return data(); }",
		"",
		"\tconstexpr iterator end() const noexcept { return data() + size(); }",
		"",
		"\tconstexpr const_iterator cbegin() const noexcept { return begin(); }",
		"",
		"\tconstexpr const_iterator cend() const noexcept { return end(); }",
		"",
		"\tTCB_SPAN_ARRAY_CONSTEXPR reverse_iterator rbegin() const noexcept",
		"\t{",
		"\t\treturn reverse_iterator(end());",
		"\t}",
		"",
		"\tTCB_SPAN_ARRAY_CONSTEXPR reverse_iterator rend() const noexcept",
		"\t{",
		"\t\treturn reverse_iterator(begin());",
		"\t}",
		"",
		"\tTCB_SPAN_ARRAY_CONSTEXPR const_reverse_iterator crbegin() const noexcept",
		"\t{",
		"\t\treturn const_reverse_iterator(cend());",
		"\t}",
		"",
		"\tTCB_SPAN_ARRAY_CONSTEXPR const_reverse_iterator crend() const noexcept",
		"\t{",
		"\t\treturn const_reverse_iterator(cbegin());",
		"\t}",
		"",
		"private:",
		"\tstorage_type storage_{};",
		"};",
		"",
		"#ifdef TCB_SPAN_HAVE_DEDUCTION_GUIDES",
		"",
		"/* Deduction Guides */",
		"template <class T, size_t N>",
		"span(T (&)[N])->span<T, N>;",
		"",
		"template <class T, size_t N>",
		"span(std::array<T, N>&)->span<T, N>;",
		"",
		"template <class T, size_t N>",
		"span(const std::array<T, N>&)->span<const T, N>;",
		"",
		"template <class Container>",
		"span(Container&)->span<typename Container::value_type>;",
		"",
		"template <class Container>",
		"span(const Container&)->span<const typename Container::value_type>;",
		"",
		"#endif // TCB_HAVE_DEDUCTION_GUIDES",
		"",
		"template <typename ElementType, std::ptrdiff_t Extent>",
		"constexpr span<ElementType, Extent>",
		"make_span(span<ElementType, Extent> s) noexcept",
		"{",
		"\treturn s;",
		"}",
		"",
		"template <typename T, std::size_t N>",
		"constexpr span<T, N> make_span(T (&arr)[N]) noexcept",
		"{",
		"\treturn {arr};",
		"}",
		"",
		"template <typename T, std::size_t N>",
		"TCB_SPAN_ARRAY_CONSTEXPR span<T, N> make_span(std::array<T, N>& arr) noexcept",
		"{",
		"\treturn {arr};",
		"}",
		"",
		"template <typename T, std::size_t N>",
		"TCB_SPAN_ARRAY_CONSTEXPR span<const T, N>",
		"make_span(const std::array<T, N>& arr) noexcept",
		"{",
		"\treturn {arr};",
		"}",
		"",
		"template <typename Container>",
		"constexpr span<typename Container::value_type> make_span(Container& cont)",
		"{",
		"\treturn {cont};",
		"}",
		"",
		"template <typename Container>",
		"constexpr span<const typename Container::value_type>",
		"make_span(const Container& cont)",
		"{",
		"\treturn {cont};",
		"}",
		"",
		"/* Comparison operators */",
		"// Implementation note: the implementations of == and < are equivalent to",
		"// 4-legged std::equal and std::lexicographical_compare respectively",
		"",
		"template <typename T, std::ptrdiff_t X, typename U, std::ptrdiff_t Y>",
		"TCB_SPAN_CONSTEXPR14 bool operator==(span<T, X> lhs, span<U, Y> rhs)",
		"{",
		"\tif (lhs.size() != rhs.size()) {",
		"\t\treturn false;",
		"\t}",
		"",
		"\tfor (std::ptrdiff_t i = 0; i < lhs.size(); i++) {",
		"\t\tif (lhs[i] != rhs[i]) {",
		"\t\t\treturn false;",
		"\t\t}",
		"\t}",
		"",
		"\treturn true;",
		"}",
		"",
		"template <typename T, std::ptrdiff_t X, typename U, std::ptrdiff_t Y>",
		"TCB_SPAN_CONSTEXPR14 bool operator!=(span<T, X> lhs, span<U, Y> rhs)",
		"{",
		"\treturn !(lhs == rhs);",
		"}",
		"",
		"template <typename T, std::ptrdiff_t X, typename U, std::ptrdiff_t Y>",
		"TCB_SPAN_CONSTEXPR14 bool operator<(span<T, X> lhs, span<U, Y> rhs)",
		"{",
		"\t// No std::min to avoid dragging in <algorithm>",
		"\tconst std::ptrdiff_t size =",
		"\t\tlhs.size() < rhs.size() ? lhs.size() : rhs.size();",
		"",
		"\tfor (std::ptrdiff_t i = 0; i < size; i++) {",
		"\t\tif (lhs[i] < rhs[i]) {",
		"\t\t\treturn true;",
		"\t\t}",
		"\t\tif (lhs[i] > rhs[i]) {",
		"\t\t\treturn false;",
		"\t\t}",
		"\t}",
		"\treturn lhs.size() < rhs.size();",
		"}",
		"",
		"template <typename T, std::ptrdiff_t X, typename U, std::ptrdiff_t Y>",
		"TCB_SPAN_CONSTEXPR14 bool operator<=(span<T, X> lhs, span<U, Y> rhs)",
		"{",
		"\treturn !(rhs < lhs);",
		"}",
		"",
		"template <typename T, std::ptrdiff_t X, typename U, std::ptrdiff_t Y>",
		"TCB_SPAN_CONSTEXPR14 bool operator>(span<T, X> lhs, span<U, Y> rhs)",
		"{",
		"\treturn rhs < lhs;",
		"}",
		"",
		"template <typename T, std::ptrdiff_t X, typename U, std::ptrdiff_t Y>",
		"TCB_SPAN_CONSTEXPR14 bool operator>=(span<T, X> lhs, span<U, Y> rhs)",
		"{",
		"\treturn !(lhs < rhs);",
		"}",
		"",
		"template <typename ElementType, std::ptrdiff_t Extent>",
		"span<const byte, ((Extent == dynamic_extent)",
		"\t\t\t\t\t  ? dynamic_extent",
		"\t\t\t\t\t  : (static_cast<ptrdiff_t>(sizeof(ElementType)) * Extent))>",
		"as_bytes(span<ElementType, Extent> s) noexcept",
		"{",
		"\treturn {reinterpret_cast<const byte*>(s.data()), s.size_bytes()};",
		"}",
		"",
		"template <",
		"\tclass ElementType, ptrdiff_t Extent,",
		"\ttypename std::enable_if<!std::is_const<ElementType>::value, int>::type = 0>",
		"span<byte, ((Extent == dynamic_extent)",
		"\t\t\t\t? dynamic_extent",
		"\t\t\t\t: (static_cast<ptrdiff_t>(sizeof(ElementType)) * Extent))>",
		"as_writable_bytes(span<ElementType, Extent> s) noexcept",
		"{",
		"\treturn {reinterpret_cast<byte*>(s.data()), s.size_bytes()};",
		"}",
		"",
		"/* Extension: nonmember subview operations */",
		"",
		"#ifndef TCB_SPAN_STD_COMPLIANT_MODE",
		"",
		"template <std::ptrdiff_t Count, typename T>",
		"TCB_SPAN_CONSTEXPR11 auto first(T& t)",
		"\t-> decltype(make_span(t).template first<Count>())",
		"{",
		"\treturn make_span(t).template first<Count>();",
		"}",
		"",
		"template <std::ptrdiff_t Count, typename T>",
		"TCB_SPAN_CONSTEXPR11 auto last(T& t)",
		"\t-> decltype(make_span(t).template last<Count>())",
		"{",
		"\treturn make_span(t).template last<Count>();",
		"}",
		"",
		"template <std::ptrdiff_t Offset, std::ptrdiff_t Count = dynamic_extent,",
		"\t\t  typename T>",
		"TCB_SPAN_CONSTEXPR11 auto subspan(T& t)",
		"\t-> decltype(make_span(t).template subspan<Offset, Count>())",
		"{",
		"\treturn make_span(t).template subspan<Offset, Count>();",
		"}",
		"",
		"template <typename T>",
		"TCB_SPAN_CONSTEXPR11 auto first(T& t, std::ptrdiff_t count)",
		"\t-> decltype(make_span(t).first(count))",
		"{",
		"\treturn make_span(t).first(count);",
		"}",
		"",
		"template <typename T>",
		"TCB_SPAN_CONSTEXPR11 auto last(T& t, std::ptrdiff_t count)",
		"\t-> decltype(make_span(t).last(count))",
		"{",
		"\treturn make_span(t).last(count);",
		"}",
		"",
		"template <typename T>",
		"TCB_SPAN_CONSTEXPR11 auto subspan(T& t, std::ptrdiff_t offset,",
		"\t\t\t\t\t\t\t\t  std::ptrdiff_t count = dynamic_extent)",
		"\t-> decltype(make_span(t).subspan(offset, count))",
		"{",
		"\treturn make_span(t).subspan(offset, count);",
		"}",
		"",
		"#endif // TCB_SPAN_STD_COMPLIANT_MODE",
		"",
		"} // namespace TCB_SPAN_NAMESPACE_NAME",
		"",
		"/* Extension: support for C++17 structured bindings */",
		"",
		"#ifndef TCB_SPAN_STD_COMPLIANT_MODE",
		"",
		"namespace TCB_SPAN_NAMESPACE_NAME {",
		"",
		"template <std::ptrdiff_t N, typename E, std::ptrdiff_t S>",
		"constexpr auto get(span<E, S> s) -> decltype(s[N])",
		"{",
		"\treturn s[N];",
		"}",
		"",
		"} // namespace TCB_SPAN_NAMESPACE_NAME",
		"",
		"namespace std {",
		"",
		"template <typename E, ptrdiff_t S>",
		"class tuple_size<tcb::span<E, S>> : public integral_constant<size_t, S> {};",
		"",
		"template <typename E>",
		"class tuple_size<tcb::span<E, tcb::dynamic_extent>>; // not defined",
		"",
		"template <size_t N, typename E, ptrdiff_t S>",
		"class tuple_element<N, tcb::span<E, S>> {",
		"public:",
		"\tusing type = E;",
		"};",
		"",
		"} // end namespace std",
		"",
		"#endif // TCB_SPAN_STD_COMPLIANT_MODE",
		"",
		"#endif // TCB_SPAN_HPP_INCLUDED",
	};

	auto Format(int val) const {
		static std::array<char, 5> str;
		auto [p, ec] = std::to_chars(str.data(), str.data() + str.size(), val);
		*p++ = ',';
		return std::string(str.data(), p - str.data());
	}

	auto FromFilename(const std::string& filename) {
		return fs::path(root).append(filename).string();
	}

	bool IsSame(const std::stringstream& data, fs::path resource_path) const {
		std::ifstream test_file(resource_path);
		if (!test_file)
			return false;

		return std::string(std::istreambuf_iterator<char>(test_file), std::istreambuf_iterator<char>()) == data.str();
	}

public:
	Saver(fs::path root_path) :
		root(root_path), 
		resource_holder_hpp(FromFilename("resource_holder.hpp")),
		resource_hpp(FromFilename("resource.hpp")),
		span_hpp(FromFilename("span.hpp")) {

		if (!resource_holder_hpp.is_open())
			throw std::runtime_error("Unable to create helper header");
		if (!resource_hpp.is_open())
			throw std::runtime_error("Unable to create resource header");
		if (!span_hpp.is_open())
			throw std::runtime_error("Unable to create span header");
		
		auto subfolder = root.append(subfolder_name);
		if (!fs::exists(subfolder))
			fs::create_directory(subfolder);
	}

	~Saver() noexcept {
		try {
			resource_holder_hpp << "#pragma once" << std::endl << std::endl;
			resource_holder_hpp << "#include \"resource.hpp\"" << std::endl;

			for (auto&el : filenames)
				resource_holder_hpp << "#include \"" << subfolder_name << "/" << el << ".hpp\"" << std::endl;

			resource_holder_hpp << std::endl;

			for (auto&el : resource_holder_begin_text)
				resource_holder_hpp << el << std::endl;

			resource_holder_hpp << "\tstd::array<Resource, " << filenames.size() << "> resources {" << std::endl;
			for (auto&el : filenames)
				resource_holder_hpp << "\t\tResource(" << el << ",\t" << el << "_path)," << std::endl;
			resource_holder_hpp << "\t};" << std::endl;

			for (auto&el : resource_holder_text)
				resource_holder_hpp << el << std::endl;

			for (auto&el : resource_text)
				resource_hpp << el << std::endl;

			for (auto&el : span_text)
				span_hpp << el << std::endl;
		}
		catch (std::exception& e) {
			std::cerr << e.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Unknown exception has occured" << std::endl;
		}
	}

	void Save(const Resource& res) {
		Save(res.GetArray(), res.GetPath());
	}

	void Save(Resource::EmbeddedData data, fs::path resource_path) {
		try {
			if (verbose)
				std::cout << "embed.exe: saving " << resource_path.string();

			auto array_filename = "resource_" + std::to_string(fs::hash_value(resource_path));
			filenames.push_back(array_filename);
			auto header_filename = array_filename + ".hpp";
			auto header_path = fs::path(root).append(header_filename);

			std::stringstream out;
			out << "#pragma once" << std::endl << std::endl;
			out << R"(#include "../resource_holder.hpp")" << std::endl << std::endl;
			out << "namespace { " << std::endl;
			out << "\tconst std::array<std::uint8_t, " << data.size() << "> " << array_filename << " {" << std::endl;
			out << "\t\t";
			for (auto&el : data)
				out << Format(el);

			out << std::endl << "\t};" << std::endl;
			out << "\tconst auto " << array_filename << "_path = R\"(" << resource_path.string() << ")\";" << std::endl << "}" << std::endl;

			if (IsSame(out, header_path)) {
				if (verbose)
					std::cout << " ... Skipped" << std::endl;
				return;
			}
			std::ofstream out_file(header_path.c_str());
			if (!out_file.is_open())
				throw std::runtime_error("Unable to open file " + header_path.string());

			out_file << out.rdbuf();
			if (verbose)
				std::cout << std::endl;
		}
		catch (...) {
			resource_holder_hpp << "static_assert(false, R\"(Error while embedding " << resource_path.string() << " file)\");" << std::endl;
			throw;
		}
	}
};