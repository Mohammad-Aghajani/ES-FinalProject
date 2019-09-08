#include "mbed.h"
#include "rtos.h"
#include "stdio.h"
#include "temp.h"

//main2
void get_curr_time(void);
void task2(void);
void task4(void);

Thread T2, T4;
Serial board(PTE0, PTE1);
Serial pc(USBTX, USBRX); //TX , RX

char str[20];
char* time_str;
char* tmp;
char cmd[20];
char command;
char* cur_tim;

int user_tim[3] = {0}; //H , M, S
int asso_tim[3] = {0};

float curr_temp;
float avg_temp;

int temp_idx = 0;
bool flag2 = false;
bool flag4 = false;
bool SW1 = false;
bool SW2 = false;
bool full_run = false;

int main()
{
    get_curr_time();
    T2.start(task2);
    while(!pc.writeable()){};
    pc.printf("Thread 2 started\n\r");
    T4.start(task4);
    while(!pc.writeable()){};
    pc.printf("Thread 4 started\n\r");
    
    while(true){
        if (board.readable()){
            command = board.getc();
            pc.printf("Command %c Rec2 f1\n\r", command);
            
            if(command == 'F'){
                flag4 = true;
            }
            else if (command == 'B'){
                flag2 = true;
                flag4 = true;
            }
            else if (command == 'W'){
                SW1 = true;
            }
            else if (command == 'Z'){
                SW2 = true;
            }
        }
    }
}

void get_curr_time(void){
    pc.printf("Enter current time in format HH:MM:SS\n\r");
    while (!pc.readable()){};
    pc.scanf("%s", str);
    //HH:MM:SS
    user_tim[0] = ((str[0] - '0') * 10) + ((str[1] - '0'));
    user_tim[1] = ((str[3] - '0') * 10) + ((str[4] - '0'));
    user_tim[2] = ((str[6] - '0') * 10) + ((str[7] - '0'));
    pc.printf("%s\n\r", str);
}

void task2(void)
{
    while (true)
    {
        if (flag2) {
            curr_temp = getTemp(temp_idx++);
            if (temp_idx == 32){
                full_run = true;
            }
            temp_idx = temp_idx % 64;
            flag2 = false;
            while(!board.writeable());
            pc.printf("Task 3 Trig\n\r");
            board.putc('S');
            for (int i = 0; i<3; i++){
                asso_tim[i] = user_tim[i];
            }
            while (!board.writeable());
            
            pc.printf("Send asso time to task3\n\r");
            
            board.putc(asso_tim[0]);
            board.putc(asso_tim[1]);
            board.putc(asso_tim[2]);
            
            while (!board.writeable());
            pc.printf("Send curr temp to task3\n\r");
            //curr_temp = a.b
            int a = (int)(curr_temp);
            board.putc(a);
            int b = (int)(100.0 * (curr_temp - (float)(a)));
            board.putc(b);
        }
    }
}

void task4 (void)
{
    while(true){
        if (flag4){
            flag4 = false;
            user_tim[2] = (user_tim[2] + 1) % 60;
            if (user_tim[2] == 0){
                user_tim[1] = (user_tim[1] + 1) % 60;
            }
            if (user_tim[1] == 0){
                user_tim[0] = (user_tim[0] + 1) % 24;
            }
            pc.printf("T4->Time is: %d:%d:%d\n\r", user_tim[0], user_tim[1], user_tim[2]);
            
            if (SW1){
                pc.printf("CurTemp is: %f at %d:%d:%d\n\r", curr_temp, asso_tim[0], asso_tim[1], asso_tim[2]);
                SW1 = false;
            }
            if (SW2){
                if (!full_run){
                    avg_temp = 0.0;
                    for(int i = 0; i < temp_idx; i++)
                    {
                        avg_temp = avg_temp + (getTemp(i) / (float)temp_idx);
                    }
                }
                else
                {
                    avg_temp = 0.0;
                    for (size_t i = 1; i <= 32; i++)
                    {
                        avg_temp = avg_temp + (getTemp(temp_idx - i) / 32.0);
                    }   
                }
                pc.printf("AvgTemp is:%f at %d:%d:%d\n\r", avg_temp, asso_tim[0], asso_tim[1], asso_tim[2]);
                SW2 = false;
            }
        }
    }
}