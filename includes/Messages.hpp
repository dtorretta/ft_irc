#define once

#define CRLF "\r\n"

/******************/
/*    Messages    */
/******************/
#define MSG_WELCOME(nickname) (": 001 " + nickname + " : Connected to IRC network successfully!" + CRLF)
#define MSG_USERMODE(hostname, channelname, mode, user)  ":" + hostname + " MODE " + channelname + " " + mode + " " + user + CRLF
#define MSG_CREATION_TIME(nickname, channelname, creationtime) ": 329 " + nickname + " #" + channelname + " " + creationtime + CRLF
#define MSG_CHANNEL_MODES(nickname, channelname, modes) ": 324 " + nickname + " #" + channelname + " " + modes + CRLF
#define MSG_MODE_CHANGE(hostname, channelname, mode, arguments) (":" + hostname + " MODE #" + channelname + " " + mode + " " + arguments + CRLF)
#define MSG_NICK_UPDATE(oldnickname, nickname) (":" + oldnickname + " NICK " + nickname + CRLF)
#define MSG_USER_JOIN(hostname, ipaddress, channelname) (":" + hostname + "@" + ipaddress + " JOIN #" + channelname + CRLF)
#define MSG_NAMES_LIST(nickname, channelname, clientslist) (": 353 " + nickname + " @ #" + channelname + " :" + clientslist + CRLF)
#define MSG_NAMES_END(nickname, channelname) (": 366 " + nickname + " #" + channelname + " :End of /NAMES list" + CRLF)
#define MSG_CHANNEL_TOPIC(nickname, channelname, topic) (": 332 " + nickname + " #" +channelname + " :" + topic + "\r\n")

/****************/
/*    Errors    */
/****************/
#define ERROR_MODE_PARAM_REQUIRED(channelname, mode) (": 696 #" + channelname + " * Parameter required for mode. " + mode + CRLF)
#define ERROR_INVALID_MODE_PARAM(channelname, mode) ": 696 #" + channelname + " Invalid parameter for mode. " + mode + CRLF
#define ERROR_KEY_ALREADY_SET(channelname) ": 467 #" + channelname + " Channel key is already configured. " + CRLF
#define ERROR_UNRECOGNIZED_MODE(nickname, channelname, mode) ": 472 " + nickname + " #" + channelname + " " + mode + " :is an unknown channel mode" + CRLF
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
