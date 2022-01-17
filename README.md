# Teams
# Set, list, and communicate with player teams in BDS + ElementZero 1.16.20

## Usage:
### /team \<action: teamAction\> [player: target] [teamNumber: int]
### /teamlist
### ?teamwhisper [message: string]
### ?tw [message: string]

With the /team command, you can either set or clear a player's team. When players are on the same team, they cannot damage one another, and have access to the ?teamwhisper and ?tw commands to communicate with one another.

With the /teamlist command, any player, regardless of team or whether they are on a team, can see a list of all teams and corresponding player names. Offline player names will also show.

The ?tw prefix is just an alias for ?teamwhisper, they have the same functionality. With the ?teamwhisper command, players can conveniently send private messages to everyone else on the same team without operators seeing them in-game (messages will still be logged to chat.db).

Something important to note is that the team lists are network bound, meaning that the team configurations will not save on restart. I don't currently have a use for persistent team configurations so setting teams manually for each server instance suffices for me. However, you may set player teams along with scoreboard/tag commands and run /team commands on player entry with a script to implement your own persistent system.

Other mod dependencies: ChatAPI, CommandSupport