#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#define size 4

int move[4][2] = {{-1, 0}, {0, -1}, {0, 1}, {1, 0}}; //上 左 右 下  置换顺序
char op[4] = {'U', 'L', 'R', 'D'};
int map[size][size], map2[size * size], limit, path[100];
int flag, length;
int goal[16][2] = {{3, 3}, {0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 1}, {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 2}, {2, 3}, {3, 0}, {3, 1}, {3, 2}}; //各个数字应在位置对照表
int nixu(int a[size * size])
{
    int i, j, ni, w, x, y; //w代表0的位置 下标，x y 代表0的数组坐标
    ni = 0;
    for (i = 0; i < size * size; i++) //，size*size=16
    {
        if (a[i] == 0) //找到0的位置
            w = i;
        for (j = i + 1; j < size * size; j++) //注意！！每一个都跟其后所有的比一圈 查找小于i的个数相加
        {
            if (a[i] > a[j])
                ni++;
        }
    }
    x = w / size;
    y = w % size;
    ni += abs(x - 3) + abs(y - 3); //最后加上0的偏移量
    if (ni % 2 == 1)
        return 1;
    else
        return 0;
}

int hv(int a[][size]) //估价函数，曼哈顿距离，小等于实际总步数
{
    int i, j, cost = 0;
    for (i = 0; i < size; i++)
    {
        for (j = 0; j < size; j++)
        {
            int w = map[i][j];
            cost += abs(i - goal[w][0]) + abs(j - goal[w][1]);
        }
    }
    return cost;
}

void swap(int *a, int *b) //交换两个数的位置
{
    int tmp;
    tmp = *a;
    *a = *b;
    *b = tmp;
}
/* sx,sy是空格的位置 */
/* len是已经移动的步数，相当于g */
/* pre_move前一次移动方向 */
void dfs(int sx, int sy, int len, int pre_move)
{
    int i, nx, ny;
    if (flag)
        return;
    int dv = hv(map);
    if (len == limit) //已移动距离等于原始曼哈顿距离
    {
        if (dv == 0) //移动后曼哈顿距离==0，成功！ 退出
        {
            flag = 1;
            length = len;
            return;
        }
        else
            return; //进入死胡同，超过预设长度 回退
    }
    else if (len < limit) //已移动距离小于原始曼哈顿距离
    {
        if (dv == 0) //短于预设长度 成功！退出
        {
            flag = 1;
            length = len;
            return;
        }
    }
    for (i = 0; i < 4; i++)
    {
        if (i + pre_move == 3 && len > 0) //不和上一次移动方向相反，对第二步以后而言
            continue;
        nx = sx + move[i][0]; //移动的四步 上左右下
        ny = sy + move[i][1];
        if (0 <= nx && nx < size && 0 <= ny && ny < size) //判断移动合理
        {
            swap(&map[sx][sy], &map[nx][ny]);
            int p = hv(map);               //移动后的 曼哈顿距离p=16
            if (p + len <= limit && !flag) //p+len<=limit&&!flag剪枝判断语句
            {
                path[len] = i;
                dfs(nx, ny, len + 1, i); //如当前步成功则 递归调用dfs
                if (flag)
                    return;
            }
            swap(&map[sx][sy], &map[nx][ny]); //不合理则回退一步
        }
    }
}
int main()
{
    int i, j, k, l, m, n, sx, sy;
    char c, g;
    i = 0;
    /*printf("请输入您要判断几组十五数码：\n");
    scanf("%d", &n);
    while (n--)
    {*/
    printf("请输入十五数码：\n");
    flag = 0, length = 0;
    memset(path, -1, sizeof(path)); //已定义path[100]数组，将path填满-1   void* memset(void*s,int ch,size_t n):将S中前n个字节用ch替换并返回S。
    for (i = 0; i < 16; i++)        //给map 和map2赋值map是二维数组，map2是一维数组
    {
        scanf("%d", &map2[i]);
        if (map2[i] == 0)
        {
            map[i / size][i % size] = 0; //map二维数组，保存具体值
            sx = i / size;               //sx,sy保存0的坐标
            sy = i % size;
        }
        else
        {
            map[i / size][i % size] = map2[i];
        }
    }

    if (nixu(map2) == 1) //该状态可达
    {
        limit = hv(map);              //全部的曼哈顿距离之和
        while (!flag && length <= 50) //要求50步之内到达
        {
            dfs(sx, sy, 0, 0);
            if (!flag)
                limit++; //得到的是最小步数
        }
        if (flag)
        {
            for (i = 0; i < length; i++)
                printf("%c", op[path[i]]); //根据path输出URLD路径
            printf("\n");
        }
    }
    else if (!nixu(map2) || !flag)
        printf("This puzzle is not solvable.\n");

    return 0;
}