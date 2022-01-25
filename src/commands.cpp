#include "main.h"

class TeamCommand : public Command {
public:
	TeamCommand() { selector.setIncludeDeadPlayers(true); }

	enum class TeamAction { Set, Reset } action = TeamAction::Set;
	CommandSelector<Player> selector;
	int32_t teamNumber = 1;

	void handleTeamAction(Player *player, CommandOutput &output, Mod::PlayerDatabase &db, bool sendCommandFeedback) {

		auto it = db.Find(player);
		if (!it) return;
		uint64_t xuid = it->xuid;

		if (xuid <= 0) return output.error(
			it->name + "'s team change could not be processed because they are using an offline account");

		std::string teamStatusStr = "Your team number has been ";

		switch (action) {
			case TeamAction::Set: {
				playerTeams[xuid] = teamNumber;
				teamStatusStr += "set to §a" + std::to_string(teamNumber) + "§r";
				break;
			}
			case TeamAction::Reset: {
				playerTeams.erase(xuid);
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

	void execute(CommandOrigin const &origin, CommandOutput &output) {

		auto selectedEntities = selector.results(origin);
		if (selectedEntities.empty()) {
			return output.error("No targets matched selector");
		}

		if (teamNumber < 1) {
			return output.error("Team number must be at least 1");
		}

		bool sendCommandFeedback = (output.type != CommandOutputType::NoFeedback);

		auto& db = Mod::PlayerDatabase::GetInstance();

		for (auto player : selectedEntities) {
			handleTeamAction(player, output, db, sendCommandFeedback);
		}

		int32_t resultCount = selectedEntities.count();
		std::string outputStrStart = "Successfully ";
		switch (action) {
			case TeamAction::Set: {
				outputStrStart += "set the team number to " + std::to_string(teamNumber);
				break;
			}
			case TeamAction::Reset: {
				outputStrStart += "reset the team number";
				break;
			}
			default: break;
		}
		output.success(outputStrStart + " for " + std::to_string(resultCount) + std::string(resultCount == 1 ? " player" : " players"));
	}

	static void setup(CommandRegistry *registry) {
		using namespace commands;
		registry->registerCommand(
			"team", "Sets a player's team.", CommandPermissionLevel::GameMasters, CommandFlagUsage, CommandFlagNone);

		commands::addEnum<TeamAction>(registry, "setTeamAction", {
			{ "set", TeamAction::Set }
		});

		commands::addEnum<TeamAction>(registry, "resetTeamAction", {
			{ "reset", TeamAction::Reset }
		});

		registry->registerOverload<TeamCommand>("team",
			mandatory<CommandParameterDataType::ENUM>(&TeamCommand::action, "set", "setTeamAction"),
			mandatory(&TeamCommand::selector, "player"),
			mandatory(&TeamCommand::teamNumber, "teamNumber")
		);

		registry->registerOverload<TeamCommand>("team",
			mandatory<CommandParameterDataType::ENUM>(&TeamCommand::action, "reset", "resetTeamAction"),
			optional(&TeamCommand::selector, "player")
		);
	}
};

class TeamListCommand : public Command {
public:

	void execute(CommandOrigin const &origin, CommandOutput &output) {

		auto& db = Mod::PlayerDatabase::GetInstance();
		std::unordered_map<int32_t, std::vector<uint64_t>> reverseTeamMap;
		std::string listStr;

		for (const auto& pair : playerTeams) {
			if (pair.second > 0) { // don't include team 0
				reverseTeamMap[pair.second].emplace_back(pair.first); // push_back
			}
		}

		for (const auto &thisList : reverseTeamMap) {

			listStr += "\n§bTeam " + std::to_string(thisList.first) + "§r: ";

			for (const auto& thisXuid : thisList.second) {

				auto it = db.Find(thisXuid);
				if (it) {
					listStr += it->name + ", ";
				}
				else {
					listStr += db.FindOffline(thisXuid)->name + " §c[offline]§r, ";
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
			listStr = "§cNo teams exist in the current session§r";
		}

		// to bypass silenced output from sendcommandfeedback gamerule set to false
		if (origin.getOriginType() == CommandOriginType::Player) {
			auto player = (Player*)(origin.getEntity());
			auto output = TextPacket::createTextPacket<TextPacketType::SystemMessage>(listStr);
			return player->sendNetworkPacket(output);
		}
		output.success(listStr);
	}

	static void setup(CommandRegistry *registry) {
		using namespace commands;
		registry->registerCommand(
			"teamlist", "Outputs a list of all player teams.", CommandPermissionLevel::Any, CommandFlagUsage, CommandFlagNone);
		registry->registerOverload<TeamListCommand>("teamlist");
	}
};

void initializeTeamCommands(CommandRegistry *registry) {
	TeamCommand::setup(registry);
	TeamListCommand::setup(registry);
}

void onPlayerChat(Mod::PlayerEntry const &entry, std::string &name, std::string &content, Mod::CallbackToken<std::string> &token) {

	std::string prefix1 = "?teamwhisper ";
	std::string prefix2 = "?tw ";
	bool isUsingTeamChat = false;

	// check if prefix is used then erase it from the chat message
	if (content.compare(0, prefix1.length(), prefix1) == 0) {
		content.erase(0, prefix1.length());
		isUsingTeamChat = true;
	}
	else if (content.compare(0, prefix2.length(), prefix2) == 0) {
		content.erase(0, prefix2.length());
		isUsingTeamChat = true;
	}

	if (isUsingTeamChat) {

		std::string tokenStr = "Overwritten by ?teamwhisper";
		auto& db = Mod::PlayerDatabase::GetInstance();
		std::unordered_map<int32_t, std::vector<uint64_t>> reverseTeamMap;
		int32_t selfTeamNum = playerTeams[entry.xuid];

		if (selfTeamNum <= 0) {
			std::string errorStr = "§cThe whisper could not not be sent because you are not an a team§r";
			auto error = TextPacket::createTextPacket<TextPacketType::SystemMessage>(errorStr);
			entry.player->sendNetworkPacket(error);
			return token(tokenStr);
		}

		std::string whisperPrefix = "§b[Team " + std::to_string(selfTeamNum) + "]§r";

		auto whisperPkt = TextPacket::createTextPacket<TextPacketType::SystemMessage>(
			name, whisperPrefix + " <" + name + "> " + content, std::to_string(entry.xuid));

		for (const auto& pair : playerTeams) {
			if (pair.second > 0) {
				reverseTeamMap[pair.second].emplace_back(pair.first); // emplace_back
			}
		}

		for (const auto& thisXuid : reverseTeamMap[selfTeamNum]) {

			auto it = db.Find(thisXuid);
			if (it) {

				it->player->sendNetworkPacket(whisperPkt);

				if (thisXuid != entry.xuid) { // so the sender doesn't hear the whisper sound effect

					PlaySoundPacket soundPkt("random.orb", it->player->getPos(), 0.375f);
					it->player->sendNetworkPacket(soundPkt);
				}
			}
		}
		token(tokenStr);
	}
}