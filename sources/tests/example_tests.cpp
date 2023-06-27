/*
 * License: https://wiimag.com/LICENSE
 * Copyright 2022-2023 Wiimag inc. All rights reserved.
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
