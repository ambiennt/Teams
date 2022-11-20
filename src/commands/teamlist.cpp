#include "../main.h"

void TeamListCommand::execute(CommandOrigin const &origin, CommandOutput &output) {

	output.type = CommandOutputType::Normal;

	if (TeamUtils::teamToXuidMap.empty()) {
		output.error("No teams exist in the current session");
		return;
	}

	std::string listStr{};
	for (const auto& xuidList : TeamUtils::teamToXuidMap) {

		listStr += "\n§bTeam " + std::to_string(xuidList.first) + "§r: ";

		for (const auto &thisXuid : xuidList.second) {

			auto onlineIt = PLAYER_DB.Find(thisXuid);
			if (onlineIt) {
				listStr += onlineIt->name + ", ";
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

	std::string cmdName("teamlist");

	registry->registerCommand(cmdName, "Outputs a list of all player teams.",
		CommandPermissionLevel::Any, CommandFlagUsage, CommandFlagNone);
	registry->registerOverload<TeamListCommand>(cmdName);
}