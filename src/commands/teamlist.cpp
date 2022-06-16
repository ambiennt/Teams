#include "../main.h"

void TeamListCommand::execute(CommandOrigin const &origin, CommandOutput &output) {

	output.type = CommandOutputType::Normal;

	std::unordered_map<int32_t, std::vector<uint64_t>> reverseTeamMap;
	std::string listStr;

	for (const auto& pair : TeamUtils::playerTeams) {
		reverseTeamMap[pair.second].push_back(pair.first);
	}

	for (const auto& thisList : reverseTeamMap) {

		listStr += "\n§bTeam " + std::to_string(thisList.first) + "§r: ";

		for (const auto& thisXuid : thisList.second) {

			auto it = PLAYER_DB.Find(thisXuid);
			if (it) {
				listStr += it->name + ", ";
			}
			else {
				listStr += PLAYER_DB.FindOffline(thisXuid)->name + " §c[offline]§r, ";
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

	if (listStr.empty()) {
		return output.error("No teams exist in the current session");
	}

	output.success(listStr);
}

void TeamListCommand::setup(CommandRegistry *registry) {

	std::string cmdName("teamlist");

	registry->registerCommand(cmdName, "Outputs a list of all player teams.",
		CommandPermissionLevel::Any, CommandFlagUsage, CommandFlagNone);
	registry->registerOverload<TeamListCommand>(cmdName);
}