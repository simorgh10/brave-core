/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BAT_ADS_INTERNAL_ELIGIBLE_ADS_H_
#define BAT_ADS_INTERNAL_ELIGIBLE_ADS_H_

#include <string>
#include <vector>

namespace ads {

class EligibleAds {
 public:
  virtual ~EligibleAds() = default;

  virtual std::vector<std::string> GetCreativeInstanceIdList() = 0;

  virtual std::string get_last_message() const = 0;
};

}  // namespace ads

#endif  // BAT_ADS_INTERNAL_ELIGIBLE_ADS_H_
