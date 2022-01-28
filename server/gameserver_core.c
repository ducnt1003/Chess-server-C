#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "users.h"
#include "rooms.h"
#include "gameserver_core.h"

FILE *fp;
char *userlist_filepath = "userlist.txt";
char *result_filepath = "result.txt";

userlist_t user_list;
roomlist_t room_list;

void gameserver_core_init()
{
	user_list = userlist_new();

	fp = fopen(userlist_filepath, "rw");
	if (fp == NULL)
		exit(EXIT_FAILURE);

	room_list = roomlist_new();
}

int gameserver_core_loadUsers()
{
	char *line_buffer = NULL;
	size_t len = 0;
	ssize_t read;

	char **line;
	line = (char **)malloc(sizeof(char *) * 1024);

	int count_line = 0;
	while ((read = getline(&line_buffer, &len, fp)) != -1)
	{
		line[count_line] = (char *)malloc(read);
		strcpy(line[count_line], line_buffer);
		count_line++;
	}

	free(line_buffer);

	for (int i = 0; i < count_line; i++)
	{
		int id;
		char *username;
		char *password;

		char *pch;
		pch = strtok(line[i], ",");
		id = atoi(pch);
		username = strtok(NULL, ",");
		password = strtok(NULL, ",");
		password[strlen(password) - 1] = '\0';
		user_t newUser = user_new(id, username, password, OFFLINE, -1);
		userlist_append(user_list, newUser);
	}

	for (int i = 0; i < count_line; i++)
	{
		free(line[i]);
	}

	free(line);
	fclose(fp);
	return count_line;
}

int gameserver_core_signin(char *username, char *password, int sockfd)
{
	userlist_t ptr;
	userlist_traverse(ptr, user_list)
	{
		if ((strcmp(ptr->user->username, username) == 0) &&
			(strcmp(ptr->user->password, password) == 0))
		{
			ptr->user->status = ONLINE;
			ptr->user->sockfd = sockfd;
			return ptr->user->id;
		}
	}

	return -1;
}

int gameserver_core_signup(char *username, char *password)
{
	int last_id = userlist_num(user_list);
	printf("%d", last_id);
	user_t newUser = user_new(last_id + 1, username, password, OFFLINE, -1);
	userlist_append(user_list, newUser);
	fp = fopen(userlist_filepath, "a");
	fprintf(fp, "\n%d,%s,%s", newUser->id, newUser->username, newUser->password);
	fclose(fp);
	return last_id;
}

char *gameserver_core_getPlayerlist(int fd)
{
	char *userlist_str = (char *)malloc(1024);
	strcpy(userlist_str, "");
	// user_id, sockfd, username
	userlist_t ptr;
	userlist_traverse(ptr, user_list)
	{
		char *buffer = (char *)malloc(512);
		if (ptr->user->status == ONLINE && ptr->user->sockfd != fd)
		{
			sprintf(buffer, "%d,%d,%s|", ptr->user->id, ptr->user->sockfd, ptr->user->username);
		
			
			strcat(userlist_str, buffer);
		}

		free(buffer);
	}

	return userlist_str;
}

char *gameserver_core_getResult(int id)
{
	int num;
	char *result_str = (char *)malloc(1024);
	FILE *f1;
	f1 = fopen(result_filepath, "r");
	if (f1 == NULL)
		exit(EXIT_FAILURE);

	fscanf(f1, "%d\n", num);
	printf("%d\n", num);
	for (int i = 0; i < num; i++)
	{
		int player_id, match;
		fscanf(f1, "%d,%d\n", player_id, match);
		for (int j = 0; j < match; j++)
		{
			char *buffer = (char *)malloc(512);
			char *name = (char *)malloc(512);
			char *result = (char *)malloc(512);
			fscanf(f1, "%s,%s\n", name, result);
			sprintf(buffer, "%s,%s|", name, result);
			if (player_id == id)
			{
				strcat(result_str, buffer);
				printf("%s\n", result_str);
			}
			free(buffer);
			free(name);
			free(result);
		}
	}
	fclose(f1);
	return result_str;
}

void gameserver_core_status_Playing(int sockfd)
{
	userlist_t ptr;
	userlist_traverse(ptr, user_list)
	{
		if (ptr->user->sockfd == sockfd)
		{
			ptr->user->status = PLAYING;
			printf("%d\n", sockfd);
		}
	}
}

void gameserver_core_status_Online(int sockfd)
{
	userlist_t ptr;
	userlist_traverse(ptr, user_list)
	{
		if (ptr->user->sockfd == sockfd)
		{
			ptr->user->status = ONLINE;
			printf("%d\n", sockfd);
		}
	}
}

void gameserver_core_status_Wait(int sockfd)
{
	userlist_t ptr;
	userlist_traverse(ptr, user_list)
	{
		if (ptr->user->sockfd == sockfd)
		{
			ptr->user->status = WAIT;
			printf("%d\n", sockfd);
		}
	}
}

void gameserver_core_status_Ofline(int sockfd)
{
	userlist_t ptr;
	userlist_traverse(ptr, user_list)
	{
		if (ptr->user->sockfd == sockfd)
		{
			ptr->user->status = OFFLINE;
			printf("%d\n", sockfd);
		}
	}
}

int gameserver_core_status_find_Wait()
{
	userlist_t ptr;
	userlist_traverse(ptr, user_list)
	{
		if (ptr->user->status == WAIT)
		{
			return ptr->user->sockfd;
		}
	}
	return 0;
}

int gameserver_core_checkOnline(int sockfd)
{
	userlist_t ptr;
	userlist_traverse(ptr, user_list)
	{
		if (ptr->user->sockfd == sockfd && ptr->user->status == ONLINE)
		{
			return 1;
		}
	}
	return 0;
}

int gameserver_core_getOtherUserSockfd(int sockfd)
{
	return get_otherUserSockfd(room_list, sockfd);
}

void gameserver_core_initNewRoom(int sockfd1, int sockfd2)
{
	roomlist_append(room_list, room_new(sockfd1, sockfd2));
}

roomlist_t findRoom(int sockfd)
{
	roomlist_t ptr;
    roomlist_traverse (ptr, room_list) {
        if (ptr->room->user1_sockfd == sockfd)
            return ptr;
        if (ptr->room->user2_sockfd == sockfd)
            return ptr;
    }
	return NULL;
}

void gameserver_core_removeRoom(int sockfd)
{
	roomlist_t ptr = findRoom(sockfd);
	if (ptr != NULL){
		roomlist_removeNode(ptr);
	}
}

void gameserver_core_endSession()
{
	fclose(fp);
}