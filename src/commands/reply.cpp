#include "../main.h"

void ReplyCommand::execute(CommandOrigin const &origin, CommandOutput &output) {
	output.type = CommandOutputType::Normal;

	if (origin.getOriginType() != CommandOriginType::Player) {
		output.error("This command can only be executed by players");
		return;
	}

	auto cmdExecutor = PLAYER_DB.Find(reinterpret_cast<Player*>(origin.getEntity()));
	if (!cmdExecutor) return;

	auto lastWhisperMsger = PLAYER_DB.Find(cmdExecutor->player->mEZPlayer->mLastWhisperMessagerXuid);
	if (!lastWhisperMsger || (lastWhisperMsger->xuid == 0)) {
		output.error(
			"Reply failed! Either you do not have anyone to reply to, the last person who whispered to you is offline, or they are using an offline account");
		  return;
	}

	std::string actualMsg = this->msg.getMessage(origin);
	if (actualMsg.empty()) {
		output.error("Reply messages must be at least 1 character long");
		return;
	}





	auto toSelfReplyPkt = TextPacket::createTextPacket<TextPacketType::SystemMessage>(
		"§e(To §a" + lastWhisperMsger->name + "§e): " + actualMsg);
	auto toTargetReplyPkt = TextPacket::createTextPacket<TextPacketType::SystemMessage>(
		"§e(From §a" + cmdExecutor->name + "§e): " + actualMsg);
	PlaySoundPacket toTargetSoundPkt("random.orb", lastWhisperMsger->player->getBlockPos(), 0.375f);

	lastWhisperMsger->player->sendNetworkPacket(toTargetReplyPkt);
	lastWhisperMsger->player->sendNetworkPacket(toTargetSoundPkt);
	cmdExecutor->player->sendNetworkPacket(toSelfReplyPkt);

	//LOGI("[%s -> %s] %s") % cmdExecutor->name % lastWhisperMsger->name % actualMsg;
	Mod::Chat::logChat(*cmdExecutor, actualMsg, &(lastWhisperMsger->name));
}

void ReplyCommand::setup(CommandRegistry *registry) {
	using namespace commands;

	std::string cmdName{"reply"};

	registry->registerCommand(cmdName, "Sends a reply message to the last player who whispered to you.",
		CommandPermissionLevel::Any, CommandFlagUsage, CommandFlagNone);
	registry->registerAlias(cmdName, "r");

	registry->registerOverload<ReplyCommand>(cmdName,
		mandatory(&ReplyCommand::msg, "content")
	);
}