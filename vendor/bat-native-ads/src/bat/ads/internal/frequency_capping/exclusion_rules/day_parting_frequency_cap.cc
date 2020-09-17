/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/frequency_capping/exclusion_rules/day_parting_frequency_cap.h"

#include <string.h>
#include <vector>

#include "base/strings/stringprintf.h"
#include "bat/ads/internal/ads_impl.h"
#include "bat/ads/internal/frequency_capping/frequency_capping_util.h"

namespace ads {

DayPartingFrequencyCap::DayPartingFrequencyCap(
    const AdsImpl* const ads)
    : ads_(ads) {
  DCHECK(ads_);
}

DayPartingFrequencyCap::~DayPartingFrequencyCap() = default;

bool DayPartingFrequencyCap::ShouldExclude(
    const CreativeAdInfo& ad) {
  if (!DoesRespectCap(ad)) {
    last_message_ = base::StringPrintf("creativeSetId %s excluded as not "
        "within the scheduled timeslot", ad.creative_set_id.c_str());

    return true;
  }

  return false;
}

std::string DayPartingFrequencyCap::get_last_message() const {
  return last_message_;
}

bool DayPartingFrequencyCap::DoesRespectCap(
    const CreativeAdInfo& ad) const {
  // If there's no day part specified, let it be displayed
  if (ad.day_parts.size() == 0) {
    return true;
  }

  std::string current_dow = DayPartingFrequencyCap::GetCurrentDayOfWeek();
  std::string days_of_week;
  int current_minutes_from_start =
    DayPartingFrequencyCap::GetCurrentLocalMinutesFromStart();
  int start_time;
  int end_time;
  std::vector<std::string> parsed_day_part;

  for (const std::string& day_part : ad.day_parts) {
      parsed_day_part = DayPartingFrequencyCap::ParseDayPart(day_part);
      days_of_week = parsed_day_part[0];
      start_time = std::stoi(parsed_day_part[1]);
      end_time = std::stoi(parsed_day_part[2]);

      if (DayPartingFrequencyCap::HasDayOfWeekMatch(current_dow, days_of_week)
          && DayPartingFrequencyCap::HasTimeSlotMatch(
              current_minutes_from_start,
              start_time,
              end_time)) {
          return true;
      }
  }
  return false;
}

bool DayPartingFrequencyCap::HasDayOfWeekMatch(
    const std::string& current_dow,
    const std::string& days_of_week) const {
  /*
  for (int i = 0; i < days_of_week.length(); i++) {
      if (current_dow == days_of_week[i])
          return true;
  }
  return false;
  */
  return days_of_week.find(current_dow) != std::string::npos;
}

bool DayPartingFrequencyCap::HasTimeSlotMatch(
    const int current_minutes_from_start,
    const int start_time,
    const int end_time) const {

    return start_time <= current_minutes_from_start &&
      current_minutes_from_start <= end_time;
}

std::vector<std::string> DayPartingFrequencyCap::ParseDayPart(
    std::string day_part) const {
  std::vector<std::string> list;
  size_t pos = 0;
  std::string token;
  std::string delimiter = "_";

  while ((pos = day_part.find(delimiter)) != std::string::npos) {
      token = day_part.substr(0, pos);
      list.push_back(token);
      day_part.erase(0, pos + delimiter.length());
  }
  list.push_back(day_part);
  return list;
}

std::string DayPartingFrequencyCap::GetCurrentDayOfWeek() const {
  auto current_datetime = std::chrono::system_clock::now();
  auto unix_local_time = std::chrono::system_clock::to_time_t(current_datetime);
  std::tm * tm_time = std::localtime(&unix_local_time);
  return std::to_string(tm_time->tm_wday);
}

int DayPartingFrequencyCap::GetCurrentLocalMinutesFromStart() const {
  auto current_datetime = std::chrono::system_clock::now();
  auto unix_local_time = std::chrono::system_clock::to_time_t(current_datetime);
  std::tm * tm_time = std::localtime(&unix_local_time);
  return 60 * tm_time->tm_hour + tm_time->tm_min;
}

}  // namespace ads
