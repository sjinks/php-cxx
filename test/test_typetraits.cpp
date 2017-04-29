#include <string>
#include <gtest/gtest.h>
#include "phpcxx/typetraits.h"
#include "phpcxx/string.h"

extern "C" {
#include <Zend/zend.h>
#include <Zend/zend_long.h>
}

TEST(TypeTraits, testIsNullPointer)
{
    EXPECT_TRUE(phpcxx::is_null_pointer<std::nullptr_t>::value);
    EXPECT_TRUE(phpcxx::is_null_pointer<const std::nullptr_t>::value);
    EXPECT_TRUE(phpcxx::is_null_pointer<volatile std::nullptr_t>::value);
    EXPECT_TRUE(phpcxx::is_null_pointer<const volatile std::nullptr_t>::value);

    EXPECT_FALSE(phpcxx::is_null_pointer<std::nullptr_t&>::value);

    EXPECT_FALSE(phpcxx::is_null_pointer<void*>::value);
    EXPECT_FALSE(phpcxx::is_null_pointer<char*>::value);
    EXPECT_FALSE(phpcxx::is_null_pointer<int>::value);
}

TEST(TypeTraits, testIsBool)
{
    EXPECT_TRUE(phpcxx::is_bool<bool>::value);
    EXPECT_TRUE(phpcxx::is_bool<const bool>::value);
    EXPECT_TRUE(phpcxx::is_bool<volatile bool>::value);
    EXPECT_TRUE(phpcxx::is_bool<const volatile bool>::value);

    EXPECT_FALSE(phpcxx::is_bool<void*>::value);
    EXPECT_FALSE(phpcxx::is_bool<char>::value);
    EXPECT_FALSE(phpcxx::is_bool<int>::value);

    EXPECT_FALSE(phpcxx::is_bool<bool*>::value);
    EXPECT_FALSE(phpcxx::is_bool<bool[]>::value);
}

TEST(TypeTraits, testIsInteger)
{
    EXPECT_FALSE(phpcxx::is_integer<bool>::value);
    EXPECT_FALSE(phpcxx::is_integer<const bool>::value);
    EXPECT_FALSE(phpcxx::is_integer<volatile bool>::value);
    EXPECT_FALSE(phpcxx::is_integer<const volatile bool>::value);

    EXPECT_TRUE(phpcxx::is_integer<char>::value);
    EXPECT_TRUE(phpcxx::is_integer<int>::value);
    EXPECT_TRUE(phpcxx::is_integer<long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<long long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<zend_long>::value);

    EXPECT_TRUE(phpcxx::is_integer<signed char>::value);
    EXPECT_TRUE(phpcxx::is_integer<unsigned char>::value);
    EXPECT_TRUE(phpcxx::is_integer<unsigned int>::value);
    EXPECT_TRUE(phpcxx::is_integer<unsigned long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<unsigned long long int>::value);

    EXPECT_TRUE(phpcxx::is_integer<const char>::value);
    EXPECT_TRUE(phpcxx::is_integer<const int>::value);
    EXPECT_TRUE(phpcxx::is_integer<const long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<const long long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<const zend_long>::value);

    EXPECT_TRUE(phpcxx::is_integer<const signed char>::value);
    EXPECT_TRUE(phpcxx::is_integer<const unsigned char>::value);
    EXPECT_TRUE(phpcxx::is_integer<const unsigned int>::value);
    EXPECT_TRUE(phpcxx::is_integer<const unsigned long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<const unsigned long long int>::value);

    EXPECT_TRUE(phpcxx::is_integer<volatile char>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile int>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile long long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile zend_long>::value);

    EXPECT_TRUE(phpcxx::is_integer<volatile signed char>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile unsigned char>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile unsigned int>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile unsigned long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile unsigned long long int>::value);

    EXPECT_TRUE(phpcxx::is_integer<volatile const char>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile const int>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile const long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile const long long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile const zend_long>::value);

    EXPECT_TRUE(phpcxx::is_integer<volatile const signed char>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile const unsigned char>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile const unsigned int>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile const unsigned long int>::value);
    EXPECT_TRUE(phpcxx::is_integer<volatile const unsigned long long int>::value);

    EXPECT_FALSE(phpcxx::is_integer<float>::value);
    EXPECT_FALSE(phpcxx::is_integer<double>::value);
    EXPECT_FALSE(phpcxx::is_integer<long double>::value);

    EXPECT_FALSE(phpcxx::is_integer<void*>::value);
    EXPECT_FALSE(phpcxx::is_integer<char*>::value);
    EXPECT_FALSE(phpcxx::is_integer<int*>::value);
    EXPECT_FALSE(phpcxx::is_integer<int[]>::value);
}

TEST(TypeTraits, testIsNumeric)
{
    EXPECT_FALSE(phpcxx::is_numeric<bool>::value);
    EXPECT_FALSE(phpcxx::is_numeric<const bool>::value);
    EXPECT_FALSE(phpcxx::is_numeric<volatile bool>::value);
    EXPECT_FALSE(phpcxx::is_numeric<const volatile bool>::value);

    EXPECT_TRUE(phpcxx::is_numeric<char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<long long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<zend_long>::value);

    EXPECT_TRUE(phpcxx::is_numeric<signed char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<unsigned char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<unsigned int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<unsigned long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<unsigned long long int>::value);

    EXPECT_TRUE(phpcxx::is_numeric<float>::value);
    EXPECT_TRUE(phpcxx::is_numeric<double>::value);
    EXPECT_TRUE(phpcxx::is_numeric<long double>::value);

    EXPECT_TRUE(phpcxx::is_numeric<const char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const long long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const zend_long>::value);

    EXPECT_TRUE(phpcxx::is_numeric<const signed char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const unsigned char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const unsigned int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const unsigned long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const unsigned long long int>::value);

    EXPECT_TRUE(phpcxx::is_numeric<const float>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const double>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const long double>::value);

    EXPECT_TRUE(phpcxx::is_numeric<volatile char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile long long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile zend_long>::value);

    EXPECT_TRUE(phpcxx::is_numeric<volatile signed char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile unsigned char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile unsigned int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile unsigned long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile unsigned long long int>::value);

    EXPECT_TRUE(phpcxx::is_numeric<volatile float>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile double>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile long double>::value);

    EXPECT_TRUE(phpcxx::is_numeric<volatile const char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile const int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile const long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile const long long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile const zend_long>::value);

    EXPECT_TRUE(phpcxx::is_numeric<volatile const signed char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile const unsigned char>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile const unsigned int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile const unsigned long int>::value);
    EXPECT_TRUE(phpcxx::is_numeric<volatile const unsigned long long int>::value);

    EXPECT_TRUE(phpcxx::is_numeric<const volatile float>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const volatile double>::value);
    EXPECT_TRUE(phpcxx::is_numeric<const volatile long double>::value);

    EXPECT_FALSE(phpcxx::is_numeric<void*>::value);
    EXPECT_FALSE(phpcxx::is_numeric<char*>::value);
    EXPECT_FALSE(phpcxx::is_numeric<int*>::value);
    EXPECT_FALSE(phpcxx::is_numeric<int[]>::value);
}

TEST(TypeTraits, testIsPZval)
{
    EXPECT_TRUE(phpcxx::is_pzval<zval*>::value);

    EXPECT_FALSE(phpcxx::is_pzval<std::nullptr_t>::value);
    EXPECT_FALSE(phpcxx::is_pzval<zval>::value);

    EXPECT_FALSE(phpcxx::is_pzval<const zval*>::value);
    EXPECT_FALSE(phpcxx::is_pzval<zval* const>::value);
    EXPECT_FALSE(phpcxx::is_pzval<volatile zval*>::value);
    EXPECT_FALSE(phpcxx::is_pzval<zval* volatile>::value);
    EXPECT_FALSE(phpcxx::is_pzval<zval[]>::value);
}

TEST(TypeTraits, testIsString)
{
    EXPECT_TRUE(phpcxx::is_string<phpcxx::string>::value);
    EXPECT_TRUE(phpcxx::is_string<const phpcxx::string>::value);
    EXPECT_TRUE(phpcxx::is_string<volatile phpcxx::string>::value);
    EXPECT_TRUE(phpcxx::is_string<const volatile phpcxx::string>::value);

    EXPECT_FALSE(phpcxx::is_string<std::string>::value);
    EXPECT_FALSE(phpcxx::is_string<phpcxx::string*>::value);
    EXPECT_FALSE(phpcxx::is_string<phpcxx::string[]>::value);
}

TEST(TypeTraits, testIsPChar)
{
    EXPECT_TRUE(phpcxx::is_pchar<char*>::value);
    EXPECT_TRUE(phpcxx::is_pchar<const char*>::value);
    EXPECT_TRUE(phpcxx::is_pchar<volatile char*>::value);
    EXPECT_TRUE(phpcxx::is_pchar<const volatile char*>::value);

    EXPECT_TRUE(phpcxx::is_pchar<char* const>::value);
    EXPECT_TRUE(phpcxx::is_pchar<const char* const>::value);
    EXPECT_TRUE(phpcxx::is_pchar<volatile char* const>::value);
    EXPECT_TRUE(phpcxx::is_pchar<const volatile char* const>::value);

    EXPECT_TRUE(phpcxx::is_pchar<char* volatile>::value);
    EXPECT_TRUE(phpcxx::is_pchar<const char* volatile>::value);
    EXPECT_TRUE(phpcxx::is_pchar<volatile char* volatile>::value);
    EXPECT_TRUE(phpcxx::is_pchar<const volatile char* volatile>::value);

    EXPECT_TRUE(phpcxx::is_pchar<char* const volatile>::value);
    EXPECT_TRUE(phpcxx::is_pchar<const char* const volatile>::value);
    EXPECT_TRUE(phpcxx::is_pchar<volatile char* const volatile>::value);
    EXPECT_TRUE(phpcxx::is_pchar<const volatile char* const volatile>::value);

    EXPECT_TRUE(phpcxx::is_pchar<char[]>::value);
    EXPECT_TRUE(phpcxx::is_pchar<const char[]>::value);
    EXPECT_TRUE(phpcxx::is_pchar<volatile char[]>::value);
    EXPECT_TRUE(phpcxx::is_pchar<const volatile char[]>::value);

    EXPECT_FALSE(phpcxx::is_pchar<char*[]>::value);
    EXPECT_FALSE(phpcxx::is_pchar<char[][1]>::value);
    EXPECT_FALSE(phpcxx::is_pchar<char**>::value);
}
