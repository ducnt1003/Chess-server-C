#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/timeb.h>
#include "data.h"
#include "protos.h"
#include "message.h"
#include <wchar.h>

#define white_king 0x2654	// ♔
#define white_queen 0x2655	// ♕
#define white_rook 0x2656	// ♖
#define white_bishop 0x2657 // ♗
#define white_knight 0x2658 // ♘
#define white_pawn 0x2659	// ♙
#define black_king 0x265A	// ♚
#define black_queen 0x265B	// ♛
#define black_rook 0x265C	// ♜
#define black_bishop 0x265D // ♝
#define black_knight 0x265E // ♞
#define black_pawn 0x265F	// ♟

BOOL ftime_ok = FALSE; /* does ftime return milliseconds? */
int get_ms()
{
	struct timeb timebuffer;
	ftime(&timebuffer);
	if (timebuffer.millitm != 0)
		ftime_ok = TRUE;
	return (timebuffer.time * 1000) + timebuffer.millitm;
}

#define MAXLENGTH 100

/* mainConsoleChess() is basically an infinite loop that either calls
   think() when it's the enermy's turn to move or prompts
   the user for a command (and deciphers it). */

int my_print_result(int sockfd, int a)
{
	int i;

	/* is there a legal move? */
	for (i = 0; i < first_move[1]; ++i)
		if (makemove(gen_dat[i].m.b))
		{
			takeback();
			break;
		}
	if (i == first_move[1])
	{
		if (in_check(side))
		{
			if (side == LIGHT)
			{
				printf("0-1 {Black mates}\n");
			}
			else
			{
				printf("1-0 {White mates}\n");
			}
			if (a == 1)
			{
				printf("You Win!!!!\n");
				if (send(sockfd, message_toString(message_construct(RESULT, "1", "0", "1", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
				{
					printf("send() sent a different number of bytes than expected\n");
				}
				return 1;
			}
			else if (a == 0)
			{
				printf("You Lose!!!!\n");
				if (send(sockfd, message_toString(message_construct(RESULT, "1", "0", "1", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
				{
					printf("send() sent a different number of bytes than expected\n");
				}
				return 1;
			}
		}
		else
		{
			printf("1/2-1/2 {Stalemate}\n");
			if (send(sockfd, message_toString(message_construct(RESULT, "1/2", "1/2", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
			{
				printf("send() sent a different number of bytes than expected\n");
			}
			return 1;
		}
	}
	else if (reps() == 3)
	{
		printf("1/2-1/2 {Draw by repetition}\n");
		if (send(sockfd, message_toString(message_construct(RESULT, "1/2", "1/2", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
		{
			printf("send() sent a different number of bytes than expected\n");
		}
		return 1;
	}
	else if (fifty >= 100)
	{
		printf("1/2-1/2 {Draw by fifty move rule}\n");
		if (send(sockfd, message_toString(message_construct(RESULT, "1/2", "1/2", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
		{
			printf("send() sent a different number of bytes than expected\n");
		}
		return 1;
	}
	return 0;
}

int mainConsoleChess(int pick_side, int sockfd)
{
	int enermy_side;
	char s[1024];
	int m;
	char recvline[MAXLENGTH];
	message_t newMesg;

	printf("\n");
	printf("Console Chess\n");
	printf("\n");
	printf("Type \"help\" to displays a list of commands.\n");
	printf("\n");
	init_hash();
	init_board();
	gen();
	enermy_side = pick_side ^ 1;

	max_time = 1 << 25;
	max_depth = 4;

	for (;;)
	{
		if (side == enermy_side)
		{ /* enermy's turn */
			printf("Your opposite's turn now. Please wait...\n");
			if (recv(sockfd, recvline, MESSAGE_MAXLEN, 0) < 0)
			{
				printf("Server terminated prematurely\n");
				exit(1);
			}
			recvline[strlen(recvline)] = '\0';
			newMesg = message_parse(recvline);
			strcpy(s, newMesg->arg);
			if (newMesg->cmd == SIGNOUT)
			{
				// if (send(sockfd, message_toString(message_construct(RESULT, "1/2", "1/2", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
				// {
				// 	printf("send() sent a different number of bytes than expected\n");
				// }
				printf("Your opponent has surrendered!!!!!\nYou Win!!!!!!\n");
				goto DONE_GAME_ENEMY;
			}
			if (newMesg->cmd == DRAW)
			{
				printf("Your opponent want to draw (y/n) ?\n");
				scanf("%s", s);
				if (strcmp(s, "y") == 0 || strcmp(s, "Y") == 0)
				{
					if (send(sockfd, message_toString(message_construct(DRAW, "0", "1", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
					{
						printf("send() sent a different number of bytes than expected\n");
					}
					printf("DRAW !!!!!!\n");
					goto DONE_GAME_ENEMY;
				}
				else
				{
					if (send(sockfd, message_toString(message_construct(DRAW, "0", "0", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
					{
						printf("send() sent a different number of bytes than expected\n");
					}
					continue;
				}
			}

			printf("Your enermy's move: %s\n", s);

			m = parse_move(s);
			if (m == -1 || !makemove(gen_dat[m].m.b))
				printf("Illegal move.\n");
			else
			{
				ply = 0;
				gen();
				;
				if (my_print_result(sockfd, 0) == 1)
				{
					goto DONE_GAME;
				}
			}
			continue;
		}
		if (pick_side == 0)
		{
			print_board();
		}
		else
			print_reverse_board();
		printf("ConsoleChess > Your turn now.\n");
		printf("ConsoleChess > ");
		/* get user input */
		if (scanf("%s", s) == EOF)
			return 0;
		if (!strcmp(s, "d"))
		{
			if (pick_side == 0)
			{
				print_board();
			}
			else
				print_reverse_board();
			continue;
		}
		if (!strcmp(s, "sur"))
		{
			printf("Thanks for playing. Enjoy!\n");
			while (getchar() != '\n')
				;
			if (send(sockfd, message_toString(message_construct(RESULT, "0", "1", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
			{
				printf("send() sent a different number of bytes than expected\n");
			}
			printf("You Lose!!!!!!\n");
			goto DONE_GAME;
			break;
		}
		if (!strcmp(s, "draw"))
		{
			if (send(sockfd, message_toString(message_construct(DRAW, "1", "0", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
			{
				printf("send() sent a different number of bytes than expected\n");
			}
			printf("Please wait your opponent\n");
			if (recv(sockfd, recvline, MESSAGE_MAXLEN, 0) < 0)
			{
				printf("Server terminated prematurely\n");
				exit(1);
			}
			recvline[strlen(recvline)] = '\0';
			newMesg = message_parse(recvline);
			if (newMesg->cmd == DRAW && newMesg->arg[0] == '1')
			{
				printf("Your opponent accept to draw!!!!\n");
				printf("DRAW !!!!!!\n");
				goto DONE_GAME_ENEMY;
			}
			else
			{
				printf("Your opponent dont accept to draw!!!!\n");
				continue;
			}
		}
		if (!strcmp(s, "help"))
		{
			printf("d - display the board\n");
			printf("sur - surrender\n");
			printf("draw - want to draw\n");
			printf("Enter moves in coordinate notation, e.g., e2e4, e7e8Q (for promote moving)\n");
			continue;
		}

		/* maybe the user entered a move? */
		m = parse_move(s);
		if (m == -1 || !makemove(gen_dat[m].m.b))
			printf("Illegal move.\n");
		else
		{
			ply = 0;
			gen();
			if (send(sockfd, message_toString(message_construct(MOVE, s, "0", "0", "0")), MESSAGE_MAXLEN, 0) != MESSAGE_MAXLEN)
			{
				printf("send() sent a different number of bytes than expected\n");
			}
			if (pick_side == 0)
			{
				print_board();
			}
			else
				print_reverse_board();
			if (my_print_result(sockfd, 1) == 1)
			{
				goto DONE_GAME;
			}
		}
	}

DONE_GAME:
	if (recv(sockfd, recvline, MESSAGE_MAXLEN, 0) < 0)
	{
		printf("Server terminated prematurely\n");
		exit(1);
	}
	recvline[strlen(recvline)] = '\0';
	newMesg = message_parse(recvline);
	if (newMesg->cmd == SIGNOUT)
	{
		printf("Game over!\n");
	}
	return 0;
DONE_GAME_ENEMY:

	printf("Thanks for playing. Enjoy!\n");
	printf("Game over!\n\n");

	return 0;
}

/* parse the move s (in coordinate notation) and return the move's
   index in gen_dat, or -1 if the move is illegal */

int parse_move(char *s)
{
	int from, to, i;

	/* make sure the string looks like a move */
	if (s[0] < 'a' || s[0] > 'h' ||
		s[1] < '0' || s[1] > '9' ||
		s[2] < 'a' || s[2] > 'h' ||
		s[3] < '0' || s[3] > '9')
		return -1;

	from = s[0] - 'a';
	from += 8 * (8 - (s[1] - '0'));
	to = s[2] - 'a';
	to += 8 * (8 - (s[3] - '0'));

	for (i = 0; i < first_move[1]; ++i)
		if (gen_dat[i].m.b.from == from && gen_dat[i].m.b.to == to)
		{

			/* if the move is a promotion, handle the promotion piece;
			   assume that the promotion moves occur consecutively in
			   gen_dat. */
			if (gen_dat[i].m.b.bits & 32)
				switch (s[4])
				{
				case 'N':
					return i;
				case 'B':
					return i + 1;
				case 'R':
					return i + 2;
				default: /* assume it's a queen */
					return i + 3;
				}
			return i;
		}

	/* didn't find the move */
	return -1;
}

/* move_str returns a string with move m in coordinate notation */

char *move_str(move_bytes m)
{
	static char str[6];

	char c;

	if (m.bits & 32)
	{
		switch (m.promote)
		{
		case KNIGHT:
			c = 'n';
			break;
		case BISHOP:
			c = 'b';
			break;
		case ROOK:
			c = 'r';
			break;
		default:
			c = 'q';
			break;
		}
		sprintf(str, "%c%d%c%d%c",
				COL(m.from) + 'a',
				8 - ROW(m.from),
				COL(m.to) + 'a',
				8 - ROW(m.to),
				c);
	}
	else
		sprintf(str, "%c%d%c%d",
				COL(m.from) + 'a',
				8 - ROW(m.from),
				COL(m.to) + 'a',
				8 - ROW(m.to));
	return str;
}

/* print_board() prints the board */

wchar_t translate_piece(int i)
{
	switch (color[i])
	{
	case EMPTY:
		return (' ');
		break;
	case LIGHT:
		// printf(" %c", piece_char[piece[i]]);
		switch (piece_char[piece[i]])
		{
		case 'P':
			return (black_pawn);
			break;
		case 'N':
			return (black_knight);
			break;
		case 'B':
			return (black_bishop);
			break;
		case 'R':
			return (black_rook);
			break;
		case 'Q':
			return (black_queen);
			break;
		case 'K':
			return (black_king);
			break;
		}
		break;
	case DARK:
		// printf(" %c", piece_char[piece[i]] + ('a' - 'A'));
		switch (piece_char[piece[i]])
		{
		case 'P':
			return (white_pawn);
			break;
		case 'N':
			return (white_knight);
			break;
		case 'B':
			return (white_bishop);
			break;
		case 'R':
			return (white_rook);
			break;
		case 'Q':
			return (white_queen);
			break;
		case 'K':
			return (white_king);
			break;
		}
		break;
	}
}

void print_board()
{
	setlocale(6, "en_US.UTF-8");
	printf("\n             ┌───────────────────┐          \n");
	printf("             │       C*CHESS     │          \n");
	printf("             └───────────────────┘          \n\n");
	printf("       ┌───┬───┬───┬───┬───┬───┬───┬───┐   \n");
	printf("     8 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(0), translate_piece(1), translate_piece(2), translate_piece(3), translate_piece(4), translate_piece(5), translate_piece(6), translate_piece(7));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     7 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(8), translate_piece(9), translate_piece(10), translate_piece(11), translate_piece(12), translate_piece(13), translate_piece(14), translate_piece(15));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     6 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(16), translate_piece(17), translate_piece(18), translate_piece(19), translate_piece(20), translate_piece(21), translate_piece(22), translate_piece(23));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     5 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(24), translate_piece(25), translate_piece(26), translate_piece(27), translate_piece(28), translate_piece(29), translate_piece(30), translate_piece(31));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     4 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(32), translate_piece(33), translate_piece(34), translate_piece(35), translate_piece(36), translate_piece(37), translate_piece(38), translate_piece(39));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     3 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(40), translate_piece(41), translate_piece(42), translate_piece(43), translate_piece(44), translate_piece(45), translate_piece(46), translate_piece(47));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     2 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(48), translate_piece(49), translate_piece(50), translate_piece(51), translate_piece(52), translate_piece(53), translate_piece(54), translate_piece(55));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     1 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(56), translate_piece(57), translate_piece(58), translate_piece(59), translate_piece(60), translate_piece(61), translate_piece(62), translate_piece(63));
	printf("       └───┴───┴───┴───┴───┴───┴───┴───┘   \n");
	printf("         a   b   c   d   e   f   g   h     \n\n");
}

void print_reverse_board()
{
	setlocale(6, "en_US.UTF-8");
	printf("\n             ┌───────────────────┐          \n");
	printf("             │       C*CHESS     │          \n");
	printf("             └───────────────────┘          \n\n");
	printf("       ┌───┬───┬───┬───┬───┬───┬───┬───┐   \n");
	printf("     1 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(63), translate_piece(62), translate_piece(61), translate_piece(60), translate_piece(59), translate_piece(58), translate_piece(57), translate_piece(56));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     2 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(55), translate_piece(54), translate_piece(53), translate_piece(52), translate_piece(51), translate_piece(50), translate_piece(49), translate_piece(48));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     3 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(47), translate_piece(46), translate_piece(45), translate_piece(44), translate_piece(43), translate_piece(42), translate_piece(41), translate_piece(40));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     4 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(39), translate_piece(38), translate_piece(37), translate_piece(36), translate_piece(35), translate_piece(34), translate_piece(33), translate_piece(32));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     5 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(31), translate_piece(30), translate_piece(29), translate_piece(28), translate_piece(27), translate_piece(26), translate_piece(25), translate_piece(24));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     6 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(23), translate_piece(22), translate_piece(21), translate_piece(20), translate_piece(19), translate_piece(18), translate_piece(17), translate_piece(16));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     7 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(15), translate_piece(14), translate_piece(13), translate_piece(12), translate_piece(11), translate_piece(10), translate_piece(9), translate_piece(8));
	printf("       ├───┼───┼───┼───┼───┼───┼───┼───┤   \n");
	printf("     8 │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │ %lc │  \n", translate_piece(7), translate_piece(6), translate_piece(5), translate_piece(4), translate_piece(3), translate_piece(2), translate_piece(1), translate_piece(0));
	printf("       └───┴───┴───┴───┴───┴───┴───┴───┘   \n");
	printf("         h   g   f   e   d   c   b   a     \n\n");
}

/* print_result() checks to see if the game is over, and if so,
   prints the result. */

void print_result(int sockfd)
{
	int i;

	/* is there a legal move? */
	for (i = 0; i < first_move[1]; ++i)
		if (makemove(gen_dat[i].m.b))
		{
			takeback();
			break;
		}
	if (i == first_move[1])
	{
		if (in_check(side))
		{
			if (side == LIGHT)
			{
				printf("0-1 {Black mates}\n");
			}
			else
			{
				printf("1-0 {White mates}\n");
			}
		}
		else
		{
			printf("1/2-1/2 {Stalemate}\n");
		}
	}
	else if (reps() == 3)
	{
		printf("1/2-1/2 {Draw by repetition}\n");
	}
	else if (fifty >= 100)
	{
		printf("1/2-1/2 {Draw by fifty move rule}\n");
	}
}