#include "mbed.h"
#include "rtos.h"
#include "stdio.h"

//main1
DigitalIn SW1(PTD1, PullUp);
DigitalIn SW2(PTD3, PullUp);

void task1(void);
void task3(void);

Thread T1;

Serial board(PTE0, PTE1);
Serial pc(USBTX, USBRX); //TX , RX

char h, m, s;
char str;

int counter = 0;

float temp_buf[32];
int temp_idx = 0;
double temp_avg = 0;
float tempIn = 0.0;

bool S1 = false;
bool S2 = false;
bool full_run = false;

int main() {
    T1.start(task1);
    
    while(true){
        if (SW1 == 0){
            S1 = true;
        }
        if (SW2 == 0){
            S2 = true;
        }
        if(board.readable()){
            str = board.getc();
            pc.putc(str);
            if (str == 'S')
            {
                task3();
            }
        }
    }  
}

void task1(void)
{
    while(true){
        if(S1){
            while (!board.writeable()){};
            board.putc('W');
            S1 = false;
        }
        if (S2){
            while (!board.writeable()){};
            board.putc('Z');
            S2 = false;
        }
        Thread::wait(100);
        
        counter++;
        
        if (counter % 50 == 0)
        {
            while (!board.writeable()){};
            board.putc('B'); // Both T2 and T4 should be Triggered
            counter = 0;
            pc.printf("T2 & T4 trig\n\r");
        }
        else if (counter % 10 == 0)
        {
            while (!board.writeable()){};
            board.putc('F'); // T4 should be Triggered
            pc.printf("T4 trig\r\n");
        }
    }
}

void task3(void)
{
    while (!board.readable());
    h = board.getc();
    m = board.getc();
    s = board.getc();
    
    pc.printf("Rec Time: %d:%d:%d\n\r", (int)(h), (int)(m), (int)(s));
    
    char a = board.getc();
    char b = board.getc();
    
    tempIn = (float)((float)(a) + ((float)(b) / 100.0));
    pc.printf("Rec Temp: %f\n\r", tempIn);
    temp_buf[temp_idx++] = tempIn;
    if(temp_idx == 32){
        full_run = true;
    }
    temp_idx = temp_idx % 32;
    
    temp_avg = 0;
    for (int i = 0; i < temp_idx; i++)
    {
        temp_avg = temp_avg + temp_buf[i];
    }
    if(!full_run){
        temp_avg = temp_avg / (float)(temp_idx);
    }
    else{
        temp_avg = temp_avg / 32.0;
    }
}