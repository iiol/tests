#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>

#include <sys/select.h>
#include <sys/types.h>

#define ESC 27

enum direction {
	LEFT, DOWN, UP, RIGHT,
};

struct coord {
	int x;
	int y;
};

struct snake {
	struct coord p;
	struct snake *next;
};

struct field {
	struct coord min;
	struct coord max;
	struct coord food;
	struct snake *head;
	struct snake *tail;
	int direction;
} field;

unsigned int seed;

void
genfood(void)
{
	struct coord p;
	struct snake *sp;

	while (1) {
		p.x = rand_r(&seed) % (field.max.x - field.min.x + 1) + field.min.x;
		p.y = rand_r(&seed) % (field.max.y - field.min.y + 1) + field.min.y;

		for (sp = field.tail; sp != NULL; sp = sp->next) {
			if (!memcmp(&p, &sp->p, sizeof (struct coord))) ////
				break;
		}

		if (sp == NULL)
			break;
	}

	field.food = p;

	printf("\e[%d;%dH*", p.y, p.x);
}

void
init_fld(void)
{
	int x, y;
	int xpos, ypos;
	int size;

	printf("\e[255;255H\e[6n");
	fflush(stdout);
	scanf("\e[%d;%dR", &ypos, &xpos);

	field.min.x = xpos/6;
	field.min.y = ypos/6;
	field.max.x = 5*xpos/6;
	field.max.y = 5*ypos/6;

	for (x = field.min.x - 1; x <= field.max.x + 1; ++x)
		for (y = field.min.y - 1; y <= field.max.y + 1; ++y)
			printf("\e[%d;%dHH", y, x);

	for (x = field.min.x; x <= field.max.x; ++x)
		for (y = field.min.y; y <= field.max.y; ++y)
			printf("\e[%d;%dH ", y, x);

	size = sizeof (struct snake);

	field.head = malloc(size);
	field.tail = malloc(size);

	field.direction = LEFT;

	field.head->p.x = xpos/2;
	field.head->p.y = ypos/2;
	field.head->next = NULL;

	field.tail->p.x = xpos/2;
	field.tail->p.y = ypos/2 + 1;
	field.tail->next = field.head;

	printf("\e[%d;%dH@", field.head->p.y, field.head->p.x);
	printf("\e[%d;%dH@", field.tail->p.y, field.tail->p.x);

	seed = time(NULL);
	genfood();
}

int
snake_mov(void)
{
	int size;
	struct snake *sp;

	sp = field.tail;
	field.tail = field.tail->next;
	printf("\e[%d;%dH ", sp->p.y, sp->p.x);
	free(sp);

	size = sizeof (struct snake);
	sp = field.head;

	sp->next = malloc(size);
	memcpy(sp->next, sp, size);

	field.head = sp->next;
	sp = field.head;
	sp->next = NULL;

	switch (field.direction) {
	case LEFT:
		--sp->p.x;
		break;

	case DOWN:
		++sp->p.y;
		break;

	case UP:
		--sp->p.y;
		break;

	case RIGHT:
		++sp->p.x;
		break;
	}

	if (sp->p.x < field.min.x || sp->p.x > field.max.x ||
	    sp->p.y < field.min.y || sp->p.y > field.max.y) {
		return 0;
	}
	else if (sp->p.x == field.food.x && sp->p.y == field.food.y) {
		sp = malloc(size);
		sp->next = field.tail;
		sp->p.x = sp->p.y = 1;
		field.tail = sp;

		genfood();
	}

	size = sizeof (struct coord);

	for (sp = field.tail; sp->next != NULL; sp = sp->next)
		if (!memcmp(&field.head->p, &sp->p, size))
			return 0;

	printf("\e[%d;%dH@", sp->p.y, sp->p.x);
	fflush(stdout);


	return 1;
}

int
main(void)
{
	char c;
	struct termios tty;
	struct termios ttynew;
	fd_set rfds;
	struct timeval tv;
	int ret;

	if (!isatty(0) || !isatty(1)) {
		fprintf (stderr, "stdin or stdout is not terminal\n");
		exit(EXIT_FAILURE);
	};

/*
 * Save settings, set non-canonical terminal,
 * clear terminal and make cursor invisible
 */
	tcgetattr(0, &ttynew);
	tty = ttynew;
	ttynew.c_lflag &= ~(ICANON | ECHO | ISIG);
	tcsetattr(0, TCSANOW, &ttynew);
	printf("\e[2J\e[?25l");

	init_fld();
	FD_ZERO(&rfds);
	tv.tv_sec = 0;

	while (1) {
		if (!snake_mov())
			break;

		FD_SET(0, &rfds);
		tv.tv_usec = 150000;

		//tcflush(0, TCIFLUSH); // Hard mode
		ret = select(1, &rfds, NULL, NULL, &tv);

		switch (ret) {
		case 0:
			continue;

		case -1:
			printf("Error\n");
			exit(1);
		}

		usleep(tv.tv_usec);

		read(1, &c, 1);

		if (c == 'q')
			break;

		switch (c) {
		case 'a':
		case 'h':
			if (field.direction != RIGHT)
				field.direction = LEFT;
			break;

		case 's':
		case 'j':
			if (field.direction != UP)
				field.direction = DOWN;
			break;

		case 'w':
		case 'k':
			if (field.direction != DOWN)
				field.direction = UP;
			break;

		case 'd':
		case 'l':
			if (field.direction != LEFT)
				field.direction = RIGHT;
			break;
		}
	}

/* Set old settings */
	printf("\e[2J\e[H\e[?25h");
	tcsetattr(0, TCSAFLUSH, &tty);

	return 0;
}
