// #include <stdlib.h>
#include <stdio.h>
#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"

Ticker servo_ticker;
Timer t;
PwmOut pin5(D5), pin6(D6);
DigitalInOut ping(D11);
BufferedSerial xbee(D1, D0);
BufferedSerial uart(D1, D0); //tx,rx
// BufferedSerial pc(USBTX, USBRX);

BBCar car(pin5, pin6, servo_ticker);

void openmv_detect(int state){
    float val;
    
    ping.output();
    ping = 0; wait_us(200);
    ping = 1; wait_us(5);
    ping = 0; wait_us(5);

    ping.input();
    while(ping.read() == 0);
    t.start();
    while(ping.read() == 1);
    val = t.read();
    if(!state){
        char buf1[100];
        sprintf(buf1, "Distance before carlibration = %lf\r\n", val*17700.4f);
        xbee.write(buf1, sizeof(buf1));
    }
    else{
        char buf1[100];
        sprintf(buf1, "Distance after carlibration = %lf\r\n", val*17700.4f);
        xbee.write(buf1, sizeof(buf1));
    }
    t.stop();
    t.reset();

    ThisThread::sleep_for(1s);
    
}

int main() {

    double pwm_table0[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table0[] = {-16.582, -16.263, -15.227, -11.879, -5.820, 0.000, 6.059, 12.038, 15.147, 16.263, 16.742};
    double pwm_table1[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table1[] = {-16.582, -16.263, -15.227, -11.879, -5.820, 0.000, 6.059, 12.038, 15.147, 16.263, 16.742};

    car.setCalibTable(11, pwm_table0, speed_table0, 11, pwm_table1, speed_table1);

    uart.set_baud(9600);
    xbee.set_baud(9600);
    // pc.set_baud(9600);

    openmv_detect(0);

    while(1) {

        if(uart.readable()){
            char recv[1];
            uart.read(recv, sizeof(recv));
            if(recv[0] == 'l'){
                // printf("left\n");
                car.turn_my_carlib(5, -1);
            }
            else if(recv[0] == 'r'){
                // printf("right\n");
                car.turn_my_carlib(5, 1);
            }
            else{
                char buf[100];
                sprintf(buf, "center\n");
                xbee.write(buf, sizeof(buf));
                car.goStraightCalib(5);
                ThisThread::sleep_for(500ms);
                car.stop();
                openmv_detect(1);
                break;
            }
            ThisThread::sleep_for(1000ms);
        }

    }

}