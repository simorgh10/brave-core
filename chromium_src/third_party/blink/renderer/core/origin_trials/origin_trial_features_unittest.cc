// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/platform/runtime_enabled_features.h"

#include <array>
#include <string>

#include "base/test/scoped_feature_list.h"
#include "base/test/task_environment.h"
#include "base/test/test_mock_time_task_runner.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/origin_trials/origin_trial_context.h"
#include "third_party/blink/renderer/core/origin_trials/origin_trials.h"
#include "third_party/blink/renderer/core/testing/dummy_page_holder.h"
#include "third_party/blink/renderer/platform/geometry/int_size.h"
#include "third_party/blink/renderer/platform/testing/runtime_enabled_features_test_helpers.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "third_party/blink/renderer/platform/wtf/vector.h"

namespace blink {
namespace {

typedef bool (*fn_feature_enabled_checker)(const FeatureContext*);
struct BraveDisabledTrial {
  const char* trial_name;
  OriginTrialFeature trial_feature;
  fn_feature_enabled_checker feature_enabled_checker;
};

// This list should be in sync with the ones in origin_trials.cc and
// origin_trial_context.cc overrides.
const std::array<BraveDisabledTrial, 3> kBraveDisabledTrials = {{
    // [Not released yet]
    //    {"DigitalGoods", OriginTrialFeature::kDigitalGoods,
    //     &RuntimeEnabledFeatures::DigitalGoodsEnabled},
    {"NativeFileSystem2", OriginTrialFeature::kNativeFileSystem,
     &RuntimeEnabledFeatures::NativeFileSystemEnabled},
    {"SignedExchangeSubresourcePrefetch",
     OriginTrialFeature::kSignedExchangeSubresourcePrefetch,
     &RuntimeEnabledFeatures::SignedExchangeSubresourcePrefetchEnabled},
    {"SubresourceWebBundles", OriginTrialFeature::kSubresourceWebBundles,
     &RuntimeEnabledFeatures::SubresourceWebBundlesEnabled},
}};

class OriginTrialFeaturesTest : public testing::Test {
 public:
  OriginTrialFeaturesTest() : testing::Test() {}

  void SetUp() override {
    test_task_runner_ = base::MakeRefCounted<base::TestMockTimeTaskRunner>();
    page_holder_ = std::make_unique<DummyPageHolder>(IntSize(800, 600));
    page_holder_->GetDocument().SetURL(KURL("https://example.com"));
  }

  LocalFrame* GetFrame() const { return &(page_holder_->GetFrame()); }
  LocalDOMWindow* GetWindow() const { return GetFrame()->DomWindow(); }
  ExecutionContext* GetExecutionContext() const {
    return page_holder_->GetDocument().GetExecutionContext();
  }

 private:
  base::test::SingleThreadTaskEnvironment task_environment_;
  std::unique_ptr<DummyPageHolder> page_holder_ = nullptr;
  scoped_refptr<base::TestMockTimeTaskRunner> test_task_runner_;
};

TEST_F(OriginTrialFeaturesTest, TestOriginTrialsNames) {
  // Check if our disabled trials are still valid trials in Chromium.
  // If any name fails check if the trial was removed and then it can be removed
  // from origin_trials.cc and origin_trial_context.cc overrides.
  for (const auto& trial : kBraveDisabledTrials) {
    EXPECT_TRUE(origin_trials::IsTrialValid_ForTests(trial.trial_name))
        << std::string("Failing trial: ") + trial.trial_name;
  }
}

TEST_F(OriginTrialFeaturesTest, TestBlinkRuntimeFeaturesViaOriginTrials) {
  // Test origin trials overrides.
  for (const auto& trial : kBraveDisabledTrials) {
    // Check that the runtime feature is initially disabled.
    EXPECT_FALSE((*trial.feature_enabled_checker)(GetExecutionContext()))
        << std::string("Failing trial: ") + trial.trial_name;

    // IsTrialValid override.
    EXPECT_FALSE(origin_trials::IsTrialValid(trial.trial_name))
        << std::string("Failing trial: ") + trial.trial_name;

    // Trials framework AddFeature override.
    GetWindow()->GetOriginTrialContext()->AddFeature(trial.trial_feature);
    EXPECT_FALSE(GetWindow()->GetOriginTrialContext()->IsFeatureEnabled(
        trial.trial_feature))
        << std::string("Failing trial: ") + trial.trial_name;

    // Trials framework force via origin trial names.
    WTF::Vector<WTF::String> forced_trials = {trial.trial_name};
    GetWindow()->GetOriginTrialContext()->AddForceEnabledTrials(forced_trials);
    EXPECT_FALSE(GetWindow()->GetOriginTrialContext()->IsFeatureEnabled(
        trial.trial_feature))
        << std::string("Failing trial: ") + trial.trial_name;

    // Check that the runtime feature is still disabled.
    EXPECT_FALSE((*trial.feature_enabled_checker)(GetExecutionContext()))
        << std::string("Failing trial: ") + trial.trial_name;
  }
}

TEST_F(OriginTrialFeaturesTest, TestBlinkRuntimeFeaturesWithoutOriginTrials) {
  // The following don't currently have origin trials associated with them,
  // but if they end up having them we should be able to catch them.
  RuntimeEnabledFeatures::SetDigitalGoodsEnabled(false);
  // [Available in Cr87] RuntimeEnabledFeatures::SetDirectSocketsEnabled(false);
  RuntimeEnabledFeatures::SetLangClientHintHeaderEnabled(false);
  RuntimeEnabledFeatures::SetSignedExchangePrefetchCacheForNavigationsEnabled(
      false);

  // Enable all origin trial controlled features.
  RuntimeEnabledFeatures::SetOriginTrialControlledFeaturesEnabled(true);

  // Check if features in question became enabled.
  EXPECT_FALSE(
      RuntimeEnabledFeatures::DigitalGoodsEnabled(GetExecutionContext()));
  // [Available in Cr87]
  // EXPECT_FALSE(
  //    RuntimeEnabledFeatures::DirectSocketsEnabled(GetExecutionContext()));
  EXPECT_FALSE(RuntimeEnabledFeatures::LangClientHintHeaderEnabled(
      GetExecutionContext()));
  EXPECT_FALSE(
      RuntimeEnabledFeatures::SignedExchangePrefetchCacheForNavigationsEnabled(
          GetExecutionContext()));
}

}  // namespace
}  // namespace blink
