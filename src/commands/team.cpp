#include "../main.h"

bool TeamCommand::handleTeamAction(Player &player, CommandOutput &output, bool sendCommandFeedback) {

	auto it1 = PLAYER_DB.Find(&player);
	if (!it1) return false;
	uint64_t currXuid = it1->xuid; // the xuid of the player who we are iterating through to change team status

	if (currXuid == 0) {
		output.error(it1->name + "'s team change could not be processed because they are using an offline account");
		return false;
	}

	std::string teamStatusStr{"Your team number has been "};

	switch (this->action) {
		case TeamAction::SET: {

			// only set it if we actually need to...
			// if they arent in the map or their current team doesnt match the requested one, then lets update it
			auto it2 = TeamUtils::xuidToTeamMap.find(currXuid);
			if ((it2 == TeamUtils::xuidToTeamMap.end()) || (it2->second != this->teamNumber)) {

				// add xuid into main map
				TeamUtils::xuidToTeamMap[currXuid] = this->teamNumber;

				// add team number into reverse team map
				// recreating the whole map is really slow but otherwise the code gets really ugly trust me
				TeamUtils::teamToXuidMap.clear();
				for (const auto& pair : TeamUtils::xuidToTeamMap) {
					TeamUtils::teamToXuidMap[pair.second].insert(pair.first);
				}
			}

			teamStatusStr += "set to §a" + std::to_string(this->teamNumber) + "§r";
			break;
		}
		case TeamAction::RESET: {

			// only reset it if we actually need to...
			// if theyre not in map we can assume they already dont have a team and thus resetting would be pointless
			auto it3 = TeamUtils::xuidToTeamMap.find(currXuid);
			if (it3 != TeamUtils::xuidToTeamMap.end()) {

				// remove xuid from main map by iterator...
				TeamUtils::xuidToTeamMap.erase(it3);

				TeamUtils::teamToXuidMap.clear();
				for (const auto& pair : TeamUtils::xuidToTeamMap) {
					TeamUtils::teamToXuidMap[pair.second].insert(pair.first);
				}

				// they might not be in the team chat map but we wanna make sure they cant be in this if they arent on a team
				TeamUtils::xuidsInTeamChat.erase(currXuid);
			}

			teamStatusStr += "reset";
			break;
		}
		default: break;
	}

	if (sendCommandFeedback) {
		auto teamStatusPkt = TextPacket::createTextPacket<TextPacketType::SystemMessage>(teamStatusStr);
		it1->player->sendNetworkPacket(teamStatusPkt);
	}

	return true;
}

void TeamCommand::execute(CommandOrigin const &origin, CommandOutput &output) {

	auto selectedEntities = this->selector.results(origin);
	if (selectedEntities.empty()) {
		output.error("No targets matched selector");
		return;
	}

	if ((this->action == TeamAction::SET) && (this->teamNumber < 1)) {
		output.error("Team number must be at least 1");
		return;
	}

	bool sendCommandFeedback = (output.type != CommandOutputType::NoFeedback); // in this cmd we wanna use the actual feedback policy
	int32_t teamAddSuccessCount = 0;
	for (auto player : selectedEntities) {
		if (this->handleTeamAction(*player, output, sendCommandFeedback)) {
			teamAddSuccessCount++;
		}
	}

	std::string outputStrStart{"Successfully "};
	switch (this->action) {
		case TeamAction::SET: {
			outputStrStart += "set the team number to " + std::to_string(this->teamNumber);
			break;
		}
		case TeamAction::RESET: {
			outputStrStart += "reset the team number";
			break;
		}
		default: break;
	}

	output.success(outputStrStart + " for " + std::to_string(teamAddSuccessCount) + ((teamAddSuccessCount == 1) ? " player" : " players"));
}

void TeamCommand::setup(CommandRegistry *registry) {
	using namespace commands;

	std::string cmdName{"team"};

	addEnum<TeamAction>(registry, "setTeamAction", {
		{ "set", TeamAction::SET }
	});
	addEnum<TeamAction>(registry, "resetTeamAction", {
		{ "reset", TeamAction::RESET }
	});

	registry->registerCommand(cmdName, "Sets a player's team.",
		CommandPermissionLevel::GameMasters, CommandFlagUsage, CommandFlagNone);

	registry->registerOverload<TeamCommand>(cmdName,
		mandatory<CommandParameterDataType::ENUM>(&TeamCommand::action, "set", "setTeamAction"),
		mandatory(&TeamCommand::selector, "player"),
		mandatory(&TeamCommand::teamNumber, "teamNumber")
	);
	registry->registerOverload<TeamCommand>(cmdName,
		mandatory<CommandParameterDataType::ENUM>(&TeamCommand::action, "reset", "resetTeamAction"),
		optional(&TeamCommand::selector, "player")
	);
}