#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <my-ipc.h>
#include <client-side.h>
#include <redundant.h>
#include <public.h>

#define DEBUG(a,b) printf(#a" is %d, "#b" is %d\n", (a), (b))

const char MY_NAME[11] = "4877842727";
//const char DEPLOYMENT[61] = "Ba3a4a5a6 Cc1c2c3 Cc5c6c7 De1e2 De4e5 De7e8 Sg1 Sg3 Sg5 Sg7 ";
const char DEPLOYMENT[61] = "Bg2g3g4g5 Cc0c1c2 Ci2i3i4 Da2a3 Dd8e8 De0f0 Sa5 Sb7 Sg8 Si6 ";


enum ship {
  UNKNOWN,
  SUSPECT,
  NOSHIP,
  BSHIP,
  CSHIP,
  DSHIP,
  SSHIP,
  ROCK,

  OUTOFBOUND
};
enum direction{
  UNKNOWNDIR,
  COL,//yoko
  ROW //tate
};
enum Salvodiretion{
  UPRIGHTLOWER,
  UPRIGHTHIGHER,
  UPLEFTLOWER,
  UPLEFTHIGHER,
  DOWNRIGHTLOWER,
  DOWNRIGHTHIGHER,
  DOWNLEFTLOWER,
  DOWNLEFTHIGHER
}salvodirection;

int cur_x=0,cur_y=0;
enum ship enemy_board[BD_SIZE+2][BD_SIZE+2];   
// BD_SIZE is 9 (defined in public.h)



typedef struct{
  char x;
  char y;
  enum ship ship;
  enum direction direction;
}Unsank;
Unsank *unsank[4];
int numunsank=0;

int bsank[4][2]; int csank[2][3][2]; int dsank[3][2][2];
int numbsank=0,numcsank=0,numdsank=0;

enum{
  BTRAP,
  CTRAP,
  DTRAP
}mode = BTRAP;

void (*shotfunc)(int*, int*);

void shotrattrap(int *retx, int *rety);
void shotsalvo(int *retx, int *rety);
void shotrandom(int *retx, int *rety);
void shotsuspect(int *retx, int *rety);



//●●●●●●●●●●●●●●●●●ＵＴＩＬＩＴＹ●●●●●●●●●●●●●●●●●
#define SET(a,b,c) do{							\
    if(enemy_board[(c)+1][(b)+1] < a)enemy_board[(c)+1][(b)+1]=a;	\
  }while(0)    //横着したshipの配置。
#define GET(a,b) enemy_board[(b)+1][(a)+1]
enum ship get(int x, int y){
  if(x>=0&&y>=0&&x<BD_SIZE&&y<BD_SIZE)return enemy_board[y+1][x+1];
  else return OUTOFBOUND;
}


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

    i=j=BD_SIZE-offset;
    for(;j>=offset;--j){
      if(enemy_board[i+1][j+1]==sh){*retx=j,*rety=i;return 1;}
    }
    for(;i>=offset;--i){
      if(enemy_board[i+1][j+1]==sh){*retx=j,*rety=i;return 1;}
    }
  }
  return 0;
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






//#undef SET
//●●●●●●●●●ＩＮＩＴＩＡＬＩＺＥ●●●●●●●●●
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

void init_strategy(){
  shotfunc=shotsalvo;

  int i=rand()%8;  
  switch(i){
  case 0: salvodirection=UPRIGHTLOWER;cur_x=-1,cur_y=-3;break;
  case 1: salvodirection=UPRIGHTHIGHER;cur_x=-3,cur_y=-1;break;
  case 2: salvodirection=UPLEFTLOWER;cur_x=BD_SIZE,cur_y=-3;break;
  case 3: salvodirection=UPLEFTHIGHER;cur_x=BD_SIZE+2,cur_y=-1;break;
  case 4: salvodirection=DOWNRIGHTLOWER;cur_x=-3,cur_y=BD_SIZE;break;
  case 5: salvodirection=DOWNRIGHTHIGHER;cur_x=-1,cur_y=BD_SIZE+2;break;
  case 6: salvodirection=DOWNLEFTLOWER;cur_x=BD_SIZE+2,cur_y=BD_SIZE;break;
  case 7: salvodirection=DOWNLEFTHIGHER;cur_x=BD_SIZE,cur_y=BD_SIZE+2;break;
  }
  printf("\ni=%d",i);
  init_board();
}




//●●●●●●●●●●●●●●●●ＳＨＯＴＰＡＴＴＥＲＮ●●●●●●●●●●●●●●●●●●

void shotsalvo(int *retx, int *rety){
  static int lastsalvox=0;
  static int lastsalvoy=0;
  int x,y,dx,dy;
  static int salvocounter=0;
  int salvoindex;
  do{
    dx=(salvodirection>>1)%2 ? -1 : 1;
    dy=(salvodirection>>2)%2 ? -1 : 1;
  
    DEBUG(dx, dy);
    if(lastsalvox==0 && lastsalvoy==0){
      x = cur_x + dx*3;
      y = cur_y + dy*3;
    }else{
      x = lastsalvox + dx*3;
      y = lastsalvoy + dy*3;
    }
    DEBUG(x,y);
  
    while(GET(x,y)!=UNKNOWN){
      x+=2,y+=2;
      if(x<0||y<0||x>=BD_SIZE||y>=BD_SIZE) break;
    }
  
    if(x<0||y<0||x>=BD_SIZE||y>=BD_SIZE){
      salvocounter+=3;
      if(salvocounter>=18)shotfunc = shotrandom; 
  
      salvoindex=(salvodirection+3)%8;
      switch(salvoindex){
      case 0: salvodirection=UPRIGHTLOWER;lastsalvox=-1,lastsalvoy=-3;break;
      case 1: salvodirection=UPRIGHTHIGHER;lastsalvox=-3,lastsalvoy=-1;break;
      case 2: salvodirection=UPLEFTLOWER;lastsalvox=BD_SIZE,lastsalvoy=-3;break;
      case 3: salvodirection=UPLEFTHIGHER;lastsalvox=BD_SIZE+2,lastsalvoy=-1;break;
      case 4: salvodirection=DOWNRIGHTLOWER;lastsalvox=-3,lastsalvoy=BD_SIZE;break;
      case 5: salvodirection=DOWNRIGHTHIGHER;lastsalvox=-1,lastsalvoy=BD_SIZE+2;break;
      case 6: salvodirection=DOWNLEFTLOWER;lastsalvox=BD_SIZE+2,lastsalvoy=BD_SIZE;break;
      case 7: salvodirection=DOWNLEFTHIGHER;lastsalvox=BD_SIZE,lastsalvoy=BD_SIZE+2;break;
      }
   
    }
  }while(GET(x,y)!=UNKNOWN||x<0||y<0||x>=BD_SIZE||y>=BD_SIZE);
  lastsalvox=x, lastsalvoy=y;
  *retx=x, *rety=y;
}
void shotrattrap(int *retx, int *rety){
  int x,y;
  while (TRUE)
    {
      x = rand() % BD_SIZE;
      y = rand() % BD_SIZE;
      
      if(enemy_board[y+1][x+1]==UNKNOWN
	 && (x+y)%2==0) break;
    }
  *retx = x, *rety = y;
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
void shotselect(){

  if(mode==BTRAP&&numbsank==1)mode=CTRAP;
  if(mode==CTRAP&&numcsank==2)mode=DTRAP;
  if(shotfunc==shotrattrap&& mode==DTRAP&&numdsank==3)shotfunc = shotrandom;;


}

void respond_with_shot(void)
{
  char shot_string[MSG_LEN];
  int x, y;

  if(numunsank>0){shotsuspect(&x, &y);}
  else{
    shotselect();
    (*shotfunc)(&x, &y);  
  }

  if(shotfunc==shotsalvo)printf("\n-----\n  %d\n-----\n", salvodirection);

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
  printf("numunsank is %d\n", numunsank);
  switch(numunsank){
  case 2: //dsank
    DEBUG(numdsank, numunsank);
    dsank[numdsank][0][0]=unsank[0]->x,dsank[numdsank][0][1]=unsank[0]->y;
    dsank[numdsank][1][0]=unsank[1]->x,dsank[numdsank][1][1]=unsank[1]->y;
    numdsank++;
    break;
  case 3: //csank
    DEBUG(numcsank, numunsank);
    csank[numcsank][0][0]=unsank[0]->x,csank[numcsank][0][1]=unsank[0]->y;
    csank[numcsank][1][0]=unsank[1]->x,csank[numcsank][1][1]=unsank[1]->y;
    csank[numcsank][2][0]=unsank[2]->x,csank[numcsank][2][1]=unsank[2]->y;
    numcsank++;
    break;
  case 4: //bsank
    DEBUG(numbsank,numunsank);
    bsank[0][0]=unsank[0]->x,bsank[0][1]=unsank[0]->y;
    bsank[1][0]=unsank[1]->x,bsank[1][1]=unsank[1]->y;
    bsank[2][0]=unsank[2]->x,bsank[2][1]=unsank[2]->y;
    bsank[3][0]=unsank[3]->x,bsank[3][1]=unsank[3]->y;
    numbsank++;
    break;
  default:
    puts("INVALID NUMUNSANK");
    return;
  }

  int i;
  for(i=0;i<numunsank;++i){
    //printf("set_board_around,i=%d\n", i);
    set_board_around(NOSHIP, unsank[i]->x, unsank[i]->y);
  }
  for(i=0;i<numunsank;++i){
    //printf("free, i=%d\n",i);
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



//●●●●●●●●●●ＢＡＳＥ●●●●●●●●●

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

void record_result(int x,int y,char line[])
{
  if(line[13]=='B')
  {
    //====kokokara====
    enemy_board[y+1][x+1] = BSHIP;

    if(get(x-1, y)==NOSHIP&&
       get(x+1,y)==UNKNOWN&&((get(x+2,y)!=UNKNOWN)||(get(x+2, y)==UNKNOWN&&get(x+3,y)!=UNKNOWN))) SET(NOSHIP,x+1,y);
    else if(get(x+1, y)==NOSHIP&&
       get(x-1,y)==UNKNOWN&&((get(x-2,y)!=UNKNOWN)||(get(x-2, y)==UNKNOWN&&get(x-3,y)!=UNKNOWN))) SET(NOSHIP,x-1,y);
    else if(get(x-1,y)==UNKNOWN&&get(x+1,y)==UNKNOWN&&get(x-2,y)!=UNKNOWN&&get(x+2,y)!=UNKNOWN){SET(NOSHIP, x-1,y); SET(NOSHIP, x+1,y);}
    if(get(x, y-1)==NOSHIP&&
       get(x,y+1)==UNKNOWN&&((get(x,y+2)!=UNKNOWN)||(get(x, y+2)==UNKNOWN&&get(x,y+3)!=UNKNOWN))) SET(NOSHIP,x,y+1);
    else if(get(x, y+1)==NOSHIP&&
       get(x,y-1)==UNKNOWN&&((get(x,y-2)!=UNKNOWN)||(get(x, y-2)==UNKNOWN&&get(x,y-3)!=UNKNOWN))) SET(NOSHIP,x,y-1);
    else if(get(x,y-1)==UNKNOWN&&get(x,y+1)==UNKNOWN&&get(x,y-2)!=UNKNOWN&&get(x,y+2)!=UNKNOWN){SET(NOSHIP, x,y-1); SET(NOSHIP, x,y+1);}

    //====kokomade====
  }
  else if(line[13]=='C')
  {
    //====kokokara====
    enemy_board[y+1][x+1]=CSHIP;

    if(get(x-1,y)==NOSHIP&&get(x+1,y)==UNKNOWN&&get(x+2, y)!=UNKNOWN)
      SET(NOSHIP, x+1, y);
    else if(get(x-1,y)==UNKNOWN&&get(x+1,y)==NOSHIP&&get(x-2, y)!=UNKNOWN)
      SET(NOSHIP, x-1, y);
    if(get(x,y-1)==NOSHIP&&get(x,y+1)==UNKNOWN&&get(x, y+2)!=UNKNOWN)
      SET(NOSHIP, x, y+1);
    else if(get(x,y-1)==UNKNOWN&&get(x,y+1)==NOSHIP&&get(x, y+2)!=UNKNOWN)
      SET(NOSHIP, x, y-1);

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
  init_strategy();

  
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
