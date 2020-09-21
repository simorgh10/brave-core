/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/playlist/browser/playlist_download_request_manager.h"

#include "chrome/common/chrome_isolated_world_ids.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_handle.h"
#include "content/public/browser/web_contents.h"
#include "third_party/re2/src/re2/re2.h"

namespace playlist {

namespace {

std::string GetScript(const std::string& youtubedown_script,
                      const std::string& url) {
  return youtubedown_script +
         " (function() { return window.youtubedown_urls('" + url +"'); })()";
}

}  // namespace

PlaylistDownloadRequestManager::PlaylistDownloadRequestManager(
    content::BrowserContext* context,
    Delegate* delegate,
    PlaylistYoutubeDownComponentManager* manager)
    : context_(context),
      delegate_(delegate),
      youtubedown_component_manager_(manager) {
  observed_.Add(youtubedown_component_manager_);
  youtubedown_script_ = youtubedown_component_manager_->youtubedown_script();
}

PlaylistDownloadRequestManager::~PlaylistDownloadRequestManager() {
}

void PlaylistDownloadRequestManager::CreateWebContents() {
  if (webcontents_)
    return;

  // TODO(simonhong): Destroy webcontents when idle.
  constexpr char kYoutubeURL[] = "https://www.youtube.com/";
  content::WebContents::CreateParams create_params(context_, nullptr);
  webcontents_ = content::WebContents::Create(create_params);
  webcontents_->GetController().LoadURLWithParams(
      content::NavigationController::LoadURLParams(GURL(kYoutubeURL)));
  Observe(webcontents_.get());
}

void PlaylistDownloadRequestManager::GeneratePlaylistCreateParamsForYoutubeURL(
    const std::string& url) {

  if (!ReadyToRunYoutubeDownJS()) {
    pending_youtube_urls_.push_back(url);

    CreateWebContents();
    youtubedown_component_manager_->RegisterIfNeeded();
    return;
  }

  FetchYoutubeDownData(url);
}

void PlaylistDownloadRequestManager::OnYoutubeDownScriptReady(
    const std::string& youtubedown_script) {
  youtubedown_script_ = youtubedown_script;

  if (!ReadyToRunYoutubeDownJS())
    return;

  FetchAllPendingYoutubeURLs();
}

void PlaylistDownloadRequestManager::FetchAllPendingYoutubeURLs() {
  if (pending_youtube_urls_.empty())
    return;

    // Run all pending request.
  for (const auto& url : pending_youtube_urls_) {
    FetchYoutubeDownData(url);
  }

  pending_youtube_urls_.clear();
}

void PlaylistDownloadRequestManager::FetchYoutubeDownData(
    const std::string& url) const {
  webcontents_->GetMainFrame()->ExecuteJavaScriptInIsolatedWorld(
      base::UTF8ToUTF16(GetScript(youtubedown_script_, url)),
      base::BindOnce(&PlaylistDownloadRequestManager::OnGetYoutubeDownData,
                     weak_factory_.GetWeakPtr()),
      ISOLATED_WORLD_ID_CHROME_INTERNAL);
}

bool PlaylistDownloadRequestManager::ReadyToRunYoutubeDownJS() const {
  if (!youtubedown_script_.empty() && webcontents_ && webcontents_ready_)
    return true;

  return false;
}

void PlaylistDownloadRequestManager::RenderFrameCreated(
    content::RenderFrameHost* render_frame_host) {
  // To run youtubedown.js, frame should be ready.
  // After that, we don't need to observe.
  webcontents_ready_ = true;
  Observe(nullptr);

  FetchAllPendingYoutubeURLs();
}

void PlaylistDownloadRequestManager::OnGetYoutubeDownData(base::Value value) {
  if (!value.is_list()) {
    LOG(ERROR) << __func__ << " Got invalid value after running youtubedown.js";
    return;
  }

  CreatePlaylistParams p;
  const bool has_audio = value.GetList().size() > 1;
  const auto list = value.GetList();
  p.playlist_name = *list[0].FindStringKey("file");
  // Get clean playlist name.
  RE2::Replace(&p.playlist_name, "\\s\\[.*$", "");
  p.playlist_thumbnail_url = *list[0].FindStringKey("thumb");
  if (std::string* url = list[0].FindStringKey("url")) {
    p.video_media_files.emplace_back(*url, "");
  } else {
    DCHECK(list[0].FindListKey("url"));
    for (const auto& value : list[0].FindListKey("url")->GetList()) {
      p.video_media_files.emplace_back(value.GetString(), "");
    }
  }

  if (has_audio) {
    if (std::string* url = list[1].FindStringKey("url")) {
      p.audio_media_files.emplace_back(*url, "");
    } else {
      DCHECK(list[0].FindListKey("url"));
      for (const auto& value : list[1].FindListKey("url")->GetList()) {
        p.audio_media_files.emplace_back(value.GetString(), "");
      }
    }
  }

  delegate_->OnPlaylistCreationParamsReady(p);
}
}  // namespace playlist
