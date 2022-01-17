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