/*
References:
https://blog.csdn.net/lichengyu/article/details/41621099 | C语言从stdin读取一行字符串的几种方法_大唐游子的博客-CSDN博客_c读取一行
https://www.runoob.com/cprogramming/c-macro-errno.html | C 库宏 – errno | 菜鸟教程
https://zhuanlan.zhihu.com/p/348762602 | C语言丨正确使用extern关键字详解 - 知乎
https://blog.csdn.net/zhouzhenhe2008/article/details/74011399 | 字符串分割利器—strtok_r函数_zhouzhenhe2008的博客-CSDN博客
https://www.runoob.com/cprogramming/c-function-strstr.html | C 库函数 – strstr() | 菜鸟教程
https://blog.csdn.net/qq_31941921/article/details/121128854?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-121128854-blog-100806493.pc_relevant_3mothn_strategy_recovery&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7ERate-1-121128854-blog-100806493.pc_relevant_3mothn_strategy_recovery&utm_relevant_index=2 | 进程间通信——管道_小明的笔记仓库的博客-CSDN博客_进程管道
https://www.jianshu.com/p/e0c6749dbcdc | C语言-系统-wait()和waitpid()函数 - 简书
https://www.tutorialspoint.com/unix_system_calls/waitpid.htm | waitpid() - Unix, Linux System Call
https://blog.csdn.net/endeavor_g/article/details/80552680 | c语言链表详解（超详细）_Mr.Gzj的博客-CSDN博客_链表
https://www.runoob.com/cprogramming/c-header-files.html | C 头文件 | 菜鸟教程
https://zhuanlan.zhihu.com/p/129325486 | c语言makefile文件 - 知乎
*/

#include "global.h"

int main(void)
{
    // Ignore SIGINT, SIGQUIT, SIGTSTP in the shell
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    // Initialize memory for STDIN and basename
    char *str = (char *)malloc(sizeof(char) * 1001);
    char *cwd = (char *)malloc(sizeof(char) * 100);

    printf("[nyush %s]$ ", get_basename(cwd, 100));
    fflush(stdout);
    while (fgets(str, 1001, stdin))
    {
        command_parser(str);
        printf("[nyush %s]$ ", get_basename(cwd, 100));
        fflush(stdout);
    }

    printf("\n");
}