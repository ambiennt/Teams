#include "../main.h"

void WhisperCommand::execute(CommandOrigin const &origin, CommandOutput &output) {

	output.type = CommandOutputType::Normal;

	if (origin.getOriginType() != CommandOriginType::Player) {
		output.error("This command can only be executed by players");
		return;
	}

	auto cmdExecutor = PLAYER_DB.Find((Player*)origin.getEntity());
	if (!cmdExecutor) return;

	if (this->specificName == cmdExecutor->name) {
		output.error("You cannot send a whisper to yourself");
		return;
	}

	auto it = PLAYER_DB.Find(this->specificName);
	if (!it) {
		output.error("No player was found with the name: \"" + this->specificName + "\"");
		return;
	}
	auto& target = *it->player;

	std::string actualMsg = this->msg.getMessage(origin);
	if (actualMsg.length() <= 0) {
		output.error("Whisper messages must be at least 1 character long");
		return;
	}




	auto toSelfWhisperPkt = TextPacket::createTextPacket<TextPacketType::SystemMessage>(
		"§e(To §a" + target.mPlayerName + "§e): " + actualMsg);
	auto toTargetWhisperPkt = TextPacket::createTextPacket<TextPacketType::SystemMessage>(
		"§e(From §a" + cmdExecutor->name + "§e): " + actualMsg);
	PlaySoundPacket toTargetSoundPkt(std::string("random.orb"), target.getBlockPos(), 0.375f);

	if (cmdExecutor->xuid != 0) {
		target.mEZPlayer->mLastWhisperMessagerXuid = cmdExecutor->xuid;
	}

	target.sendNetworkPacket(toTargetWhisperPkt);
	target.sendNetworkPacket(toTargetSoundPkt);
	cmdExecutor->player->sendNetworkPacket(toSelfWhisperPkt);




	//LOGI("[%s -> %s] %s") % cmdExecutor->name % target.mPlayerName % actualMsg;
	Mod::Chat::logChat(cmdExecutor.value(), actualMsg, &target.mPlayerName);
}

void WhisperCommand::setup(CommandRegistry *registry) {
	using namespace commands;

	std::string cmdName{"whisper"};

	registry->addSoftEnum(WHISPER_COMMAND_SOFTENUM_NAME, {});

	registry->registerCommand(cmdName, "Sends a private message to a specific player.",
		CommandPermissionLevel::Any, CommandFlagUsage, CommandFlagNone);
	registry->registerAlias(cmdName, "w");

	registry->registerOverload<WhisperCommand>(cmdName,
		CommandParameterData(
	 		Mod::CommandSupport::GetParameterTypeId<std::string>(),
	  		CommandRegistry::getParseFn<std::string>(),
			"player",
	  		CommandParameterDataType::SOFTENUM,
	  		WHISPER_COMMAND_SOFTENUM_NAME,
	  		getOffset(&WhisperCommand::specificName),
	  		false,
	  		-1
 		),
		mandatory(&WhisperCommand::msg, "content")
	);
}