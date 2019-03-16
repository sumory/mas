#pragma once
#include <gtest/gtest.h>
#include "mas/store.h"

class StoreTest : public testing::Test {
public:
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};