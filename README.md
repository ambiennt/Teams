# Teams
# Set, list, and communicate with player teams in BDS + ElementZero 1.16.20

## Usage:
### /team \<teamAction: set\> \<player: target\> \<teamNumber: int\>
### /team \<teamAction: reset\> [player: target]

### /teamlist
### /tl

### /teamwhisper \<content: message\>
### /tw \<content: message\>

### /whisper \<player: string\> \<content: message\>
### /w \<player: string\> \<content: message\>

### /reply \<content: message\>
### /r \<content: message\>

### /teamchat \<bool: toggle\>
### /tc \<bool: toggle\>

With the /team command, you can either set or clear a player's team. When players are on the same team, they cannot damage one another, and have access to the /teamwhisper and /tw commands to communicate with one another.

With the /teamlist command, any player, regardless of team or whether they are on a team, can see a list of all teams and corresponding player names. Offline player names will also show. The /tl command is an alias for /teamlist.

With the /teamwhisper command, players can conveniently send private messages to everyone else on the same team without operators seeing the message in-game (messages will still be logged to the console. The /tw command is an alias for /teamwhisper.

The /whisper command is very similar to /teamwhisper, except you must specify the person you are whispering to. It has the same level of privacy as /teamwhisper; operators cannot see the message in chat but the messages are viewable in the console. The /w command is an alias for /whisper.

The /reply command automatically deduces the target and sends a message to the last player who whispered to you. It has the same functionality as the /whisper command. The /r command is an alias for /reply.

The /teamchat command is a convenience tool to toggle between team and global chat. This command is only applicable for players who are currently on a team. The /tc command is an alias for /teamchat.

Something important to note is that the team lists are network bound, meaning that the team configurations will not save on restart. I don't currently have a use for persistent team configurations so setting teams manually for each server instance suffices for me. However, you may set player teams along with scoreboard/tag commands and run /team commands on player entry with a script to implement your own persistent system.

Other mod dependencies: CommandSupport