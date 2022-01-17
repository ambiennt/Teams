# Teams
### Set, list, and communicate with player teams in BDS + ElementZero 1.16.20

## Usage:
# /team \<action: teamAction\> [player: target] [teamNumber: int]
# /teamlist
# ?teamwhisper [message: string]
# ?tw [message: string]

With the /team command, you can either set or clear a player's team. When players are on the same team, they cannot damage one another, and have access to the teamwhisper commands to communicate with one another.

With the /teamlist command, any player, regardless of team or whether they are on a team, can see a list of all teams and corresponding player names. Offline player names will also show.

The ?tw prefix is just an alias for ?teamwhisper, they have the same functionality. With the ?teamwhisper command, players on the same team can send private m essages to one anohter without operators seeing in-game (messages will still be logged to chat.db).

Other mod dependencies: ChatAPI, CommandSupport