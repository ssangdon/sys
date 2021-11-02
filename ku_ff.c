#include "ku_ps_input.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <math.h>
#include <sys/times.h>
#include <limits.h>
#include <time.h>
#include <sys/wait.h>

int Search(int start, int end, int a1, int a2, int input[])
{
    int nums = 0;
    for (int i = start; i < end; i++)
    {
        if (input[i] >= a1 && input[i] <= a2)
        {
            nums++;
        }
    }
    return nums;
}

int receiver(int limit)
{
    key_t ipckey;
    int mqdes, k;
    int result = 0;
    size_t buf_len;
    struct
    {
        long id;
        int value;
    } mymsg;
    buf_len = sizeof(mymsg.value);
    ipckey = ftok("./tmp/foo", 1998);
    mqdes = msgget(ipckey, IPC_CREAT | 0600);
    if (mqdes < 0)
    {
        perror("msgget()");
        exit(0);
    }
    for (int i = 0; i < limit; i++)
    {
        if (msgrcv(mqdes, &mymsg, buf_len, i + 1, 0) == -1)
        {
            perror("msgrcv()");
            exit(0);
        }
        else
        {
            result += mymsg.value;
        }
    }
    //메시지큐에서 값을 다 받아오면 큐를 지워주는 동기화 부분
    if (msgctl(mqdes, IPC_RMID, 0) == -1)
    {
        printf("msgctl failed\n");
        exit(0);
    }
    return result;
}

int main(int argc, char *argv[])
{
    int k;
    int range[3];
    for (int i = 0; i < 3; i++)
    {
        range[i] = atoi(argv[i + 1]);
    }
    if (range[0] > range[1])
    {
        printf("첫번째 수가 두번째 수보다 클 수 없습니다.");
        return 0;
    }
    else
    {
        //프로세스 생성
        clock_t start, end;
        double result;
        pid_t pid[range[2]];
        int childState;
        int arr[range[2] + 2];
        for (int i = 0; i < range[2] + 2; i++)
        {
            arr[i] = 0;
        }
        arr[range[2]] = NUMS;
        int process_num = (NUMS / range[2]);
        //마지막 프로세스로 값이 몰리는 것을 방지하기 위한 코드
        if (process_num == 0)
        {
            process_num = 1;
        }
        for (int i = 1; i < range[2]; i++)
        {
            arr[i] = arr[i - 1] + process_num;
        }
        //자식프로세스들이 처리해야할일 구현 함수
        //start = clock();
        for (int i = 0; i < range[2]; i++)
        {
            if ((pid[i] = fork()) == 0)
            {
                //자식 프로세스들이 할것
                int p = Search(arr[i], arr[i + 1], range[0], range[1], input);

                //메세지 큐
                key_t ipckey;
                int mqdes, k;
                size_t buf_len;
                struct
                {
                    long id;
                    int value;
                } mymsg;
                buf_len = sizeof(mymsg.value);
                ipckey = ftok("./tmp/foo", 1998);
                mqdes = msgget(ipckey, IPC_CREAT | 0600);
                if (mqdes < 0)
                {
                    perror("msgget()");
                    exit(0);
                }
                mymsg.id = i + 1;
                mymsg.value = p;
                if (msgsnd(mqdes, &mymsg, buf_len, IPC_NOWAIT) == -1)
                {
                    perror("msgsnd()");
                    exit(0);
                }
                exit(100 + i);
            }
            else
            {
                //부모 프로세스 Reaping해주는 것
                int ret = waitpid(pid[i], &childState, 0);
            }
        }
        // end = clock();
        //메세지큐로 모든 수를 받아오는것!
        int answer = receiver(range[2]);
        printf("%d\n", answer);
        // result = (double)(end - start);
        // printf("%0.1f", result);
        return 0;
    }
}
