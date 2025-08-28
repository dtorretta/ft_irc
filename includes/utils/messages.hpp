#define once

#define CRLF "\r\n"

/******************/
/*    Messages    */
/******************/
#define MSG_WELCOME(nickname) (": 001 " + nickname + " : Connected to IRC network successfully!" + CRLF)
#define MSG_USERMODE(hostname, channelname, mode, user)  (":" + hostname + " MODE " + channelname + " " + mode + " " + user + CRLF)
#define MSG_CREATION_TIME(nickname, channelname, creationtime) (": 329 " + nickname + " " + channelname + " " + creationtime + CRLF)
#define MSG_CHANNEL_MODES(nickname, channelname, modes) (": 324 " + nickname + " #" + channelname + " " + modes + CRLF)
#define MSG_MODE_CHANGE(nickname, user, channelname, modes, params) (":" + nickname + "!~" + user + "@localhost MODE " + channelname + " " + modes + " " + params + CRLF)
#define MSG_NICK_UPDATE(oldnickname, nickname) (":" + oldnickname + " NICK " + nickname + CRLF)
#define MSG_USER_JOIN(hostname, ipaddress, channelname) (":" + hostname + "@" + ipaddress + " JOIN " + channelname + CRLF)
#define MSG_NAMES_LIST(nickname, channelname, clientslist) (": 353 " + nickname + " @ " + channelname + " :" + clientslist + CRLF)
#define MSG_NAMES_END(nickname, channelname) (": 366 " + nickname + " " + channelname + " :End of /NAMES list" + CRLF)
#define MSG_CHANNEL_TOPIC(nickname, channelname, topic) (": 332 " + nickname + " " + channelname + " :" + topic + CRLF)
#define MSG_USER_PART(nickname, user, ipaddress, channelname, reason) (":" + nickname + "!~" + user + "@localhost PART " + channelname + " :" + reason + CRLF)
#define MSG_PRIVMSG_USER(nickname, user, target, message) (":" + nickname + "!~" + user + " PRIVMSG " + target + " :" + message + CRLF)
#define MSG_PRIVMSG_CHANNEL(nickname, user, target, message) (":" + nickname + "!~" + user + "@localhost PRIVMSG " + target + " :" + message + CRLF)
#define MSG_NO_SET_TOPIC(nickname, channelname) (": 331 " + nickname + " " + channelname + " :No topic is set" CRLF)
#define MSG_TOPIC_WHO_TIME(nickname, channelname, creationtime) (": 333 " + nickname + " " + channelname + " " + nickname + " " + creationtime + CRLF)
#define MSG_TO_INVITER(nickname, guest, channelname) (": 341 " + nickname + " You successfully invited " + guest + " to " + channelname + CRLF)
#define MSG_TO_INVITEE(hostname, guest, channelname) (":" + hostname + " INVITE " + guest + " " + channelname + CRLF)
#define MSG_KICK_USER(nickname, user, channelname, target) (":" + nickname + "!~" + user + "@localhost KICK " + channelname + " " + target + CRLF)
#define MSG_KICK_USER_REASON(nickname, user, channelname, target, reason) (":" + nickname + "!~" + user + "@localhost KICK " + channelname + " " + target + " :" + reason + CRLF)

/****************/
/*    Errors    */
/****************/
#define ERROR_MODE_PARAM_REQUIRED(channelname, mode) (": 696 #" + channelname + " * Parameter required for mode. " + mode + CRLF)
#define ERROR_INVALID_MODE_PARAM(channelname, mode) (": 696 #" + channelname + " Invalid parameter for mode. " + mode + CRLF)
#define ERROR_KEY_ALREADY_SET(channelname) (": 467 #" + channelname + " Channel key is already configured. " + CRLF)
#define ERROR_UNRECOGNIZED_MODE(nickname, channelname, mode) (": 472 " + nickname + " #" + channelname + " " + mode + " :is an unknown channel mode" + CRLF)
#define ERROR_INSUFFICIENT_PARAMS(nickname) (": 461 " + nickname + " :Insufficient parameters provided." + CRLF)
#define ERROR_CHANNEL_NOT_EXISTS(nickname, channelname) (": 403 " + nickname + " " + channelname + " :Channel does not exist" + CRLF)
#define ERROR_NOT_CHANNEL_OP(channelname) (": 482 #" + channelname + " :You are not a channel operator" + CRLF)
#define ERROR_NICK_NOT_FOUND(channelname, name) (": 401 #" + channelname + " " + name + " :No such nickname/channel" + CRLF )
#define ERROR_WRONG_PASSWORD(nickname) (": 464 " + nickname + " :Incorrect password!" + CRLF )
#define ERROR_ALREADY_REGISTERED(nickname) (": 462 " + nickname + " :You cannot register again!" + CRLF )
#define ERROR_NO_NICKNAME_PROVIDED(nickname) (": 431 " + nickname + " :No nickname specified" + CRLF )
#define ERROR_NICKNAME_IN_USE(nickname) (": 433 " + nickname + " :Nickname already taken" + CRLF)
#define ERROR_INVALID_NICKNAME(nickname) (": 432 " + nickname + " :Invalid nickname format" + CRLF)
#define ERROR_NOT_REGISTERED_YET(nickname) (": 451 " + nickname + " :Registration required!" + CRLF)
#define ERROR_COMMAND_NOT_RECOGNIZED(nickname, command) (": 421 " + nickname + " " + command + " :Command not found" + CRLF)
#define ERROR_TOO_MANY_TARGETS(nickname) (": 407 " + nickname + " :Too many channels" + CRLF)
#define ERROR_IN_TOO_MANY_CHANNELS(nickname) (": 405 " + nickname + " :You have joined too many channels" + CRLF)
#define ERROR_WRONG_KEY(nickname, channelname) (" : 475 " + nickname + " #" + channelname + " :Incorrect password for channel" + CRLF)
#define ERROR_INVITE_ONLY(nick, chan) (": 473 " + nick + " " + chan + " :Cannot join channel (+i)" + CRLF)
#define ERROR_CHANNEL_FULL(nick, chan) (": 471 " + nick + " " + chan + " :Cannot join channel (+l)" + CRLF)
#define ERROR_NOT_IN_CHANNEL(nick, chan) (": 442 " + nick + " " + chan + " :You are not on this channel" + CRLF)
#define ERROR__USER_NOT_IN_CHANNEL(nick, chan) (": 441 " + nick + " " + chan + " :User not on this channel" + CRLF)
#define ERROR_ALREADY_IN_CHANNEL(nick, chan) (": 443 " + nick + " " + chan + " :is already on this channel" + CRLF)
#define ERROR_NO_TEXT_TO_SEND(nick) (": 412 " + nick + " :No text to send" + CRLF)
#define ERROR_NO_RECIPIENT(nickname) (": 411 " + nickname + " :No recipient given (PRIVMSG)" + CRLF)


/*
#define RPL_CONNECTED(nickname) (": 001 " + nickname + " : Welcome to the IRC server!" + CRLF)
#define RPL_UMODEIS(hostname, channelname, mode, user)  ":" + hostname + " MODE " + channelname + " " + mode + " " + user + CRLF
#define RPL_CREATIONTIME(nickname, channelname, creationtime) ": 329 " + nickname + " #" + channelname + " " + creationtime + CRLF
#define RPL_CHANNELMODES(nickname, channelname, modes) ": 324 " + nickname + " #" + channelname + " " + modes + CRLF
#define RPL_CHANGEMODE(hostname, channelname, mode, arguments) (":" + hostname + " MODE #" + channelname + " " + mode + " " + arguments + CRLF)
#define RPL_NICKCHANGE(oldnickname, nickname) (":" + oldnickname + " NICK " + nickname + CRLF)
#define RPL_JOINMSG(hostname, ipaddress, channelname) (":" + hostname + "@" + ipaddress + " JOIN #" + channelname + CRLF)
#define RPL_NAMREPLY(nickname, channelname, clientslist) (": 353 " + nickname + " @ #" + channelname + " :" + clientslist + CRLF)
#define RPL_ENDOFNAMES(nickname, channelname) (": 366 " + nickname + " #" + channelname + " :END of /NAMES list" + CRLF)
#define RPL_TOPICIS(nickname, channelname, topic) (": 332 " + nickname + " #" +channelname + " :" + topic + "\r\n")

///////// ERRORS ////////////////
#define ERR_NEEDMODEPARM(channelname, mode) (": 696 #" + channelname + " * You must specify a parameter for the key mode. " + mode + CRLF)
#define ERR_INVALIDMODEPARM(channelname, mode) ": 696 #" + channelname + " Invalid mode parameter. " + mode + CRLF
#define ERR_KEYSET(channelname) ": 467 #" + channelname + " Channel key already set. " + CRLF
#define ERR_UNKNOWNMODE(nickname, channelname, mode) ": 472 " + nickname + " #" + channelname + " " + mode + " :is not a recognised channel mode" + CRLF
#define ERR_NOTENOUGHPARAM(nickname) (": 461 " + nickname + " :Not enough parameters." + CRLF)
#define ERR_CHANNELNOTFOUND(nickname, channelname) (": 403 " + nickname + " " + channelname + " :No such channel" + CRLF)
#define ERR_NOTOPERATOR(channelname) (": 482 #" + channelname + " :You're not a channel operator" + CRLF)
#define ERR_NOSUCHNICK(channelname, name) (": 401 #" + channelname + " " + name + " :No such nick/channel" + CRLF )
#define ERR_INCORPASS(nickname) (": 464 " + nickname + " :Password incorrect !" + CRLF )
#define ERR_ALREADYREGISTERED(nickname) (": 462 " + nickname + " :You may not reregister !" + CRLF )
#define ERR_NONICKNAME(nickname) (": 431 " + nickname + " :No nickname given" + CRLF )
#define ERR_NICKINUSE(nickname) (": 433 " + nickname + " :Nickname is already in use" + CRLF)
#define ERR_ERRONEUSNICK(nickname) (": 432 " + nickname + " :Erroneus nickname" + CRLF)
#define ERR_NOTREGISTERED(nickname) (": 451 " + nickname + " :You have not registered!" + CRLF)
#define ERR_CMDNOTFOUND(nickname, command) (": 421 " + nickname + " " + command + " :Unknown command" + CRLF)
*/
