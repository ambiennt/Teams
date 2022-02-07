#pragma once

#include <base/log.h>
#include <base/playerdb.h>
#include <hook.h>
#include <Level/Level.h>
#include <Level/GameRules.h>
#include <Level/GameRulesIndex.h>
#include <Actor/Player.h>
#include <Actor/GameMode.h>
#include <Actor/ActorType.h>
#include <Actor/ActorDamageSource.h>
#include <Packet/TextPacket.h>
#include <Packet/PlaySoundPacket.h>
#include <mods/CommandSupport.h>
#include <mods/ChatAPI.h>

DEF_LOGGER("Teams");

void initializeTeamCommands(CommandRegistry *registry);
void onPlayerChat(Mod::PlayerEntry const &entry, std::string &name, std::string &content, Mod::CallbackToken<std::string> &token);

extern std::unordered_map<uint64_t, int32_t> playerTeams; // xuid and team number

inline bool isOnSameTeam(uint64_t thisXuid, uint64_t thatXuid) {

	auto it1 = playerTeams.find(thisXuid);
	if (it1 == playerTeams.end()) return false;

	auto it2 = playerTeams.find(thatXuid);
	if (it2 == playerTeams.end()) return false;

	return (it1->second == it2->second);
}