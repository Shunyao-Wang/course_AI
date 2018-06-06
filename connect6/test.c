#include <stdio.h>
#include <termio.h>
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

int sh_getch(void)
{
}

int main(int argc, char *argv[])
{
    int a[3];
    for (int i = 0; i < 3; i++)
    {
        a[i] = getch();
    }
    printf("%d,%d,%d\n", a[0],a[1],a[2]);
    return 0;
}
