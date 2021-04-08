#if defined _DCP_included
	#endinput
#endif
#define _DCP_included

#define MAX_FUNC_NAME (32)
#define DCMD:%1(%2)          \
			forward DCP_%1(%2); \
			public DCP_%1(%2)

#if !defined discordPrefix
    #define discordPrefix '!'
#endif

#if !defined DCP_MAX_COMMANDS
	#define DCP_MAX_COMMANDS 				100
#endif

#if defined OnDiscordCommandReceived
	forward OnDiscordCommandReceived(DCC_Channel:channel,DCC_User:author, DCC_Message:message, cmdid, const arguments[]);
#endif
#if defined OnDiscordCommandPerformed
	forward OnDiscordCommandPerformed(DCC_Channel:channel,DCC_User:author, DCC_Message:message, cmdid, const arguments[], success);
#endif
#if defined OnDiscordMessageCreated
	forward OnDiscordMessageCreated(DCC_Message:message);
#endif

#define DISCMD_MODE_DEFAULT 				0
#define DISCMD_MODE_HELP 					1

#define INVALID_DISCORD_COMMAND				-1

#include <YSI_Storage\y_amx>


#define DCP_MAX_MAP_COLLISIONS				10

enum _:e_DCP_COMMAND_DATA {
	e_DCP_COMMAND_DATA_NAME[MAX_FUNC_NAME],
	bool:e_DCP_COMMAND_DATA_DISABLED
}

enum _:e_DCP_SUCCESS {
	e_DCP_SUCCESS_OK = 1,
	e_DCP_SUCCESS_DISABLED,
	e_DCP_SUCCESS_ROLE_DENIED,
	e_DCP_SUCCESS_CHANNEL_DENIED,
	e_DCP_SUCCESS_INVALID
}

new DCP_CommandInfo[DCP_MAX_COMMANDS][e_DCP_COMMAND_DATA];
new DCP_CommandCollisionsCount[DCP_MAX_COMMANDS];
new DCP_CommandIds[DCP_MAX_COMMANDS][DCP_MAX_MAP_COLLISIONS];
new DCP_CommandCount;

#if !defined DCP_MAX_ALLOWED_ROLES
	#define DCP_MAX_ALLOWED_ROLES			16
#endif

#if !defined DCP_MAX_ALLOWED_CHANNELS
	#define DCP_MAX_ALLOWED_CHANNELS		16
#endif

#if !defined DCP_MAX_GROUP_NAME
	#define DCP_MAX_GROUP_NAME				32
#endif

#if !defined DCP_MAX_COMMANDS_IN_ROLE
	#define DCP_MAX_GROUP_NAME				32
#endif

new DCC_Channel:DCP_AllowedChannels[DCP_MAX_COMMANDS][DCP_MAX_ALLOWED_CHANNELS];
new DCP_ChannelCount[DCP_MAX_COMMANDS];

stock DCP_AddChannel(cmdid, DCC_Channel:channel) {
	if(DCP_ChannelCount[cmdid] == 0) {
		DCP_AllowedChannels[cmdid][DCP_ChannelCount[cmdid]] = channel;
		DCP_ChannelCount[cmdid]++;
		return 1;
	}

	new index;
	if(DCP_BinarySearch(channel, DCP_AllowedChannels[cmdid], DCP_ChannelCount[cmdid], index) != -1) {
		return 0;
	}

	if(DCP_AllowedChannels[cmdid][index] < channel) {
		DCP_ShiftRigth(DCP_AllowedChannels[cmdid], DCP_ChannelCount[cmdid], index+1);
		DCP_AllowedChannels[cmdid][index+1] = channel;
	} else {
		DCP_ShiftRigth(DCP_AllowedChannels[cmdid], DCP_ChannelCount[cmdid], index);
		DCP_AllowedChannels[cmdid][index+1] = channel;
	}
	DCP_ChannelCount[cmdid]++;
	return 1;
}

stock DCP_RemoveChannel(cmdid, DCC_Channel:channel) {
	new index = DCP_BinarySearch(channel, DCP_AllowedChannels[cmdid], DCP_ChannelCount[cmdid]);
	if(index == -1) {
		return 0;
	}
	DCP_ShiftLeft(DCP_AllowedChannels[cmdid], DCP_ChannelCount[cmdid]--, index);
	return 1;
}

stock bool:DCP_IsChannelAllowed(cmdid, DCC_Channel:channel) {
	return DCP_BinarySearch(channel, DCP_AllowedChannels[cmdid], DCP_ChannelCount[cmdid]) != -1;
}

new DCC_Role:DCP_AllowedRoles[DCP_MAX_COMMANDS][DCP_MAX_ALLOWED_ROLES];
new DCP_RoleCount[DCP_MAX_COMMANDS];

stock DCP_AddRole(cmdid, DCC_Role:role) {
	if(DCP_RoleCount[cmdid] == 0) {
		DCP_AllowedRoles[cmdid][DCP_RoleCount[cmdid]++] = role;
		return 1;
	}

	new index;
	if(DCP_BinarySearch(role, DCP_AllowedRoles[cmdid], DCP_RoleCount[cmdid], index) != -1) {
		return 0;
	}

	if(DCP_AllowedRoles[cmdid][index] < role) {
		DCP_ShiftRigth(DCP_AllowedRoles[cmdid], DCP_RoleCount[cmdid], index+1);
		DCP_AllowedRoles[cmdid][index+1] = role;
	} else {
		DCP_ShiftRigth(DCP_AllowedRoles[cmdid], DCP_RoleCount[cmdid], index);
		DCP_AllowedRoles[cmdid][index] = role;
	}
	DCP_RoleCount[cmdid]++;
	return 1;
}

stock DCP_RemoveRole(cmdid, DCC_Role:role) {
	new index = DCP_BinarySearch(role, DCP_AllowedRoles[cmdid], DCP_RoleCount[cmdid]);
	if(index == -1) {
		return 0;
	}
	DCP_ShiftLeft(DCP_AllowedRoles[cmdid], DCP_RoleCount[cmdid]--, index);
	return 1;
}

stock bool:DCP_IsRoleAllowed(cmdid, DCC_Role:role) {
	return DCP_BinarySearch(role, DCP_AllowedRoles[cmdid], DCP_RoleCount[cmdid]) != -1;
}

stock DCP_GetCommandId(const name[]) {
	new hash = fnv1(name) % DCP_MAX_COMMANDS;
	for(new i = 0; i < DCP_CommandCollisionsCount[hash]; i++) {
		if(!strcmp(name, DCP_CommandInfo[DCP_CommandIds[hash][i]][e_DCP_COMMAND_DATA_NAME])) {
			return DCP_CommandIds[hash][i];
		}
	}
	return INVALID_DISCORD_COMMAND;
}

#define DCP_SetEnabled(%0,%1)				DCP_CommandInfo[(%0)][e_DCP_COMMAND_DATA_DISABLED] = !(%1)
#define DCP_IsEnabled(%0)					!DCP_CommandInfo[(%0)][e_DCP_COMMAND_DATA_DISABLED]

#define DCP_CommandCount()					DCP_CommandCount

#define DCP_GetName(%0)					DCP_CommandInfo[(%0)][e_DCP_COMMAND_DATA_NAME]

/*
	internal
*/

#if !defined DCP_GUILD_NAME
	#error Some of the functionality of DCP requieres the guild name, please define DCP_GUILD_NAME before including this package
#endif

new DCC_Guild:DCP_guild;

#include <YSI_Coding\y_hooks>
hook OnScriptInit() {
	new
		functionName[MAX_FUNC_NAME],
		idx;

	memset(DCP_CommandIds[0], INVALID_DISCORD_COMMAND, sizeof (DCP_CommandIds) * sizeof(DCP_CommandIds[]));

	DCP_guild = DCC_FindGuildByName(DCP_GUILD_NAME);

    while ((idx = AMX_GetPublicNamePrefix(idx, functionName, _A<DCP_>))) {
        strunpack(functionName, functionName);
		DCP_RegisterCommand(functionName[4]);
    }
}

public DCC_OnMessageCreate(DCC_Message:message) {
	new DCC_Channel:channel, DCC_User:author, content[250];
	DCC_GetMessageChannel(message, channel);
	DCC_GetMessageAuthor(message, author);
	DCC_GetMessageContent(message, content, sizeof content);

	if(content[0] == discordPrefix)	{
		processCommand(message, channel, author, content);
		return 1;
	}
	else//message doesn't have prefix thus isn't a command
	{
	  	#if defined OnDiscordMessageCreated
			return OnDiscordMessageCreated(DCC_Message:message);
		#endif
	}
}

static const FNV_offset_basis = 2166136261;
static const FNV_prime = 16777619;

static stock fnv1(const str[], &len = -1) {
    if(len < 0) {
        len = strlen(str);
    }

    new hash = FNV_offset_basis, i;
    while(i != len) {
        hash = hash * FNV_prime;
        hash = hash ^ str[i++];
    }
    return hash;
}

static stock DCP_ShiftRigth({DCC_Role, DCC_Channel}:array[], length, index) {
	for(new  i = length-1; i > index; i--) {
		array[i] = array[i-1];
	}
}

static stock DCP_ShiftLeft({DCC_Role, DCC_Channel}:array[], length, index) {
	for(new i = index; i < length-1; i++) {
		array[i] = array[i+1];
	}
}

static stock DCP_BinarySearch({DCC_Role, DCC_Channel}:tofind, const {DCC_Role, DCC_Channel}:array[], length, &midout = 0) {
	new first, last = length-1, mid;
	while(first <= last) {
		mid = (first + last)/2;
		midout = mid;
		if(array[first] == tofind) {
			return first;
		} else if(array[last] == tofind) {
			return last;
		} else {
			if(array[mid] == tofind) {
				return mid;
			} else if(array[mid] < tofind) {
				first = mid+1;
				last -= 1;
			} else {
				first += 1;
				last = mid-1;
			}
		}
	}
	return -1;
}

static DCP_RegisterCommand(const name[]) {
	new hash = fnv1(name) % DCP_MAX_COMMANDS;
	DCP_CommandIds[hash][DCP_CommandCollisionsCount[hash]] = DCP_CommandCount;
	DCP_CommandCollisionsCount[hash]++;
	DCP_CommandInfo[DCP_CommandCount][e_DCP_COMMAND_DATA_NAME][0] = '\0';
	strcat(DCP_CommandInfo[DCP_CommandCount][e_DCP_COMMAND_DATA_NAME], name, MAX_FUNC_NAME);
	++DCP_CommandCount;
	return DCP_CommandCount-1;
}

static processCommand(DCC_Message:message, DCC_Channel:channel, DCC_User:author, const cmdtext[]) {
		new
			pos,
			funcname[MAX_FUNC_NAME],
			bool:hasArgs = true,
			success;
		while (cmdtext[++pos] > ' ') {
			funcname[pos-1] = cmdtext[pos];
		}

		while (cmdtext[pos] == ' ') pos++;
		if(!cmdtext[pos]) {
			hasArgs = false;
		}

		new cmdid = DCP_GetCommandId(funcname), ret;
		if(hasArgs)	{
			ret = CallLocalFunction("OnDiscordCommandReceived", "iiiis",
				_:channel, _:author, _:message, cmdid, cmdtext[pos]);
		} else {
			ret = CallLocalFunction("OnDiscordCommandReceived", "iiiis",
				_:channel, _:author, _:message, cmdid, "\1");
		}
		if(!ret)
			return 0;

		if(cmdid == INVALID_DISCORD_COMMAND) {
			if(hasArgs)	{
				CallLocalFunction("OnDiscordCommandPerformed", "iiiisi",
					_:channel, _:author, _:message, cmdid, cmdtext[pos], e_DCP_SUCCESS_INVALID);
			} else {
				CallLocalFunction("OnDiscordCommandPerformed", "iiiisi",
					_:channel, _:author, _:message, cmdid, "\1", e_DCP_SUCCESS_INVALID);
			}
			return 1;
		}

		if(!DCP_IsEnabled(cmdid)) {
			success = e_DCP_SUCCESS_DISABLED;
		}

		new roleCount, bool:allowed;
		if(!DCP_RoleCount[cmdid]) {
			allowed = true;
		} else {
			DCC_GetGuildMemberRoleCount(DCP_guild, author, roleCount);
			new authorName[DCC_USERNAME_SIZE];
			DCC_GetUserName(author, authorName);
			for(new i = 0, DCC_Role:role; i < roleCount; i++) {
				DCC_GetGuildMemberRole(DCP_guild, author, i, role);
				if(DCP_IsRoleAllowed(cmdid, role)) {
					allowed = true;
					break;
				}
			}
		}

		if(!allowed) {
			success = e_DCP_SUCCESS_ROLE_DENIED;
		}

		if(DCP_ChannelCount[cmdid]) {
			if(!DCP_IsChannelAllowed(cmdid, channel)) {
				success = e_DCP_SUCCESS_CHANNEL_DENIED;
			}
		}

		if(success)	{
			if(hasArgs)	{
				CallLocalFunction("OnDiscordCommandPerformed", "iiiisi",
					_:channel, _:author, _:message, cmdid, cmdtext[pos], success);
			} else {
				CallLocalFunction("OnDiscordCommandPerformed", "iiiisi",
					_:channel, _:author, _:message, cmdid, "\1", success);
			}
			return 1;
		}

		format(funcname, sizeof(funcname), "DCP_%s", funcname);

		if(!hasArgs) {
			return CallLocalFunction("OnDiscordCommandPerformed", "",
				_:channel, _:author,_: message, cmdid,"\1",
				CallLocalFunction(funcname, "iiiisi", cmdid, _:message, _:channel, _:author, "\1",0));
		}
		return CallLocalFunction("OnDiscordCommandPerformed", "iiiisi",
					_:channel, _:author, _:message, cmdid, cmdtext[pos],
					CallLocalFunction(funcname, "iiiisi", cmdid, _:message, _:channel, _:author, cmdtext[pos],0));
}