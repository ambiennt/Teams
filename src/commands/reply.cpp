#include "../main.h"

void ReplyCommand::execute(CommandOrigin const &origin, CommandOutput &output) {

	auto cmdExecutor = PLAYER_DB.Find((Player*)origin.getEntity());
	if (!cmdExecutor) return;
	auto lastWhisperMessager = PLAYER_DB.Find(cmdExecutor->player->mEZPlayer->mLastWhisperMessagerXuid);

	if (!lastWhisperMessager || (lastWhisperMessager->xuid == 0)) {
		output.error(
			"Reply failed! Either you do not have anyone to reply to, the last person who whispered to you is offline, or they are using an offline account");
          return;
	}

	std::string actualMsg = this->msg.getMessage(origin);
	if (actualMsg.length() <= 0) {
		output.error("Reply messages must be at least 1 character long");
		return;
	}

	auto toSelfReplyPkt = TextPacket::createTextPacket<TextPacketType::SystemMessage>(
		"§e(To §a" + lastWhisperMessager->name + "§e): " + actualMsg);
	auto toTargetReplyPkt = TextPacket::createTextPacket<TextPacketType::SystemMessage>(
		"§e(From §a" + cmdExecutor->name + "§e): " + actualMsg);
	PlaySoundPacket toTargetSoundPkt(std::string("random.orb"), lastWhisperMessager->player->getBlockPos(), 0.375f);

	lastWhisperMessager->player->sendNetworkPacket(toTargetReplyPkt);
	lastWhisperMessager->player->sendNetworkPacket(toTargetSoundPkt);
	cmdExecutor->player->sendNetworkPacket(toSelfReplyPkt);




	//LOGI("[%s -> %s] %s") % cmdExecutor->name % lastWhisperMessager->name % actualMsg;
	Mod::Chat::logChat(cmdExecutor.value(), actualMsg, &lastWhisperMessager->name);
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