#include "main.h"
#include <dllentry.h>

void dllenter() {}
void dllexit() {}
void PreInit() {
	Mod::CommandSupport::GetInstance().AddListener(SIG("loaded"), TeamUtils::initializeTeamCommands);
	Mod::Chat::GetInstance().AddListener(SIG("chat"), {Mod::RecursiveEventHandlerAdaptor(TeamUtils::onPlayerChat)});
	TeamUtils::updateWhisperCommandSoftEnum();
}
void PostInit() {}







bool TeamUtils::stringEndsWith(const std::string& str, std::string_view suffix) {
	if (suffix.length() > str.length()) return false;
	return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
};

void TeamUtils::initializeTeamCommands(CommandRegistry *registry) {
	TeamCommand::setup(registry);
	TeamListCommand::setup(registry);
	TeamWhisperCommand::setup(registry);
	WhisperCommand::setup(registry);
	ReplyCommand::setup(registry);
	TeamChatCommand::setup(registry);
}

bool TeamUtils::isOnSameTeam(uint64_t thisXuid, uint64_t thatXuid) {

	auto it1 = xuidToTeamMap.find(thisXuid);
	if (it1 == xuidToTeamMap.end()) return false;

	auto it2 = xuidToTeamMap.find(thatXuid);
	if (it2 == xuidToTeamMap.end()) return false;

	return (it1->second == it2->second);
}

std::optional<int32_t> TeamUtils::getTeamNumber(Player &player) {
	auto dbIt = PLAYER_DB.Find(&player); // why is this not const...
	if (dbIt) {
		auto mapIt = TeamUtils::xuidToTeamMap.find(dbIt->xuid);
		if (mapIt != TeamUtils::xuidToTeamMap.end()) {
			return mapIt->second;
		}
	}
	return {};
}

void TeamUtils::updateWhisperCommandSoftEnum() {

	Mod::PlayerDatabase::GetInstance().AddListener(SIG("joined"), [](Mod::PlayerEntry const &entry) {
		LocateService<CommandRegistry>()->addSoftEnumValues(
			WhisperCommand::WHISPER_COMMAND_SOFTENUM_NAME, {entry.name});
	});

	Mod::PlayerDatabase::GetInstance().AddListener(SIG("left"), [](Mod::PlayerEntry const &entry) {
		LocateService<CommandRegistry>()->removeSoftEnumValues(
			WhisperCommand::WHISPER_COMMAND_SOFTENUM_NAME, {entry.name});
	});

}

std::string TeamUtils::getFormattedHealthAndPosString(Player &player) {
	int32_t currHealth = player.getHealthAsInt();
	int32_t currAbsorption = player.getAbsorptionAsInt();

	auto pos = player.getBlockPosGrounded();
	std::string nametag{player.mPlayerName + " §7[" + std::to_string(pos.x) + ", " +
		std::to_string(pos.y) + ", " + std::to_string(pos.z) + "] [" + std::to_string(currHealth)};

	nametag += HEALTH_GLYPH;
	if (currAbsorption > 0) {
		nametag += " " + std::to_string(currAbsorption) + ABSORPTION_GLYPH;
	}
	nametag += "]§r";
	return nametag;
}

void TeamUtils::trySendTeamWhisper(const Mod::PlayerEntry &cmdExecutor, int32_t selfTeamNum, const std::string &chatMsg) {

	std::string whisperPrefix = "§b[Team " + std::to_string(selfTeamNum) + "]§r";
	auto whisperPkt = TextPacket::createTextPacket<TextPacketType::SystemMessage>(whisperPrefix + " <" + cmdExecutor.name + "> " + chatMsg);
	PlaySoundPacket soundPkt{};
	soundPkt.mName = "random.orb";
	soundPkt.mVolume = 0.375f;

	for (const auto& currXuid : TeamUtils::teamToXuidMap[selfTeamNum]) {
		auto it2 = PLAYER_DB.Find(currXuid);
		if (it2) {
			it2->player->sendNetworkPacket(whisperPkt);
			if (currXuid != cmdExecutor.xuid) { // so the sender doesn't hear the whisper sound effect
				soundPkt.mPos = it2->player->getBlockPos() * 8; // for some reason it needs to be * 8 idk
				it2->player->sendNetworkPacket(soundPkt);
			}
		}
	}

	//LOGI("[%s -> Team %d] %s") % cmdExecutor.name % selfTeamNum % chatMsg;
	std::string recipient = "(TEAM WHISPER) Team " + std::to_string(selfTeamNum);
	Mod::Chat::logChat(cmdExecutor, chatMsg, &recipient);
}

void TeamUtils::onPlayerChat(const Mod::PlayerEntry &entry, std::string &name, std::string &chatMsg, Mod::CallbackToken<std::string> &token) {
	auto chatSender = PLAYER_DB.Find(entry.player);
	if (chatSender) {
		if (TeamUtils::isInTeamChat(chatSender->xuid)) {
			auto teamIt = TeamUtils::xuidToTeamMap.find(chatSender->xuid);
			if (teamIt != TeamUtils::xuidToTeamMap.end()) { // is on a team, should always be true if xuid is in xuidsInTeamChat map, but doesnt hurt to check
				int32_t selfTeamNum = teamIt->second;
				TeamUtils::trySendTeamWhisper(*chatSender, selfTeamNum, chatMsg);
				token("Cancelled by team chat");
			}
		}
	}
}






// for child entity sources like projectiles
TInstanceHook(bool, "?isInvulnerableTo@Player@@UEBA_NAEBVActorDamageSource@@@Z", Player, ActorDamageSource &source) {
	if (source.getEntityType() == ActorType::Player_0) {

		auto attacker = (Player*)(this->mLevel->fetchEntity(source.getEntityUniqueID(), false));
		if (!attacker) {
			return original(this, source);
		}

		auto targetEntry = PLAYER_DB.Find(this);
		if (!targetEntry) { // should never be nullopt tbh but just to be safe
			return original(this, source);
		}

		auto attackerEntry = PLAYER_DB.Find(attacker);
		if (!attackerEntry) { // should never be nullopt tbh but just to be safe
			return original(this, source);
		}

		if (TeamUtils::isOnSameTeam(targetEntry->xuid, attackerEntry->xuid)) {
			return true;
		}
	}
	return original(this, source);
}

// for normal melee attacks
TInstanceHook(void, "?attack@GameMode@@UEAA_NAEAVActor@@@Z", GameMode, Actor &target) {
	if (target.isInstanceOfPlayer()) {

		auto attackerEntry = PLAYER_DB.Find(this->mPlayer);
		if (!attackerEntry) { // should never be nullopt tbh but just to be safe
			return original(this, target);
		}

		auto targetEntry = PLAYER_DB.Find((Player*)&target);
		if (!targetEntry) { // should never be nullopt tbh but just to be safe
			return original(this, target);
		}

		if (TeamUtils::isOnSameTeam(attackerEntry->xuid, targetEntry->xuid)) {
			return;
		}
	}
	original(this, target);
}