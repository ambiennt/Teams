#include "../main.h"

void TeamCommand::handleTeamAction(Player *player, CommandOutput &output, bool sendCommandFeedback) {

	auto it = PLAYER_DB.Find(player);
	if (!it) return;
	uint64_t xuid = it->xuid;

	if (xuid == 0) {
		output.error(it->name +
			"'s team change could not be processed because they are using an offline account");
		return;	
	}

	std::string teamStatusStr("Your team number has been ");

	switch (this->action) {
		case TeamAction::SET: {

			// add xuid into main map
			TeamUtils::xuidToTeamMap[xuid] = this->teamNumber;

			// update reverse team map
			for (const auto &pair : TeamUtils::xuidToTeamMap) {
				TeamUtils::teamToXuidMap[pair.second].insert(pair.first);
			}

			teamStatusStr += "set to §a" + std::to_string(this->teamNumber) + "§r";
			break;
		}
		case TeamAction::RESET: {
			TeamUtils::xuidToTeamMap.erase(xuid);
			teamStatusStr += "reset";
			break;
		}
		default: break;
	}

	if (sendCommandFeedback) {
		auto output = TextPacket::createTextPacket<TextPacketType::SystemMessage>(teamStatusStr);
		player->sendNetworkPacket(output);
	}
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

	bool sendCommandFeedback = (output.type != CommandOutputType::NoFeedback);

	for (auto player : selectedEntities) {
		handleTeamAction(player, output, sendCommandFeedback);
	}

	int32_t resultCount = selectedEntities.count();
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
	output.success(outputStrStart + " for " + std::to_string(resultCount) + std::string((resultCount == 1) ? " player" : " players"));
}

void TeamCommand::setup(CommandRegistry *registry) {
	using namespace commands;

	std::string cmdName("team");

	registry->registerCommand(cmdName, "Sets a player's team.",
		CommandPermissionLevel::GameMasters, CommandFlagUsage, CommandFlagNone);

	addEnum<TeamAction>(registry, "setTeamAction", {
		{ "set", TeamAction::SET }
	});
	addEnum<TeamAction>(registry, "resetTeamAction", {
		{ "reset", TeamAction::RESET }
	});

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