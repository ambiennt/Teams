#include "../main.h"

void TeamWhisperCommand::execute(CommandOrigin const &origin, CommandOutput &output) {
	output.type = CommandOutputType::Normal;

	if (origin.getOriginType() != CommandOriginType::Player) {
		output.error("This command can only be executed by players");
		return;
	}

	std::string chatMsg = this->msg.getMessage(origin);
	if (chatMsg.empty()) {
		output.error("Whisper messages must be at least 1 character long");
		return;
	}

	auto cmdExecutor = PLAYER_DB.Find(reinterpret_cast<Player*>(origin.getEntity()));
	if (!cmdExecutor) return;

	auto it = TeamUtils::xuidToTeamMap.find(cmdExecutor->xuid);
	if (it == TeamUtils::xuidToTeamMap.end()) {
		output.error("The whisper could not not be sent because you are not an a team");
		return;
	}

	int32_t selfTeamNum = it->second;
	TeamUtils::trySendTeamWhisper(*cmdExecutor, selfTeamNum, chatMsg);
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