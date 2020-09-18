// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/runtime_enabled_features.h"

#include <vector>

#include "base/test/scoped_feature_list.h"
#include "content/public/common/content_features.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/public/common/features.h"
#include "third_party/blink/renderer/platform/testing/runtime_enabled_features_test_helpers.h"

class RuntimeEnabledFeaturesTest : public testing::Test {
 public:
  RuntimeEnabledFeaturesTest() : testing::Test() {}

  void CheckAllDisabled() {
    using rtf = blink::RuntimeEnabledFeatures;
    EXPECT_FALSE(rtf::DigitalGoodsEnabled(nullptr));
    // [Available in Cr87] EXPECT_FALSE(rtf::DirectSocketsEnabled(nullptr));
    EXPECT_FALSE(rtf::LangClientHintHeaderEnabled(nullptr));
    EXPECT_FALSE(rtf::NativeFileSystemEnabled(nullptr));
    EXPECT_FALSE(rtf::SubresourceWebBundlesEnabled(nullptr));
    EXPECT_FALSE(
        rtf::SignedExchangePrefetchCacheForNavigationsEnabled(nullptr));
    EXPECT_FALSE(rtf::SignedExchangeSubresourcePrefetchEnabled(nullptr));
  }
};

TEST_F(RuntimeEnabledFeaturesTest, TestBlinkRuntimeFeaturesDisabled) {
  CheckAllDisabled();
}
