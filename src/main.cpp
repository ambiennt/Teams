#include "main.h"
#include <dllentry.h>

std::unordered_map<uint64_t, int32_t> playerTeams;

void dllenter() {}
void dllexit() {}
void PreInit() {
	Mod::CommandSupport::GetInstance().AddListener(SIG("loaded"), initializeTeamCommands);
	Mod::Chat::GetInstance().AddListener(SIG("chat"), {Mod::RecursiveEventHandlerAdaptor(onPlayerChat)});
}
void PostInit() {}

// for child entity sources like projectiles
THook(bool, "?isInvulnerableTo@Player@@UEBA_NAEBVActorDamageSource@@@Z", Player *player, ActorDamageSource &source) {

	if (source.getEntityType() == ActorType::Player_0) {

		auto attacker = (Player*)(LocateService<Level>()->fetchEntity(source.getEntityUniqueID(), false));
		if (!attacker) return original(player, source);
		auto& db = Mod::PlayerDatabase::GetInstance();

		int32_t selfTeamNum = playerTeams[db.Find(player)->xuid];
		int32_t attackerTeamNum = playerTeams[db.Find(attacker)->xuid];

		if ((selfTeamNum > 0) && (selfTeamNum == attackerTeamNum)) return true;
	}
	return original(player, source);
}

// for normal melee attacks
THook(void, "?attack@GameMode@@UEAA_NAEAVActor@@@Z", GameMode *mode, Actor &actor) {

	if (actor.getEntityTypeId() == ActorType::Player_0) {

		auto player = mode->mPlayer;
		auto attacker = (Player*)(&actor);
		auto& db = Mod::PlayerDatabase::GetInstance();

		int32_t selfTeamNum = playerTeams[db.Find(player)->xuid];
		int32_t attackerTeamNum = playerTeams[db.Find(attacker)->xuid];

		if ((selfTeamNum > 0) && (selfTeamNum == attackerTeamNum)) return;
	}
	original(mode, actor);
}