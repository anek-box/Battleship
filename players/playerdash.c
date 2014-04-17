#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <my-ipc.h>
#include <client-side.h>
#include <redundant.h>
#include <public.h>

const char MY_NAME[11] = "9999999999";
const char DEPLOYMENT[61] = "Ba3a4a5a6 Cc1c2c3 Cc5c6c7 De1e2 De4e5 De7e8 Sg1 Sg3 Sg5 Sg7 ";

enum ship {
  UNKNOWN,
  SUSPECT,
  NOSHIP,
  BSHIP,
  CSHIP,
  DSHIP,
  SSHIP,
  ROCK
};
enum direction{
  UNKNOWNDIR,
  COL,//yoko
  ROW //tate
};


int cur_x=0,cur_y=0;
enum ship enemy_board[BD_SIZE+2][BD_SIZE+2];   
// BD_SIZE is 9 (defined in public.h)

#define SET(a,b,c) do{						\
    if(enemy_board[(c)+1][(b)+1] < a)enemy_board[(c)+1][(b)+1]=a;	\
  }while(0)

typedef struct{
  char x;
  char y;
  enum ship ship;
  enum direction direction;
}Unsank;
Unsank *unsank[4];
int numunsank=0;


//●●●●●●●●●●●●●●●●●ＳＥＴ●●●●●●●●●●●●●●●●●
//指定マスの上下左右をｓｈにセットする
void set_board_crossy(enum ship sh, int x, int y){
  int i, j,ittr;
  for(ittr=1;ittr<9;ittr+=2){
    i=ittr/3 - 1; j=ittr%3 - 1;
    if(enemy_board[y+i+1][x+j+1] < sh)enemy_board[y+i+1][x+j+1] = sh;
  }
}
//指定マスの周囲8マスをshにセット
void set_board_around(enum ship sh, int x, int y){
  int i, j,ittr;
  for(ittr=0;ittr<9;ittr++){
    i=ittr/3 - 1; j=ittr%3 - 1;
    if(enemy_board[y+i+1][x+j+1] < sh)enemy_board[y+i+1][x+j+1] = sh;
  }
}
//指定マスの上下または左右をセット ＊＊＊仕様を確定させてないので使わない
/*void set_board_row_or_col(enum ship sh, int x, int y, enum direction dir, int overwriteflag){
  if(dir==ROW){
    if(enemy_board[y+2][x+1]==NOSHIP || enemy_board[y+2][x+1] < sh)enemy_board[y+2][x+1] = sh;
    if(overwriteflag || enemy_board[y][x+1] < sh)enemy_board[y][x+1] = sh;
  }else{
    if(overwriteflag || enemy_board[y+1][x+2] < sh)enemy_board[y+1][x+2] = sh;
    if(overwriteflag || enemy_board[y+1][x] < sh)enemy_board[y+1][x] = sh;
  }
  }*/

//ボードの周囲から目標のshipのセルを探す。グルグル渦を巻いて中心に向かう
int  search_spiral(enum ship sh, int offset, int *retx, int *rety){
  int i,j;
  for(;offset<BD_SIZE+1;offset++){
    i=j=offset;

    for(;j<BD_SIZE-offset;++j){
      if(enemy_board[i+1][j+1]==sh){*retx=j,*rety=i;return 1;}
    }
    for(;i<BD_SIZE-offset;++i){
      if(enemy_board[i+1][j+1]==sh){*retx=j,*rety=i;return 1;}
    }
    for(;j>=offset;--j){
      if(enemy_board[i+1][j+1]==sh){*retx=j,*rety=i;return 1;}
    }
    for(;i>=offset;--i){
      if(enemy_board[i+1][j+1]==sh){*retx=j,*rety=i;return 1;}
    }
  }
  return 0;
}


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
      enemy_board[iy][ix]=NOSHIP;
      //======kokomade======
    }
  }
for(ix = 1; ix < (BD_SIZE+1); ix++)
  {
    for(iy = 1; iy < (BD_SIZE+1); iy++)
    {
      //======kokokara======
      enemy_board[iy][ix]=UNKNOWN;
      //======kokomade======
    }
  }

  //rock is out of bound
  //======kokokara======
enemy_board[1][1]=enemy_board[1][2]=enemy_board[2][1]=enemy_board[BD_SIZE][BD_SIZE]
  =enemy_board[BD_SIZE-1][BD_SIZE]=enemy_board[BD_SIZE][BD_SIZE-1]=enemy_board[1][BD_SIZE]
  =enemy_board[2][BD_SIZE]=enemy_board[1][BD_SIZE-1]=enemy_board[BD_SIZE][1]=enemy_board[BD_SIZE][2]
  =enemy_board[BD_SIZE-1][1] = ROCK;
  //======kokomade======
}

enum ship aroundship(int x, int y){
  enum ship ret;
#define ISSHIP(x, y)			  \
  do {					  \
    ret=enemy_board[y+1][x+1];		  \
    if(ret==BSHIP			  \
       || ret==CSHIP			  \
       || ret==DSHIP) return ret; }	  \
  while(0)

  ISSHIP(x-1,y-1);ISSHIP(x,y-1);ISSHIP(x+1,y-1);
  ISSHIP(x-1,y);  /* (^q^)  */  ISSHIP(x+1,y);
  ISSHIP(x-1,y+1);ISSHIP(x,y+1);ISSHIP(x+1,y+1);
#undef ISSHIP
  return UNKNOWN;
}

void shotsalvo(int *retx, int *rety){
  
}
void shotrattrap(int *retx, int *rety){
  
}
void shotrandom(int *retx, int *rety){
  int x,y;
  while (TRUE)
  {
    x = rand() % BD_SIZE;
    y = rand() % BD_SIZE;
    
    if(enemy_board[y+1][x+1]==UNKNOWN
       || enemy_board[y+1][x+1]==SUSPECT) break;
  }
  *retx = x, *rety = y;
}
void shotsuspect(int *retx, int *rety){
  int i=0,j=0;
  search_spiral(SUSPECT, 0, &j, &i);
  *retx=j;*rety=i;
}

void (*shotfunc)(int*, int*)=shotrandom;

void shotselecter(){
  if(numunsank>0) shotfunc = shotsuspect;
  else shotfunc = shotrandom;
}

void respond_with_shot(void)
{
  char shot_string[MSG_LEN];
  int x, y;
  
  shotselecter();
  (*shotfunc)(&x, &y);  

  printf("%s shooting at %d%d ... ", MY_NAME, x, y);
  sprintf(shot_string, "%d%d", x, y);
  send_to_ref(shot_string);
  cur_x = x;
  cur_y = y;
}


//●●●●●●●●●●●●●●●●ＵＮＳＡＮＫ●●●●●●●●●●●●●●●●●

void pushunsank(enum ship sh, int x, int y){
  if((unsank[numunsank] = (Unsank*)malloc(sizeof(Unsank)))==NULL){
    puts("malloc error");
    return;
  }
  unsank[numunsank]->x=x;
  unsank[numunsank]->y=y;
  unsank[numunsank]->ship=sh;
  numunsank++;
  set_board_crossy(SUSPECT,x,y);
  printf("unsank[%d] is on %p\n", numunsank-1, unsank[numunsank-1]);
}
void clearunsank(){
  int i;
  for(i=0;i<numunsank;++i){
    printf("set_board_around,i=%d\n", i);
    set_board_around(NOSHIP, unsank[i]->x, unsank[i]->y);
  }
  for(i=0;i<numunsank;++i){
    printf("free, i=%d\n",i);
    free(unsank[i]);
  }
  numunsank=0;
}
void judgedirection(){
  if(numunsank<2)return;

  int i,x,y;
  for(i=0;i<numunsank;++i){
    x=unsank[i]->x,y=unsank[i]->y;
    if(unsank[0]->x == unsank[1]->x){//タテ
      SET(NOSHIP, x-1, y-1);SET(NOSHIP, x-1, y);SET(NOSHIP, x-1, y+1);
      SET(NOSHIP, x+1, y-1);SET(NOSHIP, x+1, y);SET(NOSHIP, x+1, y+1);
    }else{                           //ヨコ   
      SET(NOSHIP, x-1, y-1);SET(NOSHIP, x, y-1);SET(NOSHIP, x+1, y-1);
      SET(NOSHIP, x-1, y+1);SET(NOSHIP, x, y+1);SET(NOSHIP, x+1, y+1);
    }
  }
}

void unsankmanage(){
  switch(enemy_board[cur_y+1][cur_x+1]){
  case BSHIP:
    puts("\nBSHIP\n");
    pushunsank(DSHIP, cur_x, cur_y);
    if(numunsank==4){
      puts("clearunsank");
      clearunsank();
    }
    break;
  case CSHIP:
    puts("\nCSHIP\n");
    pushunsank(CSHIP, cur_x, cur_y);
    if(numunsank==2) judgedirection();
    else if(numunsank==3){
      puts("clearunsank");
      clearunsank();
    }
    break;
  case DSHIP:
    puts("\nDSHIP\n");
    pushunsank(DSHIP, cur_x, cur_y);
    if(numunsank==2){
      puts("clearunsank");
      clearunsank();
    }
    break;
  default:
    return;
  }
}

void record_result(int x,int y,char line[])
{
  if(line[13]=='B')
  {
    //====kokokara====
    enemy_board[y+1][x+1] = BSHIP;

    //====kokomade====
  }
  else if(line[13]=='C')
  {
    //====kokokara====
    enemy_board[y+1][x+1]=CSHIP;

    //====kokomade====
  }
  else if(line[13]=='D')
  {
    //====kokokara====
    enemy_board[y+1][x+1]=DSHIP;

    //====kokomade====
  }
  else if(line[13]=='S')
  {
    //====kokokara====
    set_board_around(NOSHIP, x, y);

    enemy_board[y+1][x+1]=SSHIP;

    //====kokomade====
  }
  else if(line[13]=='R')
  {
    //====kokokara====
    enemy_board[y+1][x+1]=ROCK;

    //====kokomade====
  }
  else
  {
    //====kokokara====
    enemy_board[y+1][x+1]=NOSHIP;

    //====kokomade====
  }
  unsankmanage();
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
      switch(enemy_board[iy][ix])
      {
        case UNKNOWN:
	  printf("U ");
	  break; 
        case SUSPECT:
          printf("s ");
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
