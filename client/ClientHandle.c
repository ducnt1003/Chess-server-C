#define _BSD_SOURCE || (_XOPEN_SOURCE >= 500 || _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED) && !(_POSIX_C_SOURCE >= 200112 L || _XOPEN_SOURCE >= 600)
#define _POSIX_SOURCE
#include "ClientHandle.h"
#include "message.h"
#define MAXLENGTH 10
#define MAXLINE 100

void red()
{
	printf("\033[0;31m");
}

void green()
{
	printf("\033[0;32m");
}

void yellow()
{
	printf("\033[0;33m");
}

void blue()
{
	printf("\033[0;34m");
}

void reset()
{
	printf("\033[0m");
}

void str_cli(int sockfd)
{
	char *sendline, recvline[MESSAGE_MAXLEN], opt[100];
	int ch;
	char option[100];
	message_t newMesg;
	fd_set readset;
	char invite[MAXLENGTH];

	while (1)
	{
		ch = menu();
		switch (ch)
		{
		case 1:
			blue();
			printf("\n══════════════════ ♜ SIGN IN ♜ ═════════════════\n");
			sendline = message_toString(getIdAndPassword(SIGNIN));
			reset();

			if (send(sockfd, sendline, MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
			{
				printf("send() sent a different number of bytes than expected\n");
			}

			if (recv(sockfd, recvline, MESSAGE_MAXLEN, 0) < 0)
			{
				red();
				printf("\n\n            ✖✖✖ Server terminated ✖✖✖\n");
				reset();
				exit(1);
			}

			recvline[strlen(recvline)] = '\0';
			newMesg = message_parse(recvline);
			newMesg->arg[1] = '\0';
			newMesg->arg1[1] = '\0';

			switch (newMesg->cmd)
			{
			case SIGNIN:
				green();
				printf("               ✔✔✔ SUCCESSFULLY ✔✔✔\n");
				reset();
				menuPlay();

				while (1)
				{
					FD_ZERO(&readset);
					FD_SET(sockfd, &readset);
					FD_SET(fileno(stdin), &readset);
					select(max(sockfd, fileno(stdin)) + 1, &readset, NULL, NULL, NULL);
					if (FD_ISSET(sockfd, &readset))
					{
						if (read(sockfd, recvline, MESSAGE_MAXLEN) == 0)
						{
							red();
							printf("\n\n            ✖✖✖ Server terminated ✖✖✖\n");
							reset();
							exit(1);
						}

						// TODO: receive invitation
						newMesg = message_parse(recvline);

						switch (newMesg->cmd)
						{
						case INVITE_ACCEPT:
							printf("Client %s invited you to his/her game (y/n) ?\n", newMesg->arg1);
							scanf("%s", opt);
							if (strcmp(opt, "y") == 0 || strcmp(opt, "Y") == 0)
							{
								if (send(sockfd, recvline, MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
								{
									printf("send() sent a different number of bytes than expected\n");
								}
							}
							else
							{
								if (send(sockfd, message_toString(message_construct(INVITE_DECLINE, newMesg->arg1, "0", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
								{
									printf("send() sent a different number of bytes than expected\n");
								}
								menuPlay();
							}

							break;
						case PICK_SIDE:
							if (strcmp(newMesg->arg, "0") == 0)
							{
								mainConsoleChess(0, sockfd);
								menuPlay();
							}
							else if (strcmp(newMesg->arg, "1") == 0)
							{
								mainConsoleChess(1, sockfd);
								menuPlay();
							}
							break;
						case PLAY_NOW:
							printf("wait for other ....\n");
							break;
						case GET_PLAYERLIST:
							parse(newMesg->arg);
							menuPlay();
							break;

						default:
							printf("Cannot get the message\n");
							break;
						}
					}
					else if (FD_ISSET(fileno(stdin), &readset))
					{
						fgets(option, 100, stdin);
						option[strlen(option) - 1] = '\0';
						if (strcmp(option, "1") == 0)
						{
							// TODO: inviting someone
							printf("Type the one'id who you want to invite\n");
							scanf("%s", invite);

							if (send(sockfd, message_toString(message_construct(INVITE_SEND, invite, "0", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
							{
								printf("send() sent a different number of bytes than expected\n");
							}

							printf("Waiting for responding from server...\n");
							if (recv(sockfd, recvline, MESSAGE_MAXLEN, 0) < 0)
							{
								red();
								printf("\n\n            ✖✖✖ Server terminated ✖✖✖\n");
								reset();
								exit(1);
							}

							recvline[strlen(recvline)] = '\0';
							newMesg = message_parse(recvline);
							newMesg->arg[1] = '\0';
							if (newMesg->cmd == PICK_SIDE)
							{
								if (strcmp(newMesg->arg, "0") == 0)
								{
									mainConsoleChess(0, sockfd);
									menuPlay();
								}
								else if (strcmp(newMesg->arg, "1") == 0)
								{
									mainConsoleChess(1, sockfd);
									menuPlay();
								}
							}
							else if (newMesg->cmd == INVITE_DECLINE)
							{
								if (newMesg->arg[0] == '1')
								{
									printf("He/she is not free\n");
									menuPlay();
								}
								else
								{
									printf("He/she declined your request\n");
									menuPlay();
								}
							}
						}
						else if (strcmp(option, "2") == 0)
						{
							// TODO: get the current players list from server
							if (send(sockfd, message_toString(message_construct(GET_PLAYERLIST, "0", "0", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
							{
								printf("send() sent a different number of bytes than expected\n");
							}

							printf("Waiting for responding from server...\n");
						}
						else if (strcmp(option, "3") == 0)
						{
							// TODO: get the current players list from server
							if (send(sockfd, message_toString(message_construct(PLAY_NOW, "0", "0", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
							{
								printf("send() sent a different number of bytes than expected\n");
							}

							printf("Waiting for responding from server...\n");
						}
						else if (strcmp(option, "4") == 0)
						{
							yellow();
							printf("\n             〇〇〇 SIGNED OUT 〇〇〇\n");
							sendline = message_toString(message_construct(SIGNOUT, "0", "0", "0", "0"));

							if (send(sockfd, sendline, MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
							{
								printf("send() sent a different number of bytes than expected\n");
							}

							if (recv(sockfd, recvline, MESSAGE_MAXLEN, 0) < 0)
							{
								red();
								printf("\n\n            ✖✖✖ Server terminated ✖✖✖\n");
								reset();
								exit(1);
							}
							reset();
							break;
						}
						else
						{
							yellow();
							printf("\n         !!! INVALID - ENTER 1, 2 OR 3 !!!\n");
							reset();
							menuPlay();
							continue;
						}
					}
				}

				break;

			default:
				red();
				printf("             ✖✖✖ INVALID ID OR PW ✖✖✖\n");
				// printf("                MESSAGE COMMAND: %d\n", newMesg->cmd);
				reset();
				break;
			}

			continue;

		case 2:
			blue();
			printf("\n══════════════════ ♜ SIGN UP ♜ ═════════════════\n");
			sendline = message_toString(getIdAndPassword(SIGNUP));
			reset();

			if (send(sockfd, sendline, MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
			{
				printf("send() sent a different number of bytes than expected\n");
			}

			if (recv(sockfd, recvline, MESSAGE_MAXLEN, 0) < 0)
			{
				red();
				printf("\n\n            ✖✖✖ Server terminated ✖✖✖\n");
				reset();
				exit(1);
			}

			recvline[strlen(recvline)] = '\0';

			newMesg = message_parse(recvline);
			switch (newMesg->cmd)
			{
			case SIGNUP:
				green();
				printf("               ✔✔✔ SUCCESSFULLY ✔✔✔\n");
				reset();
				break;
			default:
				red();
				printf("              ✖✖✖ ACCOUNT EXISTED ✖✖✖\n");
				reset();
				break;
			}

			continue;

		case 3:
			yellow();
			printf("\n                〇〇〇 EXIT 〇〇〇               \n\n");
			reset();
			// sendline = message_toString(message_construct(SIGNOUT, "0", "0", "0", "0"));

			// if (send(sockfd, sendline, MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
			// {
			// 	printf("send() sent a different number of bytes than expected\n");
			// }

			// if (recv(sockfd, recvline, MESSAGE_MAXLEN, 0) < 0)
			// {
			// 	red();
			// 	printf("\n\n            ✖✖✖ Server terminated ✖✖✖\n");
			// 	reset();
			// 	exit(1);
			// }

			// recvline[strlen(recvline)] = '\0';
			exit(0);
			break;

		default:
			yellow();
			printf("\n         !!! INVALID - ENTER 1, 2 OR 3 !!!\n");
			reset();
			continue;
		}

		break;
	}

	printf("\n");
}

int menu()
{
	int option, x;

	do
	{
		printf("\n");
		printf("╔═══════════════ ♕ ♔ C-CHESS ♚ ♛ ═══════════════╗\n");
		printf("║                                               ║\n");
		printf("║                  1. SIGN IN                   ║\n");
		printf("║                  2. SIGN UP                   ║\n");
		printf("║                  3. EXIT                      ║\n");
		printf("║                                               ║\n");
		printf("╠═══════════════════════════════════════════════╝\n");
		printf("║                                                \n");
		printf("╚═════════════ ♞ Enter choice: ");

		x = scanf("%d", &option);

		if (x != 1)
		{
			yellow();
			printf("\n         !!! INVALID - ENTER 1, 2 OR 3 !!!\n");
			reset();
			while (getchar() != '\n')
				;
		}
	} while (x != 1);

	while (getchar() != '\n')
		;
	return option;
}

void menuPlay()
{
	printf("\n");
	printf("╔═══════════════ ♕ ♔ C-CHESS ♚ ♛ ═══════════════╗\n");
	printf("║                                               ║\n");
	printf("║              1. Invite a player               ║\n");
	printf("║              2. Display player list           ║\n");
	printf("║              3. Play Now                      ║\n");
	printf("║              4. Sign out                      ║\n");
	printf("║                                               ║\n");
	printf("╠═══════════════════════════════════════════════╝\n");
	printf("║                                                \n");
	printf("╚═════════════ ♞ Enter choice: ");
	fflush(stdout);
}

message_t getIdAndPassword(int mesgType)
{
	char id[MAXLENGTH], *pass;

	printf("                   ♞ ID: ");
	fgets(id, 1000, stdin);
	id[strlen(id) - 1] = '\0';
	pass = getpass("                   ♞ PW: ");
	return message_construct(mesgType, id, pass, "0", "0");
}

int max(int i1, int i2)
{
	if (i1 > i2)
		return i1;
	else
		return i2;
}

void parse(char *userlist_str)
{
	int a = 0;
	char *pch = strtok(userlist_str, "|");
	while (pch != NULL)
	{
		int user_id, user_sockfd;
		char username[64];
		sscanf(pch, "%d,%d,%s", &user_id, &user_sockfd, username);
		printf("Client:  %s in port %d\n", username, user_sockfd);

		pch = strtok(NULL, "|");
		a++;
	}
	if (a == 0)
		printf("No player is free!!!!!! \n");
}
