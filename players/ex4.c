#include <stdio.h>
#include <stdlib.h>
#include <my-ipc.h>
#include <client-side.h>
#include <redundant.h>
#include <public.h>

const char MY_NAME[11] = "memory";
const char DEPLOYMENT[61] = "Ba3a4a5a6 Cc1c2c3 Cc5c6c7 De1e2 De4e5 De7e8 Sg1 Sg3 Sg5 Sg7 ";

int enemy_board[BD_SIZE+2][BD_SIZE+2];    // BD_SIZE is 9 (defined in public.h)

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

void init_board(void){
  int ix, iy;

  for(ix = 0; ix < (BD_SIZE+2); ix++)
  {
    for(iy = 0; iy < (BD_SIZE+2); iy++)
    {
      //======kokokara======
      enemy_board[iy][ix]=1;
      //======kokomade======
    }
  }

  //rock is out of bound
  //======kokokara======
enemy_board[0][0]=enemy_board[0][1]=enemy_board[1][0]=enemy_board[BD_SIZE-1][BD_SIZE-1]
  =enemy_board[BD_SIZE-2][BD_SIZE-1]=enemy_board[BD_SIZE-1][BD_SIZE-2]=enemy_board[0][BD_SIZE-1]
  =enemy_board[1][BD_SIZE-1]=enemy_board[0][BD_SIZE-2]=enemy_board[BD_SIZE-1][0]=enemy_board[BD_SIZE-1][1]
  =enemy_board[BD_SIZE-2][0] = 0;

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
    //=====kokokara====
    if(enemy_board[y][x])break;
    

    //=====kokomade=====
  }
  enemy_board[y][x]=0;
  printf("%s shooting at %d%d ... ", MY_NAME, x, y);
  sprintf(shot_string, "%d%d", x, y);
  send_to_ref(shot_string);
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
      printf("result: %c\n", line[13]);
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
