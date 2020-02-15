#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <poll.h>
#include <stdbool.h>
#include <time.h>

struct point
{
	int x;
	int y;
};

struct snake_node
{
	struct point pos;
	struct snake_node *next;
};


static volatile int go;
static size_t score;
static struct point cheese_loc;

static void stop(int signo)
{
	go = 0;
}

static void make_snake(struct snake_node *head);
static void print_snake(struct snake_node *head);
static void print_hud(void);
static void move_snake(struct snake_node *head, int dir);
static int check_snake(struct snake_node *head, const struct winsize *w);
static int get_arrow();
static bool same_loc(const struct point *a, const struct point *b);
static void place_cheese(const struct snake_node *head, const struct winsize *bounds);
static void print_cheese(void);

int main()
{
	struct winsize old, new;
	struct snake_node head;
	struct termios oldt, newt;
    srand(time(NULL));
	char *end_message=NULL;
	//set up
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &old);
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	go = 1;
    score = 0;
	signal(SIGINT, stop); 
	printf("\e[?25l"); //disable cursor	
	if(old.ws_col < 24 || old.ws_row < 12)
	{
		printf("Screen to small\n");
		exit(1);
	}
	
	head.pos.x = old.ws_col/2;
	head.pos.y = old.ws_row/2;
	head.next = NULL;
	make_snake(&head);
    place_cheese(&head, &old);
	char input;
	int dir = 0;
	struct pollfd poll_items[1] = {{fd: 0, events: POLLIN}};

	while(go)
	{
		ioctl(STDOUT_FILENO, TIOCGWINSZ, &new);
		if(old.ws_row != new.ws_row || old.ws_col != old.ws_col)
		{
			end_message = "YOU CHANGED THE SCREEN SIZE!!!\n";
			break;
		}
		if(poll(poll_items, 1, 0))
		{
			if(poll_items[0].revents == POLLIN)
			{
				input = getchar();
			}
			if(input == 'q')
			{
				end_message = "Good Game\n";
				break;
			}
			else if(input == 27)
			{
				int temp = get_arrow();
				if(temp >= 0 && temp < 4)
				{
					if(!(((dir == 0 || dir == 1) && (temp == 0 || temp ==1)) ||
								((dir == 2 || dir == 3) && (temp == 2 || temp == 3))))
						dir = temp;
				}
			}
		}

		move_snake(&head, dir);
        if(check_snake(&head, &old))
		{
			end_message = "Good Game\n";
			break;
		}
		printf("\e[2J");//clear screen
		print_snake(&head);
        print_hud();
        print_cheese();
		
		fflush(stdout); //flush stdout

		usleep(300000);
	}
	printf("\e[2J\e[0;0H\e[?25h"); //clear screen reset cursor pos and show cursor
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt);
    printf("Score: %zu\n", score);

	if(end_message != NULL)
    {
        printf("%s\n", end_message);
    }

	return 1;
}

void make_snake(struct snake_node *head)
{
	struct snake_node *temp, *curr = head;
	int i;
	for(i = 0; i < 3; i++)
	{
		temp = calloc(1, sizeof(struct snake_node));
		temp->pos.x = curr->pos.x;
		temp->pos.y = curr->pos.y + 1;
		temp->next = NULL;
		curr->next = temp;
		curr = temp;
	}
}

void print_snake(struct snake_node *head)
{
	if(head == NULL)
		return;
	printf("\e[%d;%dH#", head->pos.y, head->pos.x);
	print_snake(head->next);
}

void print_hud()
{
    printf("\e[0;0HScore: %zu", score);
}

void step_snake(struct snake_node *head, struct point move_too)
{
	if(head == NULL)
		return;
	struct point prev_pos = head->pos;
	head->pos = move_too;
	step_snake(head->next, prev_pos);
}

void move_snake(struct snake_node *head, int dir)
{
	struct point prev_pos;
	struct snake_node *curr = head;
	prev_pos = curr->pos;
	switch(dir)
	{
		case 0: //up
			prev_pos.y -= 1;
			break;
		case 1: //down
			prev_pos.y += 1;		      	
			break;
		case 2: //right
			prev_pos.x += 1;
			break;
		case 3: //left
			prev_pos.x -= 1;
			break;
	}
	step_snake(head, prev_pos);
}


int check_snake(struct snake_node *head, const struct winsize *w)
{
	struct snake_node *curr = head->next;
    if(head->pos.x <= 0 || head->pos.x >= w->ws_col ||
            head->pos.y <= 0 || head->pos.y >= w->ws_row)
	{
		return 1;
	}

    if(same_loc(&head->pos, &cheese_loc))
    {
        score += 1;
        place_cheese(head, w);
        struct snake_node *new_node = malloc(sizeof(struct snake_node));
        new_node->pos = head->pos;
        new_node->next = head->next;
        head->next = new_node;
    }
	
	while(curr != NULL)
    {
        if(same_loc(&head->pos, &curr->pos))
        {
            return 1;
        }
		curr = curr->next;
	}
	return 0;
}

int get_arrow()
{
	if (getchar() == 91)
	{
		int arrow = getchar();
		if(arrow >= 65 && arrow <=68)
			return arrow - 65;
		else
			return -1;
	}
	return -1;
}

bool same_loc(const struct point *a, const struct point *b)
{
    return (a->x == b->x) && (a->y == b->y);
}

void place_cheese(const struct snake_node *head, const struct winsize *bounds)
{
    const struct snake_node *curr;
    bool good = false;

    while(true)
    {
        cheese_loc.x = rand() % bounds->ws_col;
        cheese_loc.y = rand() % bounds->ws_row;

        curr = head;

        good = true;
        while(curr != NULL)
        {
            if(same_loc(&curr->pos, &cheese_loc))
            {
                good = false;
                break;
            }
            curr = curr->next;
        }

        if(good)
        {
            return;
        }
    }
}

void print_cheese(void)
{
    printf("\e[%d;%dH$", cheese_loc.y, cheese_loc.x);
}
