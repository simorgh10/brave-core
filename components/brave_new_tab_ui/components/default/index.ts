/* This Source Code Form is subject to the terms of the Mozilla Public
 * License,
 * v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { StatsContainer, StatsItem } from './stats'
export * from './settings'
import { ListWidget, Tile, TileActionsContainer, TileAction, TileFavicon } from './gridSites'
import { SiteRemovalNotification, SiteRemovalText, SiteRemovalAction } from './notification'
import { ClockWidget } from './clock'
import { RewardsWidget } from './rewards'
import { BinanceWidget } from './binance'
import { TogetherWidget } from './together'
import { AddCardWidget } from './addCard'
import { GeminiWidget } from './gemini'
import { BitcoinDotComWidget } from './bitcoinDotCom'
import createWidget from './widget'

export * from './page'

export {
  StatsContainer,
  StatsItem,
  ListWidget,
  Tile,
  TileActionsContainer,
  TileAction,
  TileFavicon,
  SiteRemovalNotification,
  SiteRemovalText,
  SiteRemovalAction,
  ClockWidget,
  RewardsWidget,
  BinanceWidget,
  TogetherWidget,
  AddCardWidget,
  GeminiWidget,
  BitcoinDotComWidget,
  createWidget
}
