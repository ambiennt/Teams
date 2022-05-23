#pragma once

#include <base/log.h>
#include <base/playerdb.h>
#include <hook.h>
#include <Level/Level.h>
#include <Level/GameRules.h>
#include <Actor/Player.h>
#include <Actor/GameMode.h>
#include <Actor/ActorType.h>
#include <Actor/ActorDamageSource.h>
#include <Packet/TextPacket.h>
#include <Packet/PlaySoundPacket.h>
#include <mods/CommandSupport.h>

class TeamCommand : public Command {
public:

	enum class TeamAction : int8_t {
		SET   = 0,
		RESET = 1,
	};

	TeamAction action;
	CommandSelector<Player> selector;
	int32_t teamNumber;

	TeamCommand() : action(TeamAction::SET), teamNumber(0) {
		this->selector.setIncludeDeadPlayers(true);
	}

	void handleTeamAction(Player *player, CommandOutput &output, bool sendCommandFeedback);
	virtual void execute(CommandOrigin const &origin, CommandOutput &output) override;
	static void setup(CommandRegistry *registry);
};

class TeamListCommand : public Command {
public:

	virtual void execute(CommandOrigin const &origin, CommandOutput &output) override;
	static void setup(CommandRegistry *registry);
};

class TeamWhisperCommand : public Command {
public:
	CommandMessage msg;

	virtual void execute(CommandOrigin const &origin, CommandOutput &output) override;
	static void setup(CommandRegistry *registry);
};

class WhisperCommand : public Command {
public:
	std::string specificName;
	CommandMessage msg;

	static constexpr const char* WHISPER_COMMAND_SOFTENUM_NAME = "PlayerNames";

	virtual void execute(CommandOrigin const &origin, CommandOutput &output) override;
	static void setup(CommandRegistry *registry);
};

class ReplyCommand : public Command {
public:
	CommandMessage msg;

	virtual void execute(CommandOrigin const &origin, CommandOutput &output) override;
	static void setup(CommandRegistry *registry);
};








namespace TeamUtils {

extern std::unordered_map<uint64_t, int32_t> playerTeams; // xuid and team number
extern Mod::PlayerDatabase& db;

void initializeTeamCommands(CommandRegistry *registry);
bool isOnSameTeam(uint64_t thisXuid, uint64_t thatXuid);
void updateWhisperCommandSoftEnum();

} // namespace TeamUtils








DEF_LOGGER("Teams");