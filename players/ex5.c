#include <stdio.h>
#include <stdlib.h>
#include <my-ipc.h>
#include <client-side.h>
#include <redundant.h>
#include <public.h>

const char MY_NAME[11] = "4877842727";
const char DEPLOYMENT[61] = "Ba3a4a5a6 Cc1c2c3 Cc5c6c7 De1e2 De4e5 De7e8 Sg1 Sg3 Sg5 Sg7 ";

enum ship {
  UNKNOWN,
  NOSHIP,
  ROCK,
  BSHIP,
  CSHIP,
  DSHIP,
  SSHIP
};

int cur_x,cur_y;
enum ship enemy_board[BD_SIZE+2][BD_SIZE+2];    // BD_SIZE is 9 (defined in public.h)

void respond_with_name(void)
{
  char str[MSG_LEN];  // MSG_LEN is 100 (defined in my-ipc.h)
  strcpy(str, MY_NAME);
  send_to_ref(str);
}

void respond_with_deployment(void)
{
  char str[MSG_LEN];  // MSG_LEN is 100 (defined in my-ipc.h)
  strcpy(str, DEPLOYMENT);
  send_to_ref(str);
}


void init_board(void)
{
  int ix,iy;
  //======kokokara======
for(ix = 0; ix < (BD_SIZE+2); ix++)
  {
    for(iy = 0; iy < (BD_SIZE+2); iy++)
    {
      //======kokokara======
      enemy_board[iy][ix]=UNKNOWN;
      //======kokomade======
    }
  }

  //rock is out of bound
  //======kokokara======
enemy_board[0][0]=enemy_board[0][1]=enemy_board[1][0]=enemy_board[BD_SIZE-1][BD_SIZE-1]
  =enemy_board[BD_SIZE-2][BD_SIZE-1]=enemy_board[BD_SIZE-1][BD_SIZE-2]=enemy_board[0][BD_SIZE-1]
  =enemy_board[1][BD_SIZE-1]=enemy_board[0][BD_SIZE-2]=enemy_board[BD_SIZE-1][0]=enemy_board[BD_SIZE-1][1]
  =enemy_board[BD_SIZE-2][0] = ROCK;
  //======kokomade======
}

void respond_with_shot(void)
{
  char shot_string[MSG_LEN];
  int x, y;
  
  while (TRUE)
  {
    x = rand() % BD_SIZE;
    y = rand() % BD_SIZE;
    //=====kokokara=====

    if(enemy_board[y][x]==UNKNOWN) break;

    //=====kokomade=====
  }
  printf("%s shooting at %d%d ... ", MY_NAME, x, y);
  sprintf(shot_string, "%d%d", x, y);
  send_to_ref(shot_string);
  cur_x = x;
  cur_y = y;
}

void record_result(int x,int y,char line[])
{
  if(line[13]=='B')
  {
    //====kokokara====
    enemy_board[y][x] = BSHIP;

    //====kokomade====
  }
  else if(line[13]=='C')
  {
    //====kokokara====
    enemy_board[y][x]=CSHIP;

    //====kokomade====
  }
  else if(line[13]=='D')
  {
    //====kokokara====
    enemy_board[y][x]=DSHIP;

    //====kokomade====
  }
  else if(line[13]=='S')
  {
    //====kokokara====
    enemy_board[y][x]=SSHIP;

    //====kokomade====
  }
  else if(line[13]=='R')
  {
    //====kokokara====
    enemy_board[y][x]=ROCK;

    //====kokomade====
  }
  else
  {
    //====kokokara====
    enemy_board[y][x]=NOSHIP;

    //====kokomade====
  }
}

void print_board(void){
  int ix, iy;
  printf("\n  ");

  for (ix = 0; ix < BD_SIZE + 2; ix++)
  {
    printf("%2d", ix);
  }
  printf("\n");

  for (iy = BD_SIZE + 1; iy >= 0; iy--)
  {
    printf("%2d ", iy);
    for (ix = 0; ix < BD_SIZE + 2; ix++)
    {
      switch(enemy_board[ix][iy])
      {
        case UNKNOWN:
          printf("U ");
          break;
        case NOSHIP:
          printf("N ");
          break;
        case ROCK:
          printf("R ");
          break;
        case BSHIP:
          printf("B ");
          break;
        case CSHIP:
          printf("C ");
          break;
        case DSHIP:
          printf("D ");
          break;
        case SSHIP:
          printf("S ");
          break;
        default:
          break;
      }
    }
    printf("\n");
  }
  printf("\n");
}

void handle_messages(void)
{
  char line[MSG_LEN];

  srand(getpid());
  init_board();
  
  while (TRUE)
  {
    receive_from_ref(line);

    if(message_has_type(line, "name?"))
    {
      respond_with_name(); 
    }
    else if(message_has_type(line, "deployment?"))
    {
       respond_with_deployment(); 
    }
    else if(message_has_type(line, "shot?"))
    {
      respond_with_shot(); 
    }
    else if(message_has_type(line, "shot-result:"))
    {
      record_result(cur_x,cur_y,line);
      printf("result: %c\n", line[13]);
      print_board();
    }
    else if(message_has_type(line, "end:"))
    {
      break;
    }
    else
    {
      printf("ignoring message: %s", line);
    }
  }
}

int main()
{
  client_make_connection();
  handle_messages();
  client_close_connection();
  return 0;
}
