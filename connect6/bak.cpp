/* 初版程序和几个有想法的评估函数 */
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
//#include <conio.h>
#include <vector>
#include <queue>
#include <string.h>
//#include <windows.h>
#include <fstream>
#include <termio.h>
using namespace std;
enum paint_char
{
	black = 1,
	white = 2,
	_empty = -1,
	nempty = -2, //updata_e_n检视过的空
	side = 3,
	sum = 3
};
enum vertion
{
	player,
	computer
};
const int chessbored_size = 21;					  //棋盘尺寸
int chessbored[chessbored_size][chessbored_size]; //棋盘
int cursor_x, cursor_y;							  //光标坐标
int player_color = black;						  //玩家棋子颜色
int computer_color = white;						  //电脑棋子颜色
const int max2 = 2147483647, fen_max = 99999999;
const int min2 = -2147483647, fen_min = -99999999;
int wide = 4, depth = 6;											 //搜索宽度、深度											 //搜索的深度和广度
int regret_array[180][chessbored_size][chessbored_size], regret_num; //悔棋记录数组
struct node
{
	int i, j;
	int value;
	int c[2][4]; //各方向形成的连接的子数
	friend bool operator<(node a, node b)
	{
		return a.value < b.value;
	}
	friend bool operator>(node a, node b)
	{
		return a.value > b.value;
	}
};
struct sixnode
{
	int x1, y1, x2, y2;
};
//priority_queue< node,vector<node>,less<node> > empty_player;
//priority_queue< node,vector<node>,less<node> > empty_player;
vector<node> empty_node;			 //已下棋子周围的空点集
vector<node> regret_empty_node[180]; //悔棋
int szfz(int a[chessbored_size][chessbored_size], int b[chessbored_size][chessbored_size])
{
	int i, j;
	for (i = 0; i < chessbored_size; i++)
		for (j = 0; j < chessbored_size; j++)
			b[i][j] = a[i][j];
	return 0;
} //复制棋盘
int getch(void)
{
	struct termios tm, tm_old;
	int fd = 0, ch;

	if (tcgetattr(fd, &tm) < 0)
	{ //保存现在的终端设置
		return -1;
	}

	tm_old = tm;
	cfmakeraw(&tm); //更改终端设置为原始模式，该模式下所有的输入数据以字节为单位被处理
	if (tcsetattr(fd, TCSANOW, &tm) < 0)
	{ //设置上更改之后的设置
		return -1;
	}

	ch = getchar();
	if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
	{ //更改设置为最初的样子
		return -1;
	}

	return ch;
}
int ini_chessbored() //初始化棋盘
{
	memset(chessbored, -1, sizeof(chessbored));
	int i;
	for (i = 0; i < chessbored_size; i++)
	{
		chessbored[i][0] = side;
		chessbored[0][i] = side;
		chessbored[chessbored_size - 1][i] = side;
		chessbored[i][chessbored_size - 1] = side;
	}
	return 0;
}
string get_style(int i, int j) //得到坐标的字符
{
	int a = chessbored[i][j];
	if (i == cursor_x && j == cursor_y)
		return "[ ]";
	if (a == black)
		return " ● ";
	if (a == white)
		return " ○ ";
	if (i > 1)
	{
		if (i < chessbored_size - 2)
		{
			if (j > 1)
			{
				if (j < chessbored_size - 2)
				{
					return "-│-";
				}
				else
				{
					return "-┃ ";
				}
			}
			else
			{
				return " ┃-";
			}
		}
		else
		{
			if (j > 1)
			{
				if (j < chessbored_size - 2)
				{
					return "=┷=";
				}
				else
				{
					return "=┛ ";
				}
			}
			else
			{
				return " ┗=";
			}
		}
	}
	else
	{
		if (j > 1)
		{
			if (j < chessbored_size - 2)
			{
				return "=┯=";
			}
			else
			{
				return "=┓ ";
			}
		}
		else
		{
			return " ┏=";
		}
	}
}
int paint() //绘图
{
	string canvas[chessbored_size];
	int i, j;
	for (i = 1; i < chessbored_size - 1; i++)
	{
		canvas[i] = canvas[i] + " ";
		for (j = 1; j < chessbored_size - 1; j++)
			canvas[i] = canvas[i] + get_style(i, j);
		canvas[i] = canvas[i] + " ";
	}
	for (i = 0; i < chessbored_size; i++)
	{
		canvas[0] = canvas[0] + " ";
		canvas[chessbored_size - 1] = canvas[chessbored_size - 1] + " ";
	}
	//system("cls");
	system("clear"); //清屏
	system("clear");
	for (i = 0; i < chessbored_size; i++)
		cout << canvas[i] << endl;
	cout << "WASD = 移动  space = 落子  Esc = 悔棋" << endl;
	return 0;
}
int near_num(int x, int y) //返回此点形成的最高连珠数
{
	int s, m, k, n = chessbored[x][y];
	s = 1;
	for (k = 1; k < 6 && chessbored[x][y - k] == n; k++) //向左找连珠
		s++;
	for (k = 1; k < 6 && chessbored[x][y + k] == n; k++) //向右找连珠
		s++;
	m = s;
	s = 1;
	for (k = 1; k < 6 && chessbored[x - k][y] == n; k++) //向上
		s++;
	for (k = 1; k < 6 && chessbored[x + k][y] == n; k++) //向下
		s++;
	if (s > m)
		m = s;
	s = 1;
	for (k = 1; k < 6 && chessbored[x - k][y - k] == n; k++) //左上
		s++;
	for (k = 1; k < 6 && chessbored[x + k][y + k] == n; k++) //右下
		s++;
	if (s > m)
		m = s;
	s = 1;
	for (k = 1; k < 6 && chessbored[x - k][y + k] == n; k++) //右上
		s++;
	for (k = 1; k < 6 && chessbored[x + k][y - k] == n; k++) //左下
		s++;
	if (s > m)
		m = s;
	return m;
}
int empty_value_sub(node e, int a) //a表示视角是玩家还是电脑，空格评分函数。
{
	int temp = chessbored[e.i][e.j];
	chessbored[e.i][e.j] = a;
	if (near_num(e.i, e.j) > 5)
	{
		chessbored[e.i][e.j] = temp;
		return fen_max;
	}
	chessbored[e.i][e.j] = temp;
	int b = 1, i, j, k, s = 0, n, fa = black + white - a, b2 = 0;
	/* s总评分，b当前方向评分，a己方颜色，fa对方颜色 */

	/* 横向评估 */
	for (k = 1; k < 6; k++) //向左寻找连珠，遇到墙、对手颜色均直接退出循环
	{
		if (chessbored[e.i][e.j - k] == a)
			b *= 4;
		else if (chessbored[e.i][e.j - k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	b2 = 0;
	for (k = 1; k < 6; k++) //向右寻找连珠
	{
		if (chessbored[e.i][e.j + k] == a)
			b *= 4;
		else if (chessbored[e.i][e.j + k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	n = 0; //向左向右寻找发展空间
	for (k = 1; k < 6 && chessbored[e.i][e.j - k] != fa && chessbored[e.i][e.j - k] != side; k++)
		n++;
	for (k = 1; k < 6 && chessbored[e.i][e.j + k] != fa && chessbored[e.i][e.j + k] != side; k++)
		n++;
	if (n < 5) //左右空间不足5，肯定没有机会，直接评分0
		b = 0;
	if (n == 5 && b == 512) //左右空间正好5，已有4颗己方子，必胜，评分512可直接1024？
		b = 1024;
	s = s + b; //评分累加

	/* 纵向评估 */
	b = 1;
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i - k][e.j] == a)
			b *= 4;
		else if (chessbored[e.i - k][e.j] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i + k][e.j] == a)
			b *= 4;
		else if (chessbored[e.i + k][e.j] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	n = 0;
	for (k = 1; k < 6 && chessbored[e.i - k][e.j] != fa && chessbored[e.i - k][e.j] != side; k++)
		n++;
	for (k = 1; k < 6 && chessbored[e.i + k][e.j] != fa && chessbored[e.i + k][e.j] != side; k++)
		n++;
	if (n < 5)
		b = 0;
	if (n == 5 && b == 512)
		b = 1024;
	s = s + b;

	/* 主对角线评分 */
	b = 1;
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i - k][e.j - k] == a)
			b *= 4;
		else if (chessbored[e.i - k][e.j - k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i + k][e.j + k] == a)
			b *= 4;
		else if (chessbored[e.i + k][e.j + k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	n = 0;
	for (k = 1; k < 6 && chessbored[e.i - k][e.j - k] != fa && chessbored[e.i - k][e.j - k] != side; k++)
		n++;
	for (k = 1; k < 6 && chessbored[e.i + k][e.j + k] != fa && chessbored[e.i + k][e.j + k] != side; k++)
		n++;
	if (n < 5)
		b = 0;
	if (n == 5 && b == 512)
		b = 1024;
	s = s + b;

	/* 副对角线评估 */
	b = 1;
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i - k][e.j + k] == a)
			b *= 4;
		else if (chessbored[e.i - k][e.j + k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i + k][e.j - k] == a)
			b *= 4;
		else if (chessbored[e.i + k][e.j - k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	n = 0;
	for (k = 1; k < 6 && chessbored[e.i - k][e.j + k] != fa && chessbored[e.i - k][e.j + k] != side; k++)
		n++;
	for (k = 1; k < 6 && chessbored[e.i + k][e.j - k] != fa && chessbored[e.i + k][e.j - k] != side; k++)
		n++;
	if (n < 5)
		b = 0;
	if (n == 5 && b == 512)
		b = 1024;
	s = s + b;

	return s; //返回四个方向的总评分
}
int empty_value(node &e, int a) //评分
{
	int my, you;
	my = empty_value_sub(e, a);									 //己方评分
	you = empty_value_sub(e, player_color + computer_color - a); //对方评分
	e.value = my + you;
	/* 对对方分越高说明越重要，防守，所以要加对方的分 */
	return my + you;
}
int empty_value2(node &e, int a) //评分函数
{
	int my, you;
	my = empty_value_sub(e, a);
	you = empty_value_sub(e, player_color + computer_color - a);
	e.value = my - you / 2;
	return my - you / 2;
}
int pingfen(int pc) //总评函数
{
	vector<node>::iterator the_iterator;
	node e;
	int max3 = fen_min;
	the_iterator = empty_node.begin();
	while (the_iterator != empty_node.end())
	{
		e = *the_iterator;
		empty_value2(e, pc);
		if (e.value > max3)
			max3 = e.value;
		the_iterator++;
	}
	return max3;
}
/*int display()
{
	vector<node>::iterator the;
	the = empty_node.begin();
	while (the != empty_node.end())
	{
		cout << (*the).i << " " << (*the).j << " " << (*the).value << endl;
		the++;
	}
	return 0;
}*/
int before_move(int x, int y) //从empty_node中删除当前节点
{
	if (chessbored[x][y] == nempty)
	{
		vector<node>::iterator the_iterator; //iterator，容器的迭代器
		node e;
		the_iterator = empty_node.begin();
		while (the_iterator != empty_node.end())
		{
			e = *the_iterator;
			if (e.i == x && e.j == y)
			{
				empty_node.erase(the_iterator);
				break;
			}
			the_iterator++;
		}
	}
	return 0;
}
int updata_empty_node(int x, int y) //更新棋子周围的空点
{
	int i, j;
	for (i = -3; i < 4; i++)
		for (j = -3; j < 4; j++)
			if (chessbored[x + i][y + j] == _empty && x + i > 0 && x + i < 20 && y + j > 0 && y + j < 20)
			{
				node e;
				e.i = x + i;
				e.j = y + j;
				empty_node.push_back(e);
				//cout<<e.i<<" "<<e.j<<endl;
				chessbored[x + i][y + j] = nempty;
			}
	return 0;
}
int youfen(int pc, int depth2, int jz);
int myfen(int pc, int depth2, int jz) //我的分数
{
	if (depth2 >= depth) //最深层数depth
	{
		int a = pingfen(pc);
		return a;
	}
	int fa = player_color + computer_color - pc, c;
	int n = empty_node.size();
	int i, j;
	vector<node> en;
	en = empty_node;
	node e, e2;
	priority_queue<node, vector<node>, less<node>> pen;
	for (i = 0; i < n; i++)
	{
		e = en.back();
		en.pop_back();
		empty_value(e, pc);
		pen.push(e);
	}
	int max_value = min2;
	int wide2 = wide;
	if (wide2 > n)
		wide2 = n;
	node te[50];
	for (i = 0; i < wide2; i++)
	{
		te[i] = pen.top();
		pen.pop();
	}
	sixnode se[20];
	int secnt = 0;
	for (i = 0; i < wide2; i++)
		for (j = i + 1; j < wide2; j++)
		{
			se[secnt].x1 = te[i].i;
			se[secnt].y1 = te[i].j;
			se[secnt].x2 = te[j].i;
			se[secnt].y2 = te[j].j;
			secnt++;
		}
	int chessbored2[chessbored_size][chessbored_size];
	szfz(chessbored, chessbored2);
	for (i = 0; i < secnt; i++)
	{
		vector<node> empty_node2;
		empty_node2 = empty_node;
		before_move(se[i].x1, se[i].y1);
		before_move(se[i].x2, se[i].y2);
		chessbored[se[i].x1][se[i].y1] = pc;
		chessbored[se[i].x2][se[i].y2] = pc;
		if (near_num(se[i].x1, se[i].y1) > 5)
		{
			empty_node = empty_node2;
			szfz(chessbored2, chessbored);
			return fen_max;
		}
		if (near_num(se[i].x2, se[i].y2) > 5)
		{
			empty_node = empty_node2;
			szfz(chessbored2, chessbored);
			return fen_max;
		}
		updata_empty_node(se[i].x1, se[i].y1);
		updata_empty_node(se[i].x2, se[i].y2);
		int fenshu;
		fenshu = youfen(pc, depth2 + 1, max_value);
		if (fenshu > max_value)
		{
			max_value = fenshu;
		}
		empty_node = empty_node2;
		szfz(chessbored2, chessbored);
	}
	return max_value;
}
int youfen(int pc, int depth2, int jz) //你的分数 ,jz剪枝
{
	int fa = player_color + computer_color - pc;
	int n = empty_node.size();
	int i, j, i2;
	vector<node> en;
	en = empty_node;
	node e, e2;
	priority_queue<node, vector<node>, less<node>> pen;
	for (i = 0; i < n; i++)
	{
		e = en.back();
		en.pop_back();
		empty_value(e, fa); //对对方评分
		pen.push(e);
	}
	int min_value = max2, c; //max2=正无穷
	int wide2 = wide;
	if (wide2 > n)
		wide2 = n;
	node te[50];
	for (i = 0; i < wide2; i++)
	{
		te[i] = pen.top();
		pen.pop();
	}
	sixnode se[20];
	int secnt = 0;
	for (i = 0; i < wide2; i++)
		for (j = i + 1; j < wide2; j++)
		{
			se[secnt].x1 = te[i].i;
			se[secnt].y1 = te[i].j;
			se[secnt].x2 = te[j].i;
			se[secnt].y2 = te[j].j;
			secnt++;
		}
	int chessbored2[chessbored_size][chessbored_size];
	szfz(chessbored, chessbored2);
	for (i = 0; i < secnt; i++)
	{
		vector<node> empty_node2;
		empty_node2 = empty_node;
		before_move(se[i].x1, se[i].y1);
		before_move(se[i].x2, se[i].y2);
		chessbored[se[i].x1][se[i].y1] = fa;
		chessbored[se[i].x2][se[i].y2] = fa;
		if (near_num(se[i].x1, se[i].y1) > 5) //必死，返回最小值
		{
			empty_node = empty_node2;
			szfz(chessbored2, chessbored);
			return fen_min;
		}
		if (near_num(se[i].x2, se[i].y2) > 5)
		{
			empty_node = empty_node2;
			szfz(chessbored2, chessbored);
			return fen_min;
		}
		updata_empty_node(se[i].x1, se[i].y1);
		updata_empty_node(se[i].x2, se[i].y2);
		int fenshu;
		fenshu = myfen(pc, depth2 + 1, min_value); //搜索深度+1
		if (fenshu < min_value)
		{
			min_value = fenshu;
		}
		empty_node = empty_node2;
		szfz(chessbored2, chessbored);
	}
	return min_value; //返回它认为的最小值β
}
sixnode get_bestnode(int pc) //得到最优点 ,pc表示以哪一方为自己
{
	int n = empty_node.size();
	int i, c1, c2, j, i2;
	vector<node> en;
	en = empty_node;
	sixnode se2;
	node e;
	priority_queue<node, vector<node>, less<node>> pen; //大顶堆
	/* 对所有空点评分，并存入大顶堆排序 */
	for (i = 0; i < n; i++)
	{
		e = en.back();
		en.pop_back();
		empty_value(e, pc); //对该点评估，分数保存在e.value
		pen.push(e);
	}
	int max_value = min2;
	//   display();
	int wide2 = wide; //wide=4，取出评分靠前的几个点
	if (wide2 > n)
		wide2 = n;
	node te[50];
	for (i = 0; i < wide2; i++)
	{
		te[i] = pen.top();
		pen.pop();
	}
	sixnode se[20]; //存候选落点组合
	int secnt = 0;
	for (i = 0; i < wide2; i++)
		for (j = i + 1; j < wide2; j++)
		{
			se[secnt].x1 = te[i].i;
			se[secnt].y1 = te[i].j;
			se[secnt].x2 = te[j].i;
			se[secnt].y2 = te[j].j;
			secnt++;
		}
	int chessbored2[chessbored_size][chessbored_size];
	szfz(chessbored, chessbored2); //复制棋盘
	/* 遍历所有候选组合 */
	for (i = 0; i < secnt; i++)
	{
		vector<node> empty_node2;
		empty_node2 = empty_node;
		before_move(se[i].x1, se[i].y1);
		before_move(se[i].x2, se[i].y2);
		chessbored[se[i].x1][se[i].y1] = pc;
		chessbored[se[i].x2][se[i].y2] = pc;
		if (near_num(se[i].x1, se[i].y1) > 5) //如果一颗即连珠超过6，必胜
		{
			empty_node = empty_node2;
			szfz(chessbored2, chessbored);
			return se[i];
		}
		if (near_num(se[i].x2, se[i].y2) > 5)
		{
			empty_node = empty_node2;
			szfz(chessbored2, chessbored);
			return se[i];
		}
		updata_empty_node(se[i].x1, se[i].y1);
		updata_empty_node(se[i].x2, se[i].y2);
		int fenshu;
		fenshu = youfen(pc, 1, max_value);
		if (fenshu > max_value)
		{
			i2 = i;
			max_value = fenshu;
		}
		empty_node = empty_node2;
		szfz(chessbored2, chessbored);
	}
	return se[i2]; //返回综合结果最佳的一组走法
}
int computer_move(int pc)
{
	sixnode se;
	se = get_bestnode(pc); /*  !!!!!!!!!!!!!!!!!!!!!  */
	before_move(se.x1, se.y1);
	before_move(se.x2, se.y2);
	chessbored[se.x1][se.y1] = pc;
	chessbored[se.x2][se.y2] = pc;
	paint();
	if (near_num(se.x1, se.y1) > 5)
		return 1;
	if (near_num(se.x2, se.y2) > 5)
		return 1;
	updata_empty_node(se.x1, se.y1);
	updata_empty_node(se.x2, se.y2);
	return 0;
}
int regret_save() //储存悔棋信息
{
	regret_num++;
	szfz(chessbored, regret_array[regret_num]);
	regret_empty_node[regret_num] = empty_node;
	return 0;
}
int regret() //悔棋
{
	regret_num--;
	if (regret_num < 0)
		return 0;
	szfz(regret_array[regret_num], chessbored);
	empty_node = regret_empty_node[regret_num];
	return 0;
}
int player_move()
{
	int key, i;
	while (1)
	{
		key = getch();
		if (key == 119 || key == 115 || key == 97 || key == 100) //如果按下的是wsad
		{
			//key = getch();
			if (key == 119) // 上w
			{
				cursor_x--;
				if (cursor_x < 1)
					cursor_x = chessbored_size - 2;
			}
			else if (key == 115) //下s
			{
				cursor_x++;
				if (cursor_x > chessbored_size - 2)
					cursor_x = 1;
			}
			else if (key == 97) //左a
			{
				cursor_y--;
				if (cursor_y < 1)
					cursor_y = chessbored_size - 2;
			}
			else if (key == 100) //右d
			{
				cursor_y++;
				if (cursor_y > chessbored_size - 2)
					cursor_y = 1;
			}
			paint();
		}
		else if (key == 32 && chessbored[cursor_x][cursor_y] < 0) //如果按下的是空格
		{
			before_move(cursor_x, cursor_y);
			chessbored[cursor_x][cursor_y] = player_color;
			paint();
			if (near_num(cursor_x, cursor_y) > 5) //计算连珠数
				return 1;
			updata_empty_node(cursor_x, cursor_y);
			return 0;
		}
		else if (key == 27)
		{
			regret();
			paint();
		}
	}
}
int main()
{
	//system("title 六子棋 ——最爱吃兽奶制作"); //设置标题
	//system("mode con cols=45 lines=24");//设置窗口大小
	//system("color b0");						 //设置颜色
	while (1)
	{
		ini_chessbored();
		regret_num = -1;
		cursor_x = chessbored_size / 2;
		cursor_y = chessbored_size / 2;
		//		cursor_x=2;
		//		cursor_y=2;
		empty_node.clear();
		cout << " *** 六子棋 游戏 ***" << endl;
		cout << "1先手，2后手" << endl;
		int cnt;
		cin >> cnt;
		if (cnt == 2)
		{
			int f = chessbored_size / 2;
			//before_move(f, f);
			chessbored[f][f] = computer_color;
			updata_empty_node(f, f);
			paint();
		}
		else
		{
			paint();
			player_move();
			computer_move(computer_color);
		}
		while (1)
		{
			int a;
			//			a=computer_move(player_color);
			regret_save();
			player_move();
			a = player_move();
			if (a)
			{
				cout << "玩家胜利" << endl;
				int temp = 1;
				if (getch() == 27)
				{
					regret();
					paint();
					player_move();
					temp = 0;
				}
				if (temp)
					break;
			}
			//			system("pause");
			a = computer_move(computer_color);
			if (a)
			{
				cout << "电脑胜利" << endl;
				int temp = 1;
				if (getch() == 27)
				{
					regret();
					paint();
					temp = 0;
				}
				if (temp)
					break;
			}
			//			system("pause");
		}
	}
	return 0;
}

int empty_value_sub_2(node e, int a) //a表示视角是玩家还是电脑，空格评分函数。
{
	int temp = chessbored[e.i][e.j];
	chessbored[e.i][e.j] = a;
	if (near_num(e.i, e.j) > 5)
	{
		chessbored[e.i][e.j] = temp;
		return fen_max;
	}
	chessbored[e.i][e.j] = temp;
	int b, i, k, s = 0, fa = black + white - a;
	int front, back;
	/* s总评分，b当前方向评分，a己方颜色，fa对方颜色 */

	/* 横向评估 */
	b = 1;
	for (front = 0; chessbored[e.i][e.j - front - 1] != fa && chessbored[e.i][e.j - front - 1] != side; front++)
	{
	}
	for (back = 0; chessbored[e.i][e.j + back + 1] != fa && chessbored[e.i][e.j + back + 1] != side; back++)
	{
	}

	if (front + back < 5)
		b = 0;
	else
	{
		for (k = front; k != (4 - back); k--)
		{
			for (i = 0; i < 6; i++)
			{
				if (chessbored[e.i][e.j - k + i] == a)
					b *= 8;
			}
		}
	}
	s = s + b; //评分累加

	/* 纵向评估 */
	b = 1;
	for (front = 0; chessbored[e.i - front - 1][e.j] != fa && chessbored[e.i - front - 1][e.j] != side; front++)
	{
	}
	for (back = 0; chessbored[e.i + back + 1][e.j] != fa && chessbored[e.i + back + 1][e.j] != side; back++)
	{
	}

	if (front + back < 5)
		b = 0;
	else
	{
		for (k = front; k != (4 - back); k--)
		{
			for (i = 0; i < 6; i++)
			{
				if (chessbored[e.i - k + i][e.j] == a)
					b *= 8;
			}
		}
	}
	s = s + b; //评分累加

	/* 主对角线评分 */
	b = 1;
	for (front = 0; chessbored[e.i - front - 1][e.j - front - 1] != fa && chessbored[e.i - front - 1][e.j - front - 1] != side; front++)
	{
	}
	for (back = 0; chessbored[e.i + back + 1][e.j + back + 1] != fa && chessbored[e.i + back + 1][e.j + back + 1] != side; back++)
	{
	}

	if (front + back < 5)
		b = 0;
	else
	{
		for (k = front; k != (4 - back); k--)
		{
			for (i = 0; i < 6; i++)
			{
				if (chessbored[e.i - k + i][e.j - k + i] == a)
					b *= 8;
			}
		}
	}
	s = s + b; //评分累加

	/* 副对角线评估 */
	b = 1;
	for (front = 0; chessbored[e.i - front - 1][e.j + front + 1] != fa && chessbored[e.i - front - 1][e.j + front + 1] != side; front++)
	{
	}
	for (back = 0; chessbored[e.i + back + 1][e.j - back - 1] != fa && chessbored[e.i + back + 1][e.j - back - 1] != side; back++)
	{
	}

	if (front + back < 5)
		b = 0;
	else
	{
		for (k = front; k != (4 - back); k--)
		{
			for (i = 0; i < 6; i++)
			{
				if (chessbored[e.i + k - i][e.j - k + i] == a)
					b *= 8;
			}
		}
	}
	s = s + b; //评分累加

	return s; //返回四个方向的总评分
}

int empty_value_sub_1(node e, int a) //a表示视角是玩家还是电脑，空格评分函数。
{
	int temp = chessbored[e.i][e.j];
	chessbored[e.i][e.j] = a;
	if (near_num(e.i, e.j) > 5)
	{
		chessbored[e.i][e.j] = temp;
		return fen_max;
	}
	chessbored[e.i][e.j] = temp;
	int b = 1, i, j, k, s = 0, n, fa = black + white - a, b2 = 0;
	/* s总评分，b当前方向评分，a己方颜色，fa对方颜色 */

	/* 横向评估 */
	for (k = 1; k < 6; k++) //向左寻找连珠，遇到墙、对手颜色均直接退出循环
	{
		if (chessbored[e.i][e.j - k] == a)
			b *= 4;
		else if (chessbored[e.i][e.j - k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	b2 = 0;
	for (k = 1; k < 6; k++) //向右寻找连珠
	{
		if (chessbored[e.i][e.j + k] == a)
			b *= 4;
		else if (chessbored[e.i][e.j + k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	/*for (k = 0; k < 6 && chessbored[e.i][e.j - 5 + k] != fa && chessbored[e.i][e.j - 5 + k] != side && chessbored[e.i][e.j + k] != fa && chessbored[e.i][e.j + k] != side; k++)
	{
		n = 0;
		for (i = 0; i < 6; i++)
		{
			if (chessbored[e.i][e.j - 5 + k + i] == a)
				n++;
		}
		if (n >= 5)
			return fen_max;
	}*/
	s = s + b; //评分累加

	/* 纵向评估 */
	b = 1;
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i - k][e.j] == a)
			b *= 4;
		else if (chessbored[e.i - k][e.j] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i + k][e.j] == a)
			b *= 4;
		else if (chessbored[e.i + k][e.j] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	/*for (k = 0; k < 6 && chessbored[e.i - 5 + k][e.j] != fa && chessbored[e.i - 5 + k][e.j] != side && chessbored[e.i + k][e.j] != fa && chessbored[e.i + k][e.j] != side; k++)
	{
		n = 0;
		for (i = 0; i < 6; i++)
		{
			if (chessbored[e.i - 5 + k + i][e.j] == a)
				n++;
		}
		if (n >= 5)
			return fen_max;
	}*/
	s = s + b;

	/* 主对角线评分 */
	b = 1;
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i - k][e.j - k] == a)
			b *= 4;
		else if (chessbored[e.i - k][e.j - k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i + k][e.j + k] == a)
			b *= 4;
		else if (chessbored[e.i + k][e.j + k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	/*for (k = 0; k < 6 && chessbored[e.i - 5 + k][e.j - 5 + k] != fa && chessbored[e.i - 5 + k][e.j - 5 + k] != side && chessbored[e.i + k][e.j + k] != fa && chessbored[e.i + k][e.j + k] != side; k++)
	{
		n = 0;
		for (i = 0; i < 6; i++)
		{
			if (chessbored[e.i - 5 + k + i][e.j - 5 + k + i] == a)
				n++;
		}
		if (n >= 5)
			return fen_max;
	}*/
	s = s + b;

	/* 副对角线评估 */
	b = 1;
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i - k][e.j + k] == a)
			b *= 4;
		else if (chessbored[e.i - k][e.j + k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	b2 = 0;
	for (k = 1; k < 6; k++)
	{
		if (chessbored[e.i + k][e.j - k] == a)
			b *= 4;
		else if (chessbored[e.i + k][e.j - k] < 0)
		{
			b2++;
			if (b2 > 1)
			{
				break;
			}
			b *= 2;
		}
		else
			break;
	}
	/*for (k = 0; k < 6 && chessbored[e.i - 5 + k][e.j + 5 - k] != fa && chessbored[e.i - 5 + k][e.j + 5 - k] != side && chessbored[e.i + k][e.j - k] != fa && chessbored[e.i + k][e.j - k] != side; k++)
	{
		n = 0;
		for (i = 0; i < 6; i++)
		{
			if (chessbored[e.i - 5 + k + i][e.j + 5 - k - i] == a)
				n++;
		}
		if (n >= 5)
			return fen_max;
	}*/
	s = s + b;

	return s; //返回四个方向的总评分
}