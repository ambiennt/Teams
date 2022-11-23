#include "../main.h"

void TeamWhisperCommand::execute(CommandOrigin const &origin, CommandOutput &output) {

	output.type = CommandOutputType::Normal;

	if (origin.getOriginType() != CommandOriginType::Player) {
		output.error("This command can only be executed by players");
		return;
	}

	std::string actualMsg = this->msg.getMessage(origin);
	if (actualMsg.length() <= 0) {
		output.error("Whisper messages must be at least 1 character long");
		return;
	}




	auto cmdExecutor = PLAYER_DB.Find((Player*)origin.getEntity());
	if (!cmdExecutor) return;

	auto it1 = TeamUtils::xuidToTeamMap.find(cmdExecutor->xuid);
	if (it1 == TeamUtils::xuidToTeamMap.end()) {
		output.error("The whisper could not not be sent because you are not an a team");
		return;
	}

	int32_t selfTeamNum = it1->second;
	std::string whisperPrefix("§b[Team " + std::to_string(selfTeamNum) + "]§r");

	auto whisperPkt = TextPacket::createTextPacket<TextPacketType::SystemMessage>(
		whisperPrefix + " <" + cmdExecutor->player->mPlayerName + "> " + actualMsg);
	PlaySoundPacket soundPkt;
	soundPkt.mName = std::string("random.orb");
	soundPkt.mVolume = 0.375f;

	for (const auto& currXuid : TeamUtils::teamToXuidMap[selfTeamNum]) {

		auto it2 = PLAYER_DB.Find(currXuid);
		if (it2) {

			it2->player->sendNetworkPacket(whisperPkt);

			if (currXuid != cmdExecutor->xuid) { // so the sender doesn't hear the whisper sound effect
				soundPkt.mPos = it2->player->getBlockPos();
				it2->player->sendNetworkPacket(soundPkt);
			}
		}
	}



	//LOGI("[%s -> Team %d] %s") % cmdExecutor->name % selfTeamNum % actualMsg;
	std::string recipient("(TEAM WHISPER) Team " + std::to_string(selfTeamNum));
	Mod::Chat::logChat(cmdExecutor.value(), actualMsg, &recipient);
}

void TeamWhisperCommand::setup(CommandRegistry *registry) {
	using namespace commands;

	std::string cmdName{"teamwhisper"};

	// order is important! register the command, alias, then overload
	registry->registerCommand(cmdName, "Sends a message to all players on your team.",
		CommandPermissionLevel::Any, CommandFlagUsage, CommandFlagNone);
	registry->registerAlias(cmdName, "tw");

	registry->registerOverload<TeamWhisperCommand>(cmdName,
		mandatory(&TeamWhisperCommand::msg, "content")
	);
}