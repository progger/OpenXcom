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
#include "../Mod/Mod.h"
#include "../Mod/RuleItem.h"
#include "../Savegame/SavedGame.h"
#include "LimitedItemContainer.h"

namespace OpenXcom
{

LimitedItemContainer::LimitedItemContainer(const Mod *mod, SavedGame *save) : _mod(mod), _save(save)
{
}

void LimitedItemContainer::loadLimits(const YAML::Node &node)
{
  _limits = node.as< std::map<std::string, int> >(_limits);
}

YAML::Node LimitedItemContainer::saveLimits() const
{
  return YAML::Node(_limits);
}

void LimitedItemContainer::addItem(const std::string &id, int qty)
{
  RuleItem *item = _mod->getItem(id);
  addItem(item, qty);
}

void LimitedItemContainer::addItem(const RuleItem *item, int qty)
{
  if (!item) return;
  std::string id = item->getType();
  if (id.empty()) return;
  ItemContainer::addItem(id, qty);

  auto it = _limits.find(id);
  if (it == _limits.end()) return;
  int c = getItem(id);
  int d = c - it->second;
  if (d <= 0) return;

  removeItem(id, d);
  _save->setFunds(_save->getFunds() + item->getSellCost() * d);
}

int LimitedItemContainer::getItemLimit(const std::string &id)
{
  if (id.empty()) return -1;
  auto it = _limits.find(id);
  return it != _limits.end() ? it->second : -1;
}

int LimitedItemContainer::getItemLimit(const RuleItem *item)
{
  return item ? getItemLimit(item->getType()) : -1;
}

void LimitedItemContainer::setItemLimit(const std::string &id, int limit)
{
  if (id.empty()) return;
  if (limit >= 0)
  {
    _limits[id] = limit;
    addItem(id, 0);
  }
  else
  {
    auto it = _limits.find(id);
    if (it != _limits.end())
    {
      _limits.erase(it);
    }
  }
}

std::map<std::string, int> *LimitedItemContainer::getLimits()
{
  return &_limits;
}

}
