#include <unity.h>
#include <string.h>

#include "format/format.h"

namespace {
	// Buffer as big as the matrix uses for one number
	constexpr size_t NUMBER_BUFFER_SIZE = 30;

	// Format a number into a fresh buffer and check the result
	void assertFormats(const char* expected, double value, int decimals, formatNum formatType) {
		char output[NUMBER_BUFFER_SIZE] = {0};
		formatNumber(value, decimals, formatType, output, sizeof(output));
		TEST_ASSERT_EQUAL_STRING(expected, output);
	}

	// Format a price with the decimals chosen for it and check the result
	void assertFormatsPrice(const char* expected, double value, formatNum formatType) {
		assertFormats(expected, value, getPriceDecimals(value), formatType);
	}
}

// Nothing to prepare before each test
void setUp() {}

// Nothing to clean up after each test
void tearDown() {}

/******************************** Separators ********************************/

// US format uses commas for thousands and a dot for decimals
void test_us_format_separators() {
	assertFormats("1,234,567.89", 1234567.891, 2, FORMAT_US);
}

// EU format uses dots for thousands and a comma for decimals
void test_eu_format_separators() {
	assertFormats("1.234.567,89", 1234567.891, 2, FORMAT_EU);
}

// Numbers below a thousand get no thousands separator
void test_small_numbers_have_no_thousands_separator() {
	assertFormats("999.00", 999, 2, FORMAT_US);
	assertFormats("0.50", 0.5, 2, FORMAT_US);
}

// Negative numbers keep their sign in front of the digits
void test_negative_numbers() {
	assertFormats("-1,234.50", -1234.5, 2, FORMAT_US);
	assertFormats("-0.05", -0.05, 2, FORMAT_US);
}

// A value that rounds to zero is shown without a minus sign
void test_tiny_negative_rounds_to_plain_zero() {
	assertFormats("0.00", -0.001, 2, FORMAT_US);
}

// Rounding carries into the integer part
void test_rounding_carries_into_the_integer_part() {
	assertFormats("10.00", 9.999, 2, FORMAT_US);
	assertFormats("1,000.00", 999.999, 2, FORMAT_US);
}

// Market caps in the trillions are formatted in full
void test_large_market_cap() {
	assertFormats("2,400,000,000,000.00", 2.4e12, 2, FORMAT_US);
}

// The optional currency symbol goes in front with a space
void test_currency_symbol_prefix() {
	TEST_ASSERT_EQUAL_STRING("$ 5.00", addThousandsSeparators(5, 2, '.', ',', '$'));
}

/******************************** Price decimals ********************************/

// Prices from $1 up and zero use 2 decimals
void test_price_decimals_for_regular_prices() {
	TEST_ASSERT_EQUAL_INT(2, getPriceDecimals(95123.45));
	TEST_ASSERT_EQUAL_INT(2, getPriceDecimals(1.0));
	TEST_ASSERT_EQUAL_INT(2, getPriceDecimals(0.0));
	TEST_ASSERT_EQUAL_INT(2, getPriceDecimals(-1234.5));
}

// Prices below $1 keep 4 significant digits
void test_price_decimals_below_one_dollar() {
	TEST_ASSERT_EQUAL_INT(4, getPriceDecimals(0.5));
	TEST_ASSERT_EQUAL_INT(5, getPriceDecimals(0.08123));
	TEST_ASSERT_EQUAL_INT(5, getPriceDecimals(-0.08123));
	TEST_ASSERT_EQUAL_INT(6, getPriceDecimals(0.00234));
}

// Very small values are capped at 8 decimals
void test_price_decimals_are_capped() {
	TEST_ASSERT_EQUAL_INT(8, getPriceDecimals(1e-12));
}

// Kaspa prices used to be shown as "0.08" with a fixed 2 decimals
void test_kaspa_price_keeps_its_digits() {
	assertFormatsPrice("0.08123", 0.08123, FORMAT_US);
	assertFormatsPrice("0,08123", 0.08123, FORMAT_EU);
}

// A tiny daily change used to be shown as "-0.00"
void test_tiny_daily_change_is_readable() {
	assertFormatsPrice("-0.002340", -0.00234, FORMAT_US);
}

// Bitcoin prices are unchanged by the adaptive decimals
void test_bitcoin_price_is_unchanged() {
	assertFormatsPrice("95,123.46", 95123.456, FORMAT_US);
}

/******************************** Output buffer ********************************/

// A buffer that is too small gets a cut-off, terminated text and nothing past its end
void test_small_output_buffer_is_cut_off_safely() {
	char output[10];
	memset(output, 'x', sizeof(output));

	// Format into the first 6 bytes only
	formatNumber(1234.5, 2, FORMAT_US, output, 6);

	// Expect 5 characters, the terminator, and the rest untouched
	TEST_ASSERT_EQUAL_STRING("1,234", output);
	TEST_ASSERT_EQUAL_CHAR('x', output[6]);
}

// A zero-length buffer is left untouched
void test_zero_length_buffer_is_untouched() {
	char output[4] = { 'x', 'x', 'x', 'x' };

	// Format into no space at all
	formatNumber(1, 2, FORMAT_US, output, 0);

	// Expect nothing written
	TEST_ASSERT_EQUAL_CHAR('x', output[0]);
}

// Run all the format tests
int main() {
	UNITY_BEGIN();

	// Separators
	RUN_TEST(test_us_format_separators);
	RUN_TEST(test_eu_format_separators);
	RUN_TEST(test_small_numbers_have_no_thousands_separator);
	RUN_TEST(test_negative_numbers);
	RUN_TEST(test_tiny_negative_rounds_to_plain_zero);
	RUN_TEST(test_rounding_carries_into_the_integer_part);
	RUN_TEST(test_large_market_cap);
	RUN_TEST(test_currency_symbol_prefix);

	// Price decimals
	RUN_TEST(test_price_decimals_for_regular_prices);
	RUN_TEST(test_price_decimals_below_one_dollar);
	RUN_TEST(test_price_decimals_are_capped);
	RUN_TEST(test_kaspa_price_keeps_its_digits);
	RUN_TEST(test_tiny_daily_change_is_readable);
	RUN_TEST(test_bitcoin_price_is_unchanged);

	// Output buffer
	RUN_TEST(test_small_output_buffer_is_cut_off_safely);
	RUN_TEST(test_zero_length_buffer_is_untouched);

	return UNITY_END();
}