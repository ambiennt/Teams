#pragma once

#include <base/log.h>
#include <base/playerdb.h>
#include <hook.h>
#include <base/ezplayer.h>
#include <Level/Level.h>
#include <Actor/Player.h>
#include <Actor/GameMode.h>
#include <Actor/ActorType.h>
#include <Actor/ActorDamageSource.h>
#include <Packet/TextPacket.h>
#include <Packet/PlaySoundPacket.h>
#include <mods/CommandSupport.h>
#include <mods/ChatAPI.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <optional>

class TeamCommand : public Command {
public:
	enum class TeamAction {
		SET   = 0,
		RESET = 1,
	};
private:
	TeamAction action;
	CommandSelector<Player> selector;
	int32_t teamNumber;
public:

	TeamCommand() : action(TeamAction::SET), teamNumber(0) {
		this->selector.setIncludeDeadPlayers(true);
	}

	bool handleTeamAction(Player &player, CommandOutput &output, bool sendCommandFeedback);
	virtual void execute(CommandOrigin const &origin, CommandOutput &output) override;
	static void setup(CommandRegistry *registry);
};

class TeamListCommand : public Command {
public:

	virtual void execute(CommandOrigin const &origin, CommandOutput &output) override;
	static void setup(CommandRegistry *registry);
};

class TeamWhisperCommand : public Command {
	CommandMessage msg;
public:

	virtual void execute(CommandOrigin const &origin, CommandOutput &output) override;
	static void setup(CommandRegistry *registry);
};

class WhisperCommand : public Command {
	std::string specificName;
	CommandMessage msg;
public:

	static inline constexpr const char *WHISPER_COMMAND_SOFTENUM_NAME{"PlayerNames"};

	virtual void execute(CommandOrigin const &origin, CommandOutput &output) override;
	static void setup(CommandRegistry *registry);
};

class ReplyCommand : public Command {
	CommandMessage msg;
public:

	virtual void execute(CommandOrigin const &origin, CommandOutput &output) override;
	static void setup(CommandRegistry *registry);
};

class TeamChatCommand : public Command {
	bool toggle;
public:
	TeamChatCommand() : toggle(true) {}

	virtual void execute(CommandOrigin const &origin, CommandOutput &output) override;
	static void setup(CommandRegistry *registry);
};







namespace TeamUtils {

// yea i really need to make a single dll for all my uhc stuff because this code is redundant af...
inline constexpr const char* HEALTH_GLYPH      = "\ue1fe"; // , glyph 0xE1FE
inline constexpr const char* ABSORPTION_GLYPH  = "\ue1ff"; // , glyph 0xE1FF

inline std::unordered_map<uint64_t, int32_t> xuidToTeamMap{}; // xuid to team number
inline std::map<int32_t, std::unordered_set<uint64_t>> teamToXuidMap{}; // team number to list of xuids
inline std::unordered_set<uint64_t> xuidsInTeamChat{}; // xuids who are in the team chat state (as opposed to global chat)

bool stringEndsWith(const std::string& str, std::string_view suffix); // we love being on c++17!
void initializeTeamCommands(CommandRegistry *registry);
bool isOnSameTeam(uint64_t thisXuid, uint64_t thatXuid);
std::optional<int32_t> getTeamNumber(Player &player);
void updateWhisperCommandSoftEnum();
std::string getFormattedHealthAndPosString(Player &player);
void trySendTeamWhisper(const Mod::PlayerEntry &cmdExecutor, int32_t selfTeamNum, const std::string &chatMsg);
void onPlayerChat(const Mod::PlayerEntry &entry, std::string &name, std::string &chatMsg, Mod::CallbackToken<std::string> &token);
inline bool isInTeamChat(uint64_t xuid) { return TeamUtils::xuidsInTeamChat.count(xuid) > 0; }

} // namespace TeamUtils






#define PLAYER_DB Mod::PlayerDatabase::GetInstance()






DEF_LOGGER("Teams");