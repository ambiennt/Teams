#include "../main.h"

void TeamChatCommand::execute(CommandOrigin const &origin, CommandOutput &output) {
    output.type = CommandOutputType::Normal;

	if (origin.getOriginType() != CommandOriginType::Player) {
		output.error("This command can only be executed by players");
		return;
	}

    auto cmdExecutor = PLAYER_DB.Find(reinterpret_cast<Player*>(origin.getEntity()));
    if (!cmdExecutor) return;

    auto mapIt = TeamUtils::xuidToTeamMap.find(cmdExecutor->xuid);
    bool isOnATeam = mapIt != TeamUtils::xuidToTeamMap.end();

    if (!isOnATeam) {
        output.error("You are not currently on a team");
        return;
    }

    if (this->toggle) {
        TeamUtils::xuidsInTeamChat.insert(cmdExecutor->xuid);
        output.success("Successfully toggled to §ateam§r chat");
    }
    else {
        TeamUtils::xuidsInTeamChat.erase(cmdExecutor->xuid); // its ok if the xuid wasn't in the map prior, its easier to just call erase regardless
        output.success("Successfully toggled to §aglobal§r chat");
    }
}

void TeamChatCommand::setup(CommandRegistry *registry) {
	using namespace commands;

	std::string cmdName{"teamchat"};

	registry->registerCommand(cmdName, "Toggles your team or global chat status.",
		CommandPermissionLevel::Any, CommandFlagUsage, CommandFlagNone);

	registry->registerAlias(cmdName, "tc");

	registry->registerOverload<TeamChatCommand>(cmdName,
		optional(&TeamChatCommand::toggle, "toggle")
	);
}