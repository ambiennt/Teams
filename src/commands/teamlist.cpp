#include "../main.h"

void TeamListCommand::execute(CommandOrigin const &origin, CommandOutput &output) {

	output.type = CommandOutputType::Normal;

	if (TeamUtils::teamToXuidMap.empty()) {
		output.error("No teams exist in the current session");
		return;
	}

	// only show health and position to the teammates of the player executor
	// if command origin is not player or player doesn't have a team, don't show that info for anyone
	std::optional<int32_t> selfTeamNum{};
	if (origin.getOriginType() == CommandOriginType::Player) {
		auto playerExecutor = reinterpret_cast<Player*>(origin.getEntity());
		if (playerExecutor) {
			auto dbIt = PLAYER_DB.Find(playerExecutor);
			if (dbIt.has_value()) {
				auto mapIt = TeamUtils::xuidToTeamMap.find(dbIt->xuid);
				if (mapIt != TeamUtils::xuidToTeamMap.end()) {
					selfTeamNum = mapIt->second;
				}
			}
		}
	}

	std::string listStr{};
	for (const auto& xuidList : TeamUtils::teamToXuidMap) {

		listStr += "\n§bTeam " + std::to_string(xuidList.first) + "§r: ";

		for (const auto &thisXuid : xuidList.second) {

			auto onlineIt = PLAYER_DB.Find(thisXuid);
			if (onlineIt) {
				if (selfTeamNum.has_value() && (selfTeamNum.value() == xuidList.first)) {
					listStr += TeamUtils::getFormattedHealthAndPosString(*(onlineIt->player));
				}
				else {
					listStr += onlineIt->name;
				}
				listStr += ", ";
			}
			else {
				auto offlineIt = PLAYER_DB.FindOffline(thisXuid);
				if (offlineIt) { // this should never be null but lets be safe
					listStr += offlineIt->name + " §c[offline]§r, ";
				}
			}
		}

		// remove extra characters, "\n" in the front and ", " in the back
		if (listStr.front() == '\n') {
			listStr.erase(0, 1);
		}
		if (listStr.substr(listStr.length() - 2) == ", ") {
			listStr.erase(listStr.length() - 2, 2);
		}
	}

	output.success(listStr);
}

void TeamListCommand::setup(CommandRegistry *registry) {

	std::string cmdName{"teamlist"};

	// order is important! register the command, alias, then overload
	registry->registerCommand(cmdName, "Outputs a list of all player teams.",
		CommandPermissionLevel::Any, CommandFlagUsage, CommandFlagNone);
	registry->registerAlias(cmdName, "tl");

	registry->registerOverload<TeamListCommand>(cmdName);
}