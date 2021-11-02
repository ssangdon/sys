#include "ku_ps_input.h"
#include <time.h>

clock_t times(struct tms *buffer);


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
            printf("Received Message %d %ld\n", mymsg.value, mymsg.id);
            result += mymsg.value;
        }
    }
    if (msgctl(mqdes, IPC_RMID, NULL) == -1)
    {
        printf("msgctl failed\n");
        exit(0);
    }
    return result;
}

int main(int argc, char *argv[])
{
    
    int k;
    time_t t;
    struct tms mytms;
    clock_t t1, t2;

    if((t1=times(&mytms)==-1)){
        perror("times1");
        exit(1);
    }
    for(k=0; k<999999; k++){
        time(&t);
    }
    if((t2=times(&mytms)==-1)){
        perror("times2");
        exit(1);
    }


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
        pid_t pid[range[2]];
        int childState;
        // for(int i =0; i<10; i++){
        //     printf("%d\n", input[i]);
        // }
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
        for (int i = 0; i < range[2]; i++)
        {
            if ((pid[i] = fork()) == 0)
            {
                //자식 프로세스들이 할것
                //프로세스별로 숫자를 어떻게 나눌까
                int p = Search(arr[i], arr[i + 1], range[0], range[1], input);
                // printf("%d %d \n", p, getpid());
                // printf("%d %d \n ", arr[i], arr[i+1]);
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
                printf("Sending a Message %d %ld\n", mymsg.value, mymsg.id);
                if (msgsnd(mqdes, &mymsg, buf_len, 0) == -1)
                {
                    perror("msgsnd()");
                    exit(0);
                }
                exit(100 + i);
            }
            else
            {
                //reaping 해줘야 함
                int ret = waitpid(pid[i], &childState, 0);
                // printf("부모 종료 %d %d %d\n", ret, WIFEXITED(childState), WEXITSTATUS(childState));
            }
        }
        //메세지큐로 모든 수를 받아오는것!
        int kk = receiver(range[2]);
        printf("%d\n", kk);
        printf("RT : %.1f sec\n", (double)(t2-t1)/CLK_TCK);
        printf("UT : %.1f sec\n", (double)(mytms.tms_utime)/CLK_TCK);
        printf("ST : %.1f sec\n", (double)(mytms.tms_stime)/CLK_TCK);
        return 0;
    }
}