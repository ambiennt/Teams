#include "main.h"
#include <dllentry.h>

void dllenter() {}
void dllexit() {}
void PreInit() {
	Mod::CommandSupport::GetInstance().AddListener(SIG("loaded"), TeamUtils::initializeTeamCommands);
	TeamUtils::updateWhisperCommandSoftEnum();
}
void PostInit() {}







namespace TeamUtils {

std::unordered_map<uint64_t, int32_t> playerTeams;

void initializeTeamCommands(CommandRegistry *registry) {
	TeamCommand::setup(registry);
	TeamListCommand::setup(registry);
	TeamWhisperCommand::setup(registry);
	WhisperCommand::setup(registry);
	ReplyCommand::setup(registry);
}

bool isOnSameTeam(uint64_t thisXuid, uint64_t thatXuid) {

	auto it1 = playerTeams.find(thisXuid);
	if (it1 == playerTeams.end()) return false;

	auto it2 = playerTeams.find(thatXuid);
	if (it2 == playerTeams.end()) return false;

	return (it1->second == it2->second);
}

void updateWhisperCommandSoftEnum() {

	Mod::PlayerDatabase::GetInstance().AddListener(SIG("joined"), [](Mod::PlayerEntry const &entry) {
		LocateService<CommandRegistry>()->addSoftEnumValues(
			std::string(WhisperCommand::WHISPER_COMMAND_SOFTENUM_NAME), {entry.player->mPlayerName});
	});

	Mod::PlayerDatabase::GetInstance().AddListener(SIG("left"), [](Mod::PlayerEntry const &entry) {
		LocateService<CommandRegistry>()->removeSoftEnumValues(
			std::string(WhisperCommand::WHISPER_COMMAND_SOFTENUM_NAME), {entry.player->mPlayerName});
	});

}

} // namespace TeamUtils







// for child entity sources like projectiles
TInstanceHook(bool, "?isInvulnerableTo@Player@@UEBA_NAEBVActorDamageSource@@@Z", Player, ActorDamageSource &source) {

	if (source.getEntityType() == ActorType::Player_0) {

		auto attacker = (Player*)(this->mLevel->fetchEntity(source.getEntityUniqueID(), false));
		if (!attacker) return original(this, source);

		uint64_t targetXuid = PLAYER_DB.Find(this)->xuid;
		uint64_t attackerXuid = PLAYER_DB.Find(attacker)->xuid;

		if (TeamUtils::isOnSameTeam(targetXuid, attackerXuid)) return true;
	}
	return original(this, source);
}

// for normal melee attacks
TInstanceHook(void, "?attack@GameMode@@UEAA_NAEAVActor@@@Z", GameMode, Actor &target) {

	if (target.isInstanceOfPlayer()) {

		uint64_t attackerXuid = PLAYER_DB.Find(this->mPlayer)->xuid;
		uint64_t targetXuid = PLAYER_DB.Find((Player*)&target)->xuid;

		if (TeamUtils::isOnSameTeam(attackerXuid, targetXuid)) return;
	}
	original(this, target);
}