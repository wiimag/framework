/*
 * Copyright 2022-2023 - All rights reserved.
 * License: https://equals-forty-two.com/LICENSE
 */

#include <foundation/platform.h>

#if BUILD_DEVELOPMENT
 
#include <framework/tests/test_utils.h>

TEST_SUITE("Examples")
{
    TEST_CASE("Example #1")
    {
        CHECK(environment_argument("run-tests"));
    }
}

#endif // BUILD_DEVELOPMENT
