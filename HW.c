#include "ku_ff_input.h"

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

int producer(int limit1, int val)
{
    key_t ipckey;
    int mqdes, k;
    size_t buf_len;
    struct
    {
        long id;
        int value;
    } mymsg;
    buf_len = sizeof(mymsg.value);
    ipckey = ftok("./tmp/foo", 1946);
    mqdes = msgget(ipckey, IPC_CREAT | 0600);
    if (mqdes < 0)
    {
        perror("msgget()");
        exit(0);
    }
    mymsg.id = limit1 + 1;
    mymsg.value = val;
    printf("Sending a Message %d %ld\n", mymsg.value, mymsg.id);
    if (msgsnd(mqdes, &mymsg, buf_len, 0) == -1)
    {
        perror("msgsnd()");
        exit(0);
    }
    return 0;
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
    ipckey = ftok("./tmp/foo", 1946);
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
    return result;
}

int main(int argc, char *argv[])
{
    int range[3];
    for (int i = 0; i < 3; i++)
    {
        range[i] = atoi(argv[i + 1]);
    }
    // for(int i =0; i<3; i++){
    //     printf("%d\n", range[i]);
    // }
    if (range[0] > range[1])
    {
        printf("첫번째 수가 두번째 수보다 클 수 없습니다. ");
    }
    else if (range[0] == range[1])
    {
        printf("두 값이 같습니다.");
    }
    else
    {
        //쓰레드 생성
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
        // int remain_num = NUMS%range[2];
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

                //메세지 큐
                producer(i, p);
                exit(100 + i);
            }
            else
            {

                //리시버
                //reaping 해줘야 함
                int ret = waitpid(pid[i], &childState, 0);
                // printf("부모 종료 %d %d %d\n", ret, WIFEXITED(childState), WEXITSTATUS(childState));
            }
        }
        int kk = receiver(range[2]);
        printf("%d\n", kk);
        return 0;
    }
}