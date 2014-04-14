#include <stdio.h>
#include <stdlib.h>
#include <my-ipc.h>
#include <client-side.h>
#include <redundant.h>
#include <public.h>

const char MY_NAME[11] = "sweep";
const char DEPLOYMENT[61] = "Ba3a4a5a6 Cc1c2c3 Cc5c6c7 De1e2 De4e5 De7e8 Sg1 Sg4 Sh7 Si3 ";

int pre_shot_x, pre_shot_y;

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

void respond_with_shot(void)
{
  char shot_string[MSG_LEN];
  int x, y;

  //=====kokokara========
  static int i = 1;
  static int j = 0;

  if(++i>8){i=0; j++;}
  if(j>8)j=0;
  

  x = i;
  y = j;

  //=====kokomade========

  printf("%s shooting at %d%d ... ", MY_NAME, x, y);
  sprintf(shot_string, "%d%d", x, y);
  send_to_ref(shot_string);
}

void handle_messages(void)
{
  char line[MSG_LEN];

  pre_shot_x = 0;
  pre_shot_y = 0;

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
