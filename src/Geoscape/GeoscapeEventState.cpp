/*
 * Copyright 2010-2019 OpenXcom Developers.
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
#include "GeoscapeEventState.h"
#include "../Engine/Game.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/RNG.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Mod/City.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleEvent.h"
#include "../Mod/RuleRegion.h"
#include "../Savegame/Base.h"
#include "../Savegame/GeoscapeEvent.h"
#include "../Savegame/Region.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Transfer.h"
#include "../Ufopaedia/Ufopaedia.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Geoscape Event window.
 * @param geoEvent Pointer to the event.
 */
GeoscapeEventState::GeoscapeEventState(GeoscapeEvent *geoEvent)
{
	_screen = false;

	// Create objects
	_window = new Window(this, 256, 176, 32, 12, POPUP_BOTH);
	_txtTitle = new Text(236, 32, 42, 26);
	_txtMessage = new Text(236, 94, 42, 61);
	_btnOk = new TextButton(100, 18, 110, 158);

	// Set palette
	setInterface("geoscapeEvent");

	add(_window, "window", "geoscapeEvent");
	add(_txtTitle, "text1", "geoscapeEvent");
	add(_txtMessage, "text2", "geoscapeEvent");
	add(_btnOk, "button", "geoscapeEvent");

	centerAllSurfaces();

	// Set up objects
	_window->setBackground(_game->getMod()->getSurface(geoEvent->getRules().getBackground()));

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setBig();
	_txtTitle->setWordWrap(true);
	_txtTitle->setText(tr(geoEvent->getRules().getName()));

	_txtMessage->setVerticalAlign(ALIGN_TOP);
	_txtMessage->setWordWrap(true);
	_txtMessage->setText(tr(geoEvent->getRules().getDescription()));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)& GeoscapeEventState::btnOkClick);

	eventLogic(geoEvent);
}

/**
 * Helper performing event logic.
 * @param geoEvent Pointer to the event.
 */
void GeoscapeEventState::eventLogic(GeoscapeEvent *geoEvent)
{
	SavedGame *save = _game->getSavedGame();
	Base *hq = save->getBases()->front();
	const Mod *mod = _game->getMod();
	const RuleEvent &rule = geoEvent->getRules();

	RuleRegion *regionRule = nullptr;
	if (!rule.getRegionList().empty())
	{
		size_t pickRegion = RNG::generate(0, rule.getRegionList().size() - 1);
		auto regionName = rule.getRegionList().at(pickRegion);
		regionRule = _game->getMod()->getRegion(regionName, true);
		std::string place = regionName;

		if (rule.isCitySpecific())
		{
			size_t cities = regionRule->getCities()->size();
			if (cities > 0)
			{
				size_t pickCity = RNG::generate(0, cities - 1);
				City *city = regionRule->getCities()->at(pickCity);
				place = city->getName(_game->getLanguage());
			}
		}

		std::string titlePlus = tr(geoEvent->getRules().getName()).arg(place);
		_txtTitle->setText(titlePlus);

		std::string messagePlus = tr(geoEvent->getRules().getDescription()).arg(place);
		_txtMessage->setText(messagePlus);
	}

	// 1. give/take score points
	if (regionRule)
	{
		for (auto region : *_game->getSavedGame()->getRegions())
		{
			if (region->getRules() == regionRule)
			{
				region->addActivityXcom(rule.getPoints());
				break;
			}
		}
	}
	else
	{
		save->addResearchScore(rule.getPoints());
	}

	// 2. give/take funds
	save->setFunds(save->getFunds() + rule.getFunds());

	// 3. spawn/transfer item into the HQ
	if (!rule.getItemList().empty())
	{
		size_t pickItem = RNG::generate(0, rule.getItemList().size() - 1);
		const RuleItem *eventSpawnedItem = mod->getItem(rule.getItemList().at(pickItem), true);
		if (eventSpawnedItem)
		{
			Transfer *transferSpwanedItem = new Transfer(1);
			transferSpwanedItem->setItems(eventSpawnedItem->getType(), 1);
			hq->getTransfers()->push_back(transferSpwanedItem);
		}
	}

	// 4. give bonus research
	std::vector<const RuleResearch*> possibilities;

	for (auto rName : rule.getResearchList())
	{
		const RuleResearch *rRule = mod->getResearch(rName, true);
		if (!save->isResearched(rRule, false))
		{
			possibilities.push_back(rRule);
		}
	}

	if (!possibilities.empty())
	{
		size_t pickResearch = RNG::generate(0, possibilities.size() - 1);
		const RuleResearch *eventResearch = possibilities.at(pickResearch);
		save->addFinishedResearch(eventResearch, mod, hq, true);
		_researchName = eventResearch->getName();

		if (!eventResearch->getLookup().empty())
		{
			const RuleResearch *lookupResearch = mod->getResearch(eventResearch->getLookup(), true);
			save->addFinishedResearch(lookupResearch, mod, hq, true);
			_researchName = lookupResearch->getName();
		}
	}
}

/**
 *
 */
GeoscapeEventState::~GeoscapeEventState()
{
	// Empty by design
}

/**
 * Closes the window and shows a pedia article if needed.
 * @param action Pointer to an action.
 */
void GeoscapeEventState::btnOkClick(Action *)
{
	_game->popState();

	if (!_researchName.empty())
	{
		Ufopaedia::openArticle(_game, _researchName);
	}
}

}
