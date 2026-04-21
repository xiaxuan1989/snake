#include <graphics.h>

#define MAXMAP 22
#define LEFT 293
#define UP 294
#define RIGHT 295
#define DOWN 296
#define ENTER 13
#define PAUSE 32
#define ESC 27
#define COLOR random(255)

//地图结构
struct Map
{
	int x;
	int y;
}Map;

//蛇结构
struct SnakeNode
{
	int x;
	int y;
	int direction;
}Snake[400];

//食物结构
struct FoodNode		
{
	int x;
	int y;
}Food;

void InitGame();
void StartGame();
void DrawMap();
void DrawSnake();
void LastDied();
void SnakeUp();
int GetKey();
void SnakeMove();
void CleanSnake();
void DrawFood();
void EatFood();
void Died();
void Modules();
void StModules();

int n = 3;						//蛇体长度
int score = 0;					//分数
int level = 1;					//等级
int total_games = 0;			//总游戏次数
int high_score = 0;				//最高分
int high_level = 0;				//最高等级
char *level_title = "[Newbie]";		//称号
bool havefood = false;			//地图是否已有食物
bool regame = false;			//按ENTER重新游戏时标记重新开始游戏true

//主函数
int main( void )
{
	do{
		InitGame();				//初始化地图和蛇
		StartGame();			//开始游戏
	}while (regame == true);
	
	getch();					//暂停
    closegraph();				//关闭绘图区域
	return 0;
}

//蛇根据方向前进
void SnakeUp()
{
	for (int i = n - 1; i > 0; i--)
	{
		Snake[i].x = Snake[i-1].x;
		Snake[i].y = Snake[i-1].y;
	}
	if (Snake[0].direction == RIGHT)
		Snake[0].x += 30;
	else if (Snake[0].direction == LEFT)
		Snake[0].x -= 30;
	else if (Snake[0].direction == UP)
		Snake[0].y -= 30;
	else if	(Snake[0].direction == DOWN)
		Snake[0].y += 30;
}

//绘制蛇体
void DrawSnake()
{
	for (int i = 1; i < n; i++)
	{
		setfillstyle( SOLID_FILL, RGB(COLOR, COLOR, COLOR) );	//随机改变每节蛇身颜色
		bar( Snake[i].x, Snake[i].y, Snake[i].x + 29, Snake[i].y + 29 );
	}
	setcolor(RGB(230, 230, 230));
	rectangle( Snake[0].x, Snake[0].y, Snake[0].x + 29, Snake[0].y + 29 );	//蛇头边框
	setfillstyle( SOLID_FILL, RGB(0, 0, 255) );
	bar ( Snake[0].x+1, Snake[0].y+1, Snake[0].x + 28, Snake[0].y + 28 );	//蛇头填充色
}

//擦掉蛇尾
void LastDied()
{
	int i = n - 1;
	setfillstyle( SOLID_FILL, RGB(54, 54, 54) );
	bar( Snake[i].x, Snake[i].y, Snake[i].x + 29, Snake[i].y + 29 );
}

//清除蛇体
void CleanSnake()
{
	setfillstyle( SOLID_FILL, RGB(54, 54, 54) );
	for (int i = 0; i < n; i++)
		bar( Snake[i].x, Snake[i].y, Snake[i].x + 29, Snake[i].y + 29 );
}

//获取键码
int GetKey()
{
	int ch = 0;
	int ch2 = 0;
	if (kbhit())	//检测否有键盘字符输入，有返回1，无返回0
	{
		ch = getch();
		if (ch == UP || ch == DOWN || ch == LEFT || ch == RIGHT)	//判断输入是否方向
		{
			if ( (Snake[0].direction - ch) != 2 && (Snake[0].direction - ch) != -2 && Snake[0].direction != ch )	//除掉反方向和原方向
				{
					Snake[0].direction = ch;
					return 1;
				}
		}
		else if (ch == ESC)			//按ESC退出游戏
			exit(1);
		else if (ch == PAUSE)		//按SPACE暂停游戏，再按一次继续
		{
			do{
				ch2 = getch();
			}
			while (ch2 != PAUSE);
		}
		else if (ch == ENTER)		//按ENTER返回ENTER重新开始游戏
		{
			total_games++;				//游戏次数+1
			high_score = score > high_score ? score : high_score;	//保存最高分
			score = 0;					//分数归零
			high_level = level > high_level ? level : high_level;	//保存最高等级
			level = 1;					//等级归零
			n = 3;						//蛇长度初始化
			return ENTER;
		}			
	}
	return 0;
}

//变向时蛇坐标更新
void SnakeMove()
{
	int TempX = Snake[0].x;	//原蛇头坐标保存
	int TempY = Snake[0].y;	
	switch (Snake[0].direction)	//更新蛇头坐标
	{
	case LEFT:
		Snake[0].x -= 30;
		break;
	case UP:
		Snake[0].y -= 30;
		break;
	case RIGHT:
		Snake[0].x += 30;
		break;
	case DOWN:
		Snake[0].y += 30;
		break;
	default:
		break;
	}

	for (int i = n - 1; i > 1; i--)	//	除蛇头外蛇体坐标更新
	{
		Snake[i].x = Snake[i-1].x;
		Snake[i].y = Snake[i-1].y;
	}
	Snake[1].x = TempX;	//原蛇头坐标给蛇脖子
	Snake[1].y = TempY;	
}

//绘制食物
void DrawFood()
{
	if (!havefood)
	{
		Food.x = 0;
		Food.y = 0;
		int x = 0;
		int y = 0;
		int i = 0;
		randomize();
		do
		{
			x = random(MAXMAP + 1) * 30;
			y = random(MAXMAP + 1) * 30;
			if (0 == x || 0 == y)
				continue;
			for (i = 0; i < n; i++)
				if (x == Snake[i].x && y == Snake[i].y)
					break;			
		}while (i != n);		//当for遍历判断没有循环到最后就重新外循环			
		Food.x = x;
		Food.y = y;
		setcolor(GREEN);
		setfillstyle( SOLID_FILL, RGB(0, 255, 0) );
		fillellipse(Food.x + 14, Food.y + 14, 14, 14);
		havefood = true;
	}
}

//吃食物检测
void EatFood()
{
	if (Food.x == Snake[0].x && Food.y == Snake[0].y)	//如果食物和蛇头重合
	{		
		setfillstyle( SOLID_FILL, RGB(54, 54, 54) );	//擦掉食物
		bar(Food.x, Food.y, Food.x + 29, Food.y + 29);
		n++;											//蛇增加一节
		havefood = false;								//吃到食物后标记havefood为地图没有食物
		Snake[n-1].x = Snake[n-2].x;					//新尾部坐标初始化为原尾部坐标（重要！）
		Snake[n-1].y = Snake[n-2].y;
		score += 100;
		level++;			
	}
}

//碰撞检测
void Died()
{
	int ch;
	setcolor(RGB(0, 0, 255));
	settextjustify(LEFT_TEXT, TOP_TEXT);
	for (int i = 3; i < n; i++)				//检测咬自己
	{
		if (Snake[0].x == Snake[i].x && Snake[0].y == Snake[i].y)
		{
			xyprintf(740, 450, "Game Over");
			delay_ms(500);
			xyprintf(740, 470, "You ware bit yourself");
			delay_ms(500);
			xyprintf(740, 490, "[Esc]Exit");
			xyprintf(740, 510, "[Enter]Restart");
			do{
				ch = getch();
			}while (ch !=ESC && ch !=ENTER);//禁止ESC和ENTER以外的按键
			if (ch == ESC)					//按ESC退出游戏
				exit(1);
			else if (ch == ENTER)			//按ENTER重新开始游戏
			{
				regame = true;				//标记重新游戏true
				total_games++;				//游戏次数+1
				high_score = score > high_score ? score : high_score;	//保存最高分
				score = 0;					//分数归零
				high_level = level > high_level ? level : high_level;	//保存最高等级
				level = 1;					//等级归零
				n = 3;						//蛇长度初始化
				level_title = "[Newbie]";		//称号初始化
				break;
			}
			else
				break;						//按其它键终止咬自己检测，开始撞墙检测
		}
	}
	if (!regame)
	{
		if (Snake[0].x == 0 || Snake[0].y == 0 || Snake[0].x == 690 || Snake[0].y == 690)	//撞墙检测
		{
			xyprintf(740, 450, "Game Over");
			delay_ms(500);
			xyprintf(740, 470, "Up against the wall");
			delay_ms(500);
			xyprintf(740, 490, "[Esc]Exit");
			xyprintf(740, 510, "[Enter]Restart");
			do{
				ch = getch();
			}while (ch !=ESC && ch !=ENTER);//禁止ESC和ENTER以外的按键
			if (ch == ESC)
				exit(1);
			else if (ch == ENTER)
			{
				regame = true;				//标记重新游戏true
				total_games++;				//游戏次数+1
				high_score = score > high_score ? score : high_score;	//保存最高分
				score = 0;					//分数归零
				high_level = level > high_level ? level : high_level;	//保存最高等级
				level = 1;					//等级归零
				n = 3;						//蛇长度初始化
				level_title = "[Newbie]";		//称号初始化
			}
		}
	}
}

//绘制地图
void DrawMap()
{
	Map.x = 29;
	Map.y = 29;
	int i;
	int j;
	setcolor(WHITE);
	rectangle(Map.x - 1, Map.y - 1, Map.x + 662, Map.y + 662);
	for (i = 30; i <= MAXMAP * 30; i += 30)
		for (j = 30; j <= MAXMAP * 30; j += 30)
		{
			Map.x = i;
			Map.y = j;
			if (Map.x != 0 || Map.y != 0)
			{
				setcolor(RGB (10, 10, 10));
				setfillstyle( SOLID_FILL, RGB(54, 54, 54) );
				rectangle(Map.x, Map.y, Map.x + 30, Map.y + 30);
				bar(Map.x, Map.y, Map.x + 29, Map.y + 29);
			}
		}
}

//初始化游戏
void InitGame()
{
	initgraph( 920, 720 );				//初始化绘图窗口
	setcaption("Snake");				//窗口标题
	Snake[0].x = 120;					//初始化3个长度蛇的坐标
	Snake[0].y = 120;
	Snake[0].direction = RIGHT;
	Snake[1].x = Snake[0].x - 30;
	Snake[1].y = Snake[0].y;	
	Snake[2].x = Snake[1].x - 30;
	Snake[2].y = Snake[1].y;
	StModules();						//静态模块
	DrawMap();							//初始化地图
	DrawSnake();						//初始化蛇
}

//开始游戏
void StartGame()
{
	regame = false;
	havefood = false;
	while(true)
	{
		Modules();						//右方模块
		DrawFood();						//绘制食物
		if (1 == GetKey())				//获得方向键值
		{
			LastDied();					//擦除蛇尾
			SnakeMove();				//变向时蛇坐标更新
		}
		else if (ENTER == GetKey())		//重新开始游戏
		{
			regame = true;				//标记重新游戏true
			break;
		}
		else							//获得非方向键测正常移动
		{
			LastDied();					//擦除蛇尾					
			SnakeUp();					//向蛇头方向移动
		}
		Died();							//每次改变蛇位置进行碰撞检测
		if (regame == true)
			break;
		EatFood();						//如果吃到食物蛇变长并重绘食物
		DrawSnake();					//绘制蛇
		delay_ms(300 - n*3);			//延时xxx毫秒，根据蛇长度增加蛇移动速度		
	}
}

//模块
void Modules()
{
	if (level > 9 && level < 20)
		level_title = " [ Good ]";
	else if (level >= 20 && level < 30)
		level_title = "[Awesome]";
	else if (level >= 30 && level < 40)
		level_title = "   [Perfect]";
	else if (level >= 40 && level < 50)
		level_title = " [Kichiku]";
	else if (level >= 50 && level < 60)
		level_title = "[Powerful]";
	else if (level >= 60 && level < 70)
		level_title = " [Level Up]";
	else if (level >= 70 && level < 80)
		level_title = "[Kingsman]";
	else if (level >= 80 && level < 90)
		level_title = " [No Words]";
	setfont(20, 8 , "微软雅黑");
	setcolor(WHITE);
	settextjustify(RIGHT_TEXT, TOP_TEXT);
	xyprintf(880, 30, "%d", score);
	xyprintf(880, 50, "%d", level);
	xyprintf(880, 70, "%8.2f", getfps());
	xyprintf(880, 90, "%8.1fs", fclock());
	setcolor(LIGHTGREEN);
	settextjustify(RIGHT_TEXT, TOP_TEXT);
	xyprintf(880, 410, "%s", level_title);
}

//静态模块
void StModules()
{
	setfont(20, 8 , "微软雅黑");
	settextjustify(LEFT_TEXT, TOP_TEXT);
	setcolor(WHITE);
	xyprintf(740, 30, "Score");
	xyprintf(740, 50, "Level");
	xyprintf(740, 70, "Fps");
	xyprintf(740, 90, "Time");
	xyprintf(740, 110, "Board");
	xyprintf(740, 170, "Quit");
	xyprintf(740, 190, "Restart");
	xyprintf(740, 210, "Pause");
	xyprintf(740, 230, "Left");
	xyprintf(740, 250, "Up");
	xyprintf(740, 270, "Right");
	xyprintf(740, 290, "Down");
	xyprintf(740, 330, "Total Games");
	xyprintf(740, 350, "Hi-Score");
	xyprintf(740, 370, "hI-Level");
	xyprintf(740, 410, "Medal");	
	xyprintf(700, 678, "(C) Eisenberg Andrew Roland");
	settextjustify(RIGHT_TEXT, TOP_TEXT);	
	xyprintf(880, 110, "22 × 22");
	xyprintf(880, 170, "Esc");
	xyprintf(880, 190, "Enter");
	xyprintf(880, 210, "Space");
	xyprintf(880, 230, "←");
	xyprintf(880, 250, "↑");
	xyprintf(880, 270, "→");
	xyprintf(880, 290, "↓");
	xyprintf(880, 330, "%d", total_games);
	xyprintf(880, 350, "%d", high_score);
	xyprintf(880, 370, "%d", high_level);
}