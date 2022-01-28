#ifndef __GAMESERVER_CORE_H__
#define __GAMESERVER_CORE_H__


extern void gameserver_core_init();
extern int gameserver_core_loadUsers();
extern int gameserver_core_signin(char *username, char *password, int sockfd);
extern int gameserver_core_signup(char *username, char *password);
extern char *gameserver_core_getPlayerlist(int fd);
extern char *gameserver_core_getResult(int id);
extern int gameserver_core_getOtherUserSockfd(int sockfd);
extern void gameserver_core_status_Playing(int sockfd);
extern void gameserver_core_status_Online(int sockfd);
extern void gameserver_core_status_Wait(int sockfd);
extern void gameserver_core_status_Ofline(int sockfd);
extern int gameserver_core_status_find_Wait();
extern int gameserver_core_checkOnline(int sockfd);
extern void gameserver_core_initNewRoom(int sockfd1, int sockfd2);
extern void gameserver_core_removeRoom(int sockfd);
extern void gameserver_core_endSession();

#endif
