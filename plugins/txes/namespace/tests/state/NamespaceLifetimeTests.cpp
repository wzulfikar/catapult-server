#include "src/state/NamespaceLifetime.h"
#include "tests/test/nodeps/Equality.h"
#include "tests/TestHarness.h"

namespace catapult { namespace state {

	/// region ctor

	TEST(NamespaceLifetimeTests, CanCreateNamespaceLifetime) {
		// Act:
		NamespaceLifetime lifetime(Height(123), Height(234));

		// Assert:
		EXPECT_EQ(Height(123), lifetime.Start);
		EXPECT_EQ(Height(234), lifetime.End);
	}

	TEST(NamespaceLifetimeTests, CanCreateNamespaceLifetimeWithMinLifetime) {
		// Act:
		NamespaceLifetime lifetime(Height(233), Height(234));

		// Assert:
		EXPECT_EQ(Height(233), lifetime.Start);
		EXPECT_EQ(Height(234), lifetime.End);
	}

	TEST(NamespaceLifetimeTests, CannotCreateNamespaceLifetimeWithZeroLifetime) {
		// Assert:
		EXPECT_THROW(NamespaceLifetime(Height(123), Height(123)), catapult_invalid_argument);
	}

	TEST(NamespaceLifetimeTests, CannotCreateNamespaceLifetimeWithNegativeLifetime) {
		// Assert:
		EXPECT_THROW(NamespaceLifetime(Height(125), Height(124)), catapult_invalid_argument);
		EXPECT_THROW(NamespaceLifetime(Height(125), Height(63)), catapult_invalid_argument);
	}

	// endregion

	// region isActive

	TEST(NamespaceLifetimeTests, IsActiveReturnsTrueIfHeightIsWithinLifetime) {
		// Arrange:
		NamespaceLifetime lifetime(Height(123), Height(234));

		// Assert:
		for (auto height : { 123u, 124u, 213u, 232u, 233u })
			EXPECT_TRUE(lifetime.isActive(Height(height))) << "at height " << height;
	}

	TEST(NamespaceLifetimeTests, IsActiveReturnsFalseIfHeightIsNotWithinLifetime) {
		// Arrange:
		NamespaceLifetime lifetime(Height(123), Height(234));

		// Assert:
		for (auto height : { 1u, 2u, 121u, 122u, 234u, 235u, 5000u, 100000u })
			EXPECT_FALSE(lifetime.isActive(Height(height))) << "at height " << height;
	}

	// endregion

	// region equality

	namespace {
		const char* Default_Key = "default";

		auto GenerateEqualityInstanceMap() {
			std::unordered_map<std::string, NamespaceLifetime> map;
			map.emplace(Default_Key, NamespaceLifetime(Height(123), Height(234)));
			map.emplace("copy", NamespaceLifetime(Height(123), Height(234)));
			map.emplace("diff-start-height", NamespaceLifetime(Height(169), Height(234)));
			map.emplace("diff-end-height", NamespaceLifetime(Height(123), Height(345)));
			map.emplace("diff-start-and-end-height", NamespaceLifetime(Height(169), Height(345)));
			return map;
		}

		std::unordered_set<std::string> GetEqualTags() {
			return { Default_Key, "copy" };
		}
	}

	TEST(NamespaceLifetimeTests, OperatorEqualReturnsTrueForEqualObjects) {
		// Assert:
		test::AssertOperatorEqualReturnsTrueForEqualObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	TEST(NamespaceLifetimeTests, OperatorNotEqualReturnsTrueForUnequalObjects) {
		// Assert:
		test::AssertOperatorNotEqualReturnsTrueForUnequalObjects(Default_Key, GenerateEqualityInstanceMap(), GetEqualTags());
	}

	// endregion
}}