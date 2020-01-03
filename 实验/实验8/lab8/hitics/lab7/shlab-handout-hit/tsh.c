/*
 * tsh - A tiny shell program with job control
 *
 * <Put your name and login ID here>罗瑞欣 1170300821
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */
#define MAXJOBS      16   /* max jobs at any point in time */
#define MAXJID    1<<16   /* max job ID */

/* Job states */
#define UNDEF 0 /* undefined */
#define FG 1    /* running in foreground */
#define BG 2    /* running in background */
#define ST 3    /* stopped */

/*
 * Jobs states: FG (foreground), BG (background), ST (stopped)
 * Job state transitions and enabling actions:
 *     FG -> ST  : ctrl-z
 *     ST -> FG  : fg command
 *     ST -> BG  : bg command
 *     BG -> FG  : fg command
 * At most 1 job can be in the FG state.
 */

/* Global variables */
extern char **environ;      /* defined in libc */
char prompt[] = "tsh> ";    /* command line prompt (DO NOT CHANGE) */
int verbose = 0;            /* if true, print additional output */
int nextjid = 1;            /* next job ID to allocate */
char sbuf[MAXLINE];         /* for composing sprintf messages */

struct job_t {              /* The job struct */
	pid_t pid;              /* job PID */
	int jid;                /* job ID [1, 2, ...] */
	int state;              /* UNDEF, BG, FG, or ST */
	char cmdline[MAXLINE];  /* command line */
};
struct job_t jobs[MAXJOBS]; /* The job list */
/* End global variables */


/* Function prototypes */

/* Here are the functions that you will implement */
void eval(char *cmdline);
int builtin_cmd(char **argv);
void do_bgfg(char **argv);
void waitfg(pid_t pid);
void sigchld_handler(int sig);
void sigtstp_handler(int sig);
void sigint_handler(int sig);
/* Here are helper routines that we've provided for you */
int parseline(const char *cmdline, char **argv);
void sigquit_handler(int sig);
void clearjob(struct job_t *job);
void initjobs(struct job_t *jobs);
int maxjid(struct job_t *jobs);
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline);
int deletejob(struct job_t *jobs, pid_t pid);
pid_t fgpid(struct job_t *jobs);
struct job_t *getjobpid(struct job_t *jobs, pid_t pid);
struct job_t *getjobjid(struct job_t *jobs, int jid);
int pid2jid(pid_t pid);
void listjobs(struct job_t *jobs);
void usage(void);
void unix_error(char *msg);
void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine
 */
int main(int argc, char **argv) {
	char c;
	char cmdline[MAXLINE];//命令行参数
	int emit_prompt = 1; /* emit prompt (default) */

	/* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
	dup2(1, 2);

	/* Parse the command line */
	while ((c = getopt(argc, argv, "hvp")) != EOF) {
		switch (c) {
			case 'h':             /* print help message */
				usage();
				break;
			case 'v':             /* emit additional diagnostic info */
				verbose = 1;
				break;
			case 'p':             /* don't print a prompt */
				emit_prompt = 0;  /* handy for automatic testing */
				break;
			default:
				usage();
		}
	}

	/* Install the signal handlers */

	/* These are the ones you will need to implement */
	Signal(SIGINT, sigint_handler);  //信号处理函数Signal处理SIGINT /* ctrl-c */
	Signal(SIGTSTP, sigtstp_handler); //信号处理函数Signal处理SIGSTP /* ctrl-z */
	Signal(SIGCHLD, sigchld_handler); //信号处理函数Signal处理SIGCHLD /* Terminated or stopped child */

	/* This one provides a clean way to kill the shell */
	Signal(SIGQUIT, sigquit_handler);//信号处理函数Signal处理SIGQUIT

	/* Initialize the job list */
	initjobs(jobs);//初始化jobs，将其jid=0，pid=0，state=UNDEF，cmdline置为空

	/* Execute the shell's read/eval loop */
	while (1) {

		/* Read command line */
		if (emit_prompt) {
			printf("%s", prompt);
			fflush(stdout);
		}
		if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin))
			app_error("fgets error");
		if (feof(stdin)) { /* End of file (ctrl-d) */
			fflush(stdout);
			exit(0);//读取结束直接exi(0)正常退出
		}

		/* Evaluate the command line */
		eval(cmdline);//对命令行参数求值
		fflush(stdout);//清空缓冲区
		fflush(stdout);
	}

	exit(0); /* control never reaches here */ //然鹅根据clion，这个并没有什么卵用。。。。
}

/*
 * eval - Evaluate the command line that the user has just typed in
 *
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.
*/
void eval(char *cmdline) {
	/* $begin handout */
	char *argv[MAXARGS]; /* argv for execve() */
	int bg;              /* should the job run in bg or fg? */
	pid_t pid;           /* process id */
	sigset_t mask;       /* signal mask */

	/* Parse command line */
	bg = parseline(cmdline, argv);//利用parseline函数返回值返回值，判断是否是后台运行程序
	if (argv[0] == NULL)
		return;   /* ignore empty lines */
/*
 * 利用builtin_cmd函数的返回值，判断是否是内置命令。
 * 若是内置命令，则直接执行，若不是，则新建子进程执行
 */
	if (!builtin_cmd(argv)) {

		/*
     * This is a little tricky. Block SIGCHLD, SIGINT, and SIGTSTP
     * signals until we can add the job to the job list. This
     * eliminates some nasty races between adding a job to the job
     * list and the arrival of SIGCHLD, SIGINT, and SIGTSTP signals.
     */

		if (sigemptyset(&mask) < 0)//先置空阻塞向量
			unix_error("sigemptyset error");//若返回小于0，说明函数错误
		if (sigaddset(&mask, SIGCHLD))//避免下面访问全局结构数组之间的竞争，将SIGCHLD信号加入到阻塞信号集中
			unix_error("sigaddset error");//若返回不为0，说明函数错误
		if (sigaddset(&mask, SIGINT))//避免下面访问全局结构数组之间的竞争，将SIGINT信号加入阻塞集中
			unix_error("sigaddset error");//若返回不为0，说明函数错误
		if (sigaddset(&mask, SIGTSTP))//避免下面访问全局结构数组之间的竞争.将SIGTSTP加入阻塞集中
			unix_error("sigaddset error");//若返回不为0，说明函数错误
		if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)//阻塞在向量中的信号，避免下面访问全局结构数组之间的竞争
			unix_error("sigprocmask error");//若返回小于0，说明函数错误

		/* Create a child process */
		if ((pid = fork()) < 0)//新建子进程。若返回值小于0，说明错误
			unix_error("fork error");

		/*
         * Child  process
         */

		if (pid == 0) {//若新建的子进程返回值为0，说明是子进程
			/* Child unblocks signals */
			sigprocmask(SIG_UNBLOCK, &mask, NULL);//解除阻塞，使新建的子进程要想正常执行接收信号

			/* Each new job must get a new process group ID
               so that the kernel doesn't send ctrl-c and ctrl-z
               signals to all of the shell's jobs */
			if (setpgid(0, 0) < 0)//我们要将新建的子进程加入到一个新的进程组中，因为如果不创建新的进程组的话，我们在终端发送的一些信号会递送到所有的进程
				unix_error("setpgid error");//若返回值小于0，说明函数新建进程组错误

			/* Now load and run the program in the new job */
			if (execve(argv[0], argv, environ) < 0) {//参数执行
				printf("%s: Command not found\n", argv[0]);//若未找到可执行目标文件，则报错
				exit(0);//直接exit(0)正常退出
			}
		}

		/*
         * Parent process
         */

		/* Parent adds the job, and then unblocks signals so that
           the signals handlers can run again */
		addjob(jobs, pid, (bg == 1 ? BG : FG), cmdline);//现在执行的是父进程，将此job加入到job结构体数组中(这个时候信号是阻塞的)
		sigprocmask(SIG_UNBLOCK, &mask, NULL);//解除阻塞，访问全局结构数组结束

		if (!bg)
			waitfg(pid);//如果是前台进程，则需要调用waitfg(pid)函数，阻塞信号直到前台进程不再是子进程
		else
			printf("[%d] (%d) %s", pid2jid(pid), pid, cmdline);//否则直接输出打印即可
	}
	/* $end handout */
	return;//处理完毕，直接返回
}

/*
 * parseline - Parse the command line and build the argv array.
 *
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.
 */
int parseline(const char *cmdline, char **argv) {//传入参数是命令行和要处理的命令行参数
	static char array[MAXLINE]; /* holds local copy of command line */
	char *buf = array;          /* ptr that traverses command line */
	char *delim;                /* points to first space delimiter */
	int argc;                   /* number of args */
	int bg;                     /* background job? */

	strcpy(buf, cmdline);//将命令行一行复制到buf指向的位置中
	buf[strlen(buf) - 1] = ' ';  /* replace trailing '\n' with space */
	while (*buf && (*buf == ' ')) /* ignore leading spaces */
		buf++;

	/* Build the argv list */
	argc = 0;//argc记录参数的个数
	if (*buf == '\'') {
		buf++;//变换指针指向的内容
		delim = strchr(buf, '\'');
	} else {
		delim = strchr(buf, ' ');
	}

	while (delim) {//处理定界符
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while (*buf && (*buf == ' ')) /* ignore spaces */
			buf++;

		if (*buf == '\'') {
			buf++;
			delim = strchr(buf, '\'');
		} else {
			delim = strchr(buf, ' ');
		}
	}
	argv[argc] = NULL;

	if (argc == 0)  /* ignore blank line */
		return 1;//如果是空命令行，返回1。eval函数中进一步判断

	/* should the job run in the background? */
	if ((bg = (*argv[argc - 1] == '&')) != 0) {
		argv[--argc] = NULL;
	}
	return bg;//返回值是bg。若是后台工作，则返回1；否则，则返回0
}

/*
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.
 */
int builtin_cmd(char **argv) {//传入的形参来源于命令行
	if (!strcmp(argv[0], "quit"))//如果命令行是内置quit命令,则直接执行退出
		exit(0);
	if (!strcmp(argv[0], "&"))
		return 1;//返回1，标志着是内置命令
	if (!strcmp(argv[0], "bg")) {//判断命令行是否是fg或者bg，若是则调用函数do_bgfg(argv)，执行相关工作，并返回1，标记为内置命令
		do_bgfg(argv);
		return 1;//返回1，标志着是内置命令
	}
	if (!strcmp(argv[0], "fg")) {
		do_bgfg(argv);
		return 1;//返回1，标志着是内置命令
	}
	if (!strcmp(argv[0], "jobs")) {//如果命令行是内置jobs命令,即显示当前暂停的进程,则调用函数listjobs(jobs),并返回1，标记为内置命令
		listjobs(jobs);
		return 1;//返回1，标志着是内置命令
	}
	return 0;     /* not a builtin command */ //否则，返回0，意味着非内置命令，此返回值便于直接判断是否是内置命令
}

/*
 * do_bgfg - Execute the builtin bg and fg commands
 */
void do_bgfg(char **argv)//传入的形式参数来源于命令行
{
	/* $begin handout */
	struct job_t *jobp = NULL;

	/* Ignore command if no argument */
	if (argv[1] == NULL) {//如果命令行为空，及直接return返回，并打印输出
		printf("%s command requires PID or %%jobid argument\n", argv[0]);
		return;
	}

	/* Parse the required PID or %JID arg */
	if (isdigit(argv[1][0])) {//判断是否是整数，返回值为1，说明可能是pid参数
		pid_t pid = atoi(argv[1]);//调用atoi函数将字符型参数转化为数字
		if (!(jobp = getjobpid(jobs, pid))) {//根据命令行参数中的pid得到对应的job
			printf("(%d): No such process\n", pid);//若调用函数得到返回值为空，则说明没有进程
			return;//错误了直接return返回
		}
	} else if (argv[1][0] == '%') {//如果输入的命令行中包括%，说明其后的参数是jid
		int jid = atoi(&argv[1][1]);//调用atoi函数将字符型参数转化为数字
		if (!(jobp = getjobjid(jobs, jid))) {//根据命令行参数中的jid得到对应的job
			printf("%s: No such job\n", argv[1]);//若调用函数得到返回值为空，则说明没有进程
			return;
		}
	} else {//否则说明无对应的参数，打印输出
		printf("%s: argument must be a PID or %%jobid\n", argv[0]);
		return;
	}

	/* bg command */
	if (!strcmp(argv[0], "bg")) {//如果输入参数由bg，说明是后台运行
		if (kill(-(jobp->pid), SIGCONT) < 0)//向该进程所在的进程组发送SIGCONT信号，若返回值小于0，则说明kill函数错误
			unix_error("kill (bg) error");
		jobp->state = BG;//将此job的state改为BG
		printf("[%d] (%d) %s", jobp->jid, jobp->pid, jobp->cmdline);//打印输出
	}

		/* fg command */
	else if (!strcmp(argv[0], "fg")) {//如果输入参数由fg，说明是后台运行
		if (kill(-(jobp->pid), SIGCONT) < 0)//向该进程所在的进程组发送SIGCONT信号，若返回值小于0，则说明kill函数错误
			unix_error("kill (fg) error");
		jobp->state = FG;//将此job的state改为FG
		waitfg(jobp->pid);//若是前台进程则需要调用waitfg函数，循环阻塞信号直到进程pid不再是前台进程
	} else {
		printf("do_bgfg: Internal error\n");//否则说明处理错误，直接exit即可
		exit(0);
	}
	/* $end handout */
	return;
}

/*
 * waitfg - Block until process pid is no longer the foreground process
 */
void waitfg(pid_t pid) {//传入的形式参数是前台进程pid
	while (pid == fgpid(jobs)) {//循环阻塞信号直到进程pid不再是前台进程，函数fgpid即用于获取前台进程
		sleep(0);//sleep函数中形式参数为0，并不代表无意义，而是类似于暂时”冻结“
	}
	return;//直接返回即可
}

/*****************
 * Signal handlers
 *****************/

/*
 * sigchld_handler - The kernel sends a SIGCHLD to the shell whenever
 *     a child job terminates (becomes a zombie), or stops because it
 *     received a SIGSTOP or SIGTSTP signal. The handler reaps all
 *     available zombie children, but doesn't wait for any other
 *     currently running children to terminate.
 */
void sigchld_handler(int sig)//传入的形参是信号sig
{
    int status,olderrno=errno;//olderrno用于保存原来的errno，因为错误的信号处理函数会修改errno
    pid_t pid;//用于保存引起进程终止的子进程pid
    sigset_t mask_all,prev_all;//mask_all用于保存所有的信号，pre_all用于保存原来信号阻塞向量中的所有信号
    sigfillset(&mask_all);//将每个信号添加到mask_all中
    while((pid = waitpid(-1,&status,WUNTRACED|WNOHANG))>0)//立即返回，若等待集合中的子进程中都没有停止或终止
    {                                                     //则返回0，若存在一个终止或停止，则返回值为子进程的pid
        if(WIFSTOPPED(status))//若引起进程返回的原因是子进程状态是停止的
        {
            struct job_t * job = getjobpid(jobs,pid);//获取此子进程pid所在的job
            if(job&&job->state!=ST)//若job存在且还未收到改变其状态为ST的处理
                printf("Job [%d] (%d) stopped by signal %d\n", pid2jid(pid), pid, WSTOPSIG(status));//终端输出打印
        }
        else if(WIFSIGNALED(status))//若引起进程返回的原因是子进程未捕捉信号终止
        {
            struct job_t * job = getjobpid(jobs,pid);//获取此子进程pid所在的job
            if(job)//若存在此job
            {
                printf("Job [%d] (%d) terminated by signal %d\n", pid2jid(pid), pid, WTERMSIG(status));//终端打印输出
                sigprocmask(SIG_BLOCK, &mask_all, &prev_all);//阻塞所有信号，因为下面要对全局数据结构进行访问
                deletejob(jobs, pid);//将pid所指的job从job结构数组中删除
                sigprocmask(SIG_SETMASK, &prev_all, NULL);//解除阻塞
            }
        }
        else if(WIFEXITED(status))//若引起进程返回的原因是子进程调用exit或者return正常终止
        {
            sigprocmask(SIG_BLOCK,&mask_all,&prev_all);//阻塞所有信号，因为下面要对全局数据结构进行访问
            deletejob(jobs, pid);//将pid所指的job从job结构数组中删除
            sigprocmask(SIG_SETMASK,&prev_all,NULL);//解除阻塞
        }
    }
    errno=olderrno;//恢复原来的errno
    return;//直接返回即可
}

/*
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  Catch it and send it along
 *    to the foreground job.
 */
void sigint_handler(int sig)//传入的形参是信号sig
{
    int olderrno = errno;//用于保存原来的errno，因为错误的信号处理函数会修改errno
    sigset_t mask_all,pre_all;//mask_all用于保存所有的信号，pre_all用于保存原来信号阻塞向量中的所有信号
    sigfillset(&mask_all);//将每个信号添加到mask_all中
    sigprocmask(SIG_BLOCK,&mask_all,&pre_all);//阻塞所有信号，因为下面要对全局数据结构进行访问
    pid_t pid = fgpid(jobs);//获取当前前台作业的pid
    kill(-pid,SIGINT);//根据获取的pid查询前台任务
    printf("Job [%d] (%d) terminated by signal %d\n",pid2jid(pid),pid,sig);//打印输出在终端
    deletejob(jobs,pid);//将pid所指的job从job结构数组中删除
    sigprocmask(SIG_SETMASK,&pre_all,NULL);//解除阻塞
    errno = olderrno;//恢复原来的errno
    return;//直接返回即可
}

/*
 * sigtstp_handler - The kernel sends a SIGTSTP to the shell whenever
 *     the user types ctrl-z at the keyboard. Catch it and suspend the
 *     foreground job by sending it a SIGTSTP.
 */
void sigtstp_handler(int sig)//传入的参数是信号sig
{
    int olderrno = errno;//用于保存原来的errno，因为错误的信号处理函数会修改errno
    sigset_t mask_all,pre_all;//mask_all用于保存所有的信号，pre_all用于保存原来信号阻塞向量中的所有信号
    sigfillset(&mask_all);//将每个信号添加到mask_all中
    sigprocmask(SIG_BLOCK,&mask_all,&pre_all);//阻塞所有信号，因为下面要对全局数据结构进行访问
    pid_t pid = fgpid(jobs);//获取当前前台作业的pid
    struct job_t * job = getjobpid(jobs,pid);//根据获取的pid查询前台任务
    job->state=ST;//将前台任务状态设置为ST,即停止
    kill(-pid,SIGTSTP);//向前台进程组发送SIGSTP的信号
    printf("Job [%d] (%d) stopped by signal %d\n",pid2jid(pid),pid,sig);//打印输出
    sigprocmask(SIG_SETMASK,&pre_all,NULL);//解除阻塞
    errno = olderrno;//恢复原来的errno
    return;//直接返回即可
}

/*********************
 * End signal handlers
 *********************/

/***********************************************
 * Helper routines that manipulate the job list
 **********************************************/

/* clearjob - Clear the entries in a job struct */
void clearjob(struct job_t *job) {
	job->pid = 0;
	job->jid = 0;
	job->state = UNDEF;
	job->cmdline[0] = '\0';
}

/* initjobs - Initialize the job list */
void initjobs(struct job_t *jobs) {
	int i;

	for (i = 0; i < MAXJOBS; i++)
		clearjob(&jobs[i]);
}

/* maxjid - Returns largest allocated job ID */
int maxjid(struct job_t *jobs) {
	int i, max = 0;

	for (i = 0; i < MAXJOBS; i++)
		if (jobs[i].jid > max)
			max = jobs[i].jid;
	return max;
}

/* addjob - Add a job to the job list */
int addjob(struct job_t *jobs, pid_t pid, int state, char *cmdline) {
	int i;

	if (pid < 1)
		return 0;

	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].pid == 0) {
			jobs[i].pid = pid;
			jobs[i].state = state;
			jobs[i].jid = nextjid++;
			if (nextjid > MAXJOBS)
				nextjid = 1;
			strcpy(jobs[i].cmdline, cmdline);
			if (verbose) {
				printf("Added job [%d] %d %s\n", jobs[i].jid, jobs[i].pid, jobs[i].cmdline);
			}
			return 1;
		}
	}
	printf("Tried to create too many jobs\n");
	return 0;
}

/* deletejob - Delete a job whose PID=pid from the job list */
int deletejob(struct job_t *jobs, pid_t pid) {
	int i;

	if (pid < 1)
		return 0;

	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].pid == pid) {
			clearjob(&jobs[i]);
			nextjid = maxjid(jobs) + 1;
			return 1;
		}
	}
	return 0;
}

/* fgpid - Return PID of current foreground job, 0 if no such job */
pid_t fgpid(struct job_t *jobs) {
	int i;

	for (i = 0; i < MAXJOBS; i++)
		if (jobs[i].state == FG)
			return jobs[i].pid;
	return 0;
}

/* getjobpid  - Find a job (by PID) on the job list */
struct job_t *getjobpid(struct job_t *jobs, pid_t pid) {
	int i;

	if (pid < 1)
		return NULL;
	for (i = 0; i < MAXJOBS; i++)
		if (jobs[i].pid == pid)
			return &jobs[i];
	return NULL;
}

/* getjobjid  - Find a job (by JID) on the job list */
struct job_t *getjobjid(struct job_t *jobs, int jid) {
	int i;

	if (jid < 1)
		return NULL;
	for (i = 0; i < MAXJOBS; i++)
		if (jobs[i].jid == jid)
			return &jobs[i];
	return NULL;
}

/* pid2jid - Map process ID to job ID */
int pid2jid(pid_t pid) {
	int i;

	if (pid < 1)
		return 0;
	for (i = 0; i < MAXJOBS; i++)
		if (jobs[i].pid == pid) {
			return jobs[i].jid;
		}
	return 0;
}

/* listjobs - Print the job list */
void listjobs(struct job_t *jobs) {
	int i;

	for (i = 0; i < MAXJOBS; i++) {
		if (jobs[i].pid != 0) {
			printf("[%d] (%d) ", jobs[i].jid, jobs[i].pid);
			switch (jobs[i].state) {
				case BG:
					printf("Running ");
					break;
				case FG:
					printf("Foreground ");
					break;
				case ST:
					printf("Stopped ");
					break;
				default:
					printf("listjobs: Internal error: job[%d].state=%d ",
						   i, jobs[i].state);
			}
			printf("%s", jobs[i].cmdline);
		}
	}
}
/******************************
 * end job list helper routines
 ******************************/


/***********************
 * Other helper routines
 ***********************/

/*
 * usage - print a help message
 */
void usage(void) {
	printf("Usage: shell [-hvp]\n");
	printf("   -h   print this message\n");
	printf("   -v   print additional diagnostic information\n");
	printf("   -p   do not emit a command prompt\n");
	exit(1);
}

/*
 * unix_error - unix-style error routine
 */
void unix_error(char *msg) {
	fprintf(stdout, "%s: %s\n", msg, strerror(errno));
	exit(1);
}

/*
 * app_error - application-style error routine
 */
void app_error(char *msg) {
	fprintf(stdout, "%s\n", msg);
	exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) {
	struct sigaction action, old_action;

	action.sa_handler = handler;
	sigemptyset(&action.sa_mask); /* block sigs of type being handled */
	action.sa_flags = SA_RESTART; /* restart syscalls if possible */

	if (sigaction(signum, &action, &old_action) < 0)
		unix_error("Signal error");
	return (old_action.sa_handler);
}

/*
 * sigquit_handler - The driver program can gracefully terminate the
 *    child shell by sending it a SIGQUIT signal.
 */
void sigquit_handler(int sig) {
	printf("Terminating after receipt of SIGQUIT signal\n");
	exit(1);
}
