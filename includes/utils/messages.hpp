#define once

#define CRLF "\r\n"

/******************/
/*    Messages    */
/******************/
#define MSG_WELCOME(nickname) (":ft_irc 001 " + nickname + " :Connected to IRC network successfully!" + CRLF)
#define MSG_CREATION_TIME(nickname, channelname, creationtime) (":ft_irc 329 " + nickname + " " + channelname + " " + creationtime + CRLF)
#define MSG_CHANNEL_MODES(nickname, channelname, modes) (":ft_irc 324 " + nickname + " " + channelname + " " + modes + CRLF)
#define MSG_MODE_CHANGE(nickname, user, channelname, modes, params) (":" + nickname + "!~" + user + "@localhost MODE " + channelname + " " + modes + " " + params + CRLF)
#define MSG_NICK_UPDATE(oldnickname, nickname) (":" + oldnickname + " NICK " + nickname + CRLF)
#define MSG_USER_JOIN(hostname, ipaddress, channelname) (":" + hostname + "@" + ipaddress + " JOIN " + channelname + CRLF)
#define MSG_NAMES_LIST(nickname, channelname, clientslist) (":ft_irc 353 " + nickname + " @ " + channelname + " :" + clientslist + CRLF)
#define MSG_NAMES_END(nickname, channelname) (":ft_irc 366 " + nickname + " " + channelname + " :End of /NAMES list" + CRLF)
#define MSG_CHANNEL_TOPIC(nickname, channelname, topic) (":ft_irc 332 " + nickname + " " + channelname + " :" + topic + CRLF)
#define MSG_USER_PART(nickname, user, ipaddress, channelname, reason) (":" + nickname + "!~" + user + "@localhost PART " + channelname + " :" + reason + CRLF)
#define MSG_PRIVMSG_USER(nickname, user, target, message) (":" + nickname + "!~" + user + " PRIVMSG " + target + " :" + message + CRLF)
#define MSG_PRIVMSG_CHANNEL(nickname, user, target, message) (":" + nickname + "!~" + user + "@localhost PRIVMSG " + target + " :" + message + CRLF)
#define MSG_NO_SET_TOPIC(nickname, channelname) (":ft_irc 331 " + nickname + " " + channelname + " :No topic is set" CRLF)
#define MSG_TOPIC_WHO_TIME(nickname, channelname, creationtime) (":ft_irc 333 " + nickname + " " + channelname + " " + nickname + " " + creationtime + CRLF)
#define MSG_TO_INVITER(nickname, guest, channelname) (":ft_irc 341 " + nickname + " You successfully invited " + guest + " to " + channelname + CRLF)
#define MSG_TO_INVITEE(hostname, guest, channelname) (":" + hostname + " INVITE " + guest + " " + channelname + CRLF)
#define MSG_KICK_USER(nickname, user, channelname, target) (":" + nickname + "!~" + user + "@localhost KICK " + channelname + " " + target + CRLF)
#define MSG_KICK_USER_REASON(nickname, user, channelname, target, reason) (":" + nickname + "!~" + user + "@localhost KICK " + channelname + " " + target + " :" + reason + CRLF)
#define MSG_QUIT(nickname, user, reason) (":" + nickname + "!~" + user + "@localhost QUIT :" + reason + CRLF)

/****************/
/*    Errors    */
/****************/
#define ERROR_UNRECOGNIZED_MODE(nickname, channelname, mode) (":ft_irc 472 " + nickname + " #" + channelname + " " + mode + " :is an unknown channel mode" + CRLF)
#define ERROR_INSUFFICIENT_PARAMS(nickname) (":ft_irc 461 " + nickname + " :Insufficient parameters provided." + CRLF)
#define ERROR_CHANNEL_NOT_EXISTS(nickname, channelname) (":ft_irc 403 " + nickname + " " + channelname + " :Channel does not exist" + CRLF)
#define ERROR_NOT_CHANNEL_OP(channelname) (":ft_irc 482 #" + channelname + " :You are not a channel operator" + CRLF)
#define ERROR_NICK_NOT_FOUND(channelname, name) (":ft_irc 401 #" + channelname + " " + name + " :No such nickname/channel" + CRLF )
#define ERROR_WRONG_PASSWORD(nickname) (":ft_irc 464 " + nickname + " :Incorrect password!" + CRLF )
#define ERROR_ALREADY_REGISTERED(nickname) (":ft_irc 462 " + nickname + " :You cannot register again!" + CRLF )
#define ERROR_NO_NICKNAME_PROVIDED(nickname) (":ft_irc 431 " + nickname + " :No nickname specified" + CRLF )
#define ERROR_NICKNAME_IN_USE(nickname) (":ft_irc 433 " + nickname + " :Nickname already taken" + CRLF)
#define ERROR_INVALID_NICKNAME(nickname) (":ft_irc 432 " + nickname + " :Invalid nickname format" + CRLF)
#define ERROR_NOT_REGISTERED_YET(nickname) (":ft_irc 451 " + nickname + " :Registration required!" + CRLF)
#define ERROR_COMMAND_NOT_RECOGNIZED(nickname, command) (":ft_irc 421 " + nickname + " " + command + " :Command not found" + CRLF)
#define ERROR_TOO_MANY_TARGETS(nickname) (":ft_irc 407 " + nickname + " :Too many channels" + CRLF)
#define ERROR_IN_TOO_MANY_CHANNELS(nickname) (":ft_irc 405 " + nickname + " :You have joined too many channels" + CRLF)
#define ERROR_WRONG_KEY(nickname, channelname) (":ft_irc 475 " + nickname + " #" + channelname + " :Incorrect password for channel" + CRLF)
#define ERROR_INVITE_ONLY(nick, chan) (":ft_irc 473 " + nick + " " + chan + " :Cannot join channel (+i)" + CRLF)
#define ERROR_CHANNEL_FULL(nick, chan) (":ft_irc 471 " + nick + " " + chan + " :Cannot join channel (+l)" + CRLF)
#define ERROR_NOT_IN_CHANNEL(nick, chan) (":ft_irc 442 " + nick + " " + chan + " :You are not on this channel" + CRLF)
#define ERROR__USER_NOT_IN_CHANNEL(nick, chan) (":ft_irc 441 " + nick + " " + chan + " :User not on this channel" + CRLF)
#define ERROR_ALREADY_IN_CHANNEL(nick, chan) (":ft_irc 443 " + nick + " " + chan + " :is already on this channel" + CRLF)
#define ERROR_NO_TEXT_TO_SEND(nick) (":ft_irc 412 " + nick + " :No text to send" + CRLF)
#define ERROR_NO_RECIPIENT(nickname) (":ft_irc 411 " + nickname + " :No recipient given (PRIVMSG)" + CRLF)
#define ERROR_NO_ACTIVE_MODE()
