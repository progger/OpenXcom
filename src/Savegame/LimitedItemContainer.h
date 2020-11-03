#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ItemContainer.h"

namespace OpenXcom
{

class SavedGame;

class LimitedItemContainer : public ItemContainer
{
private:
  std::map<std::string, int> _limits;
  const Mod *_mod;
  SavedGame *_save;
public:
  LimitedItemContainer(const Mod *mod, SavedGame *save);
  virtual ~LimitedItemContainer() = default;
  void loadLimits(const YAML::Node& node);
  YAML::Node saveLimits() const;
  virtual void addItem(const std::string &id, int qty = 1) override;
  virtual void addItem(const RuleItem* item, int qty = 1) override;
  int getItemLimit(const std::string &id);
  int getItemLimit(const RuleItem* item);
  void setItemLimit(const std::string &id, int limit);
  std::map<std::string, int> *getLimits();
};

}
