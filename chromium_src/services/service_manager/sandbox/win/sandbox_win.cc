/* Copyright 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/process/launch.h"
#include "services/service_manager/sandbox/switches.h"

namespace {
void BraveLaunchOption(const std::string& process_type,
                       base::LaunchOptions *options) {
 if (process_type == service_manager::switches::kUtilityProcess)
    options->start_hidden = true;

}

}  // namespace
#include "../../../../../../services/service_manager/sandbox/win/sandbox_win.cc"
