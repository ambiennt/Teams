#include "../main.h"

void TeamWhisperCommand::execute(CommandOrigin const &origin, CommandOutput &output) {

	output.type = CommandOutputType::Normal;

	if (origin.getOriginType() != CommandOriginType::Player) {
		return output.error("This command can only be executed by players");
	}

	std::string actualMsg = this->msg.getMessage(origin);
	if (actualMsg.length() <= 0) {
		return output.error("Whisper messages must be at least 1 character long");
	}




	auto cmdExecutor = TeamUtils::db.Find((Player*)origin.getEntity());
	if (!cmdExecutor) return;

	std::unordered_map<int32_t, std::vector<uint64_t>> reverseTeamMap;

	auto it1 = TeamUtils::playerTeams.find(cmdExecutor->xuid);
	if (it1 == TeamUtils::playerTeams.end()) {
		return output.error("The whisper could not not be sent because you are not an a team");
	}

	int32_t selfTeamNum = it1->second;
	std::string whisperPrefix("§b[Team " + std::to_string(selfTeamNum) + "]§r");

	auto whisperPkt = TextPacket::createTextPacket<TextPacketType::SystemMessage>(
		whisperPrefix + " <" + cmdExecutor->player->mPlayerName + "> " + actualMsg);
	PlaySoundPacket soundPkt;
	soundPkt.mName = std::string("random.orb");
	soundPkt.mVolume = 0.375f;

	for (const auto& pair : TeamUtils::playerTeams) {
		reverseTeamMap[pair.second].push_back(pair.first);
	}

	for (const auto& thisXuid : reverseTeamMap[selfTeamNum]) {

		auto it2 = TeamUtils::db.Find(thisXuid);
		if (it2) {

			it2->player->sendNetworkPacket(whisperPkt);

			if (thisXuid != cmdExecutor->xuid) { // so the sender doesn't hear the whisper sound effect
				soundPkt.mPos = it2->player->getBlockPos();
				it2->player->sendNetworkPacket(soundPkt);
			}
		}
	}




	LOGI("[%s -> Team %d] %s") % cmdExecutor->name % selfTeamNum % actualMsg;
}

void TeamWhisperCommand::setup(CommandRegistry *registry) {
	using namespace commands;

	std::string cmdName("teamwhisper");

	// order is important! register the command, alias, then overload
	registry->registerCommand(cmdName, "Sends a message to all players on your team.",
		CommandPermissionLevel::Any, CommandFlagUsage, CommandFlagNone);
	registry->registerAlias(cmdName, "tw");

	registry->registerOverload<TeamWhisperCommand>(cmdName,
		mandatory(&TeamWhisperCommand::msg, "content")
	);
}