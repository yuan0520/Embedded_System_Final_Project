// #include <stdlib.h>
#include <stdio.h>
#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"

Ticker servo_ticker;
Timer t;
PwmOut pin5(D5), pin6(D6);
DigitalInOut ping(D11);
BufferedSerial xbee(D10, D9);
BufferedSerial uart(D1, D0); //tx,rx
// BufferedSerial pc(USBTX, USBRX);

BBCar car(pin5, pin6, servo_ticker);

int main() {

    double pwm_table0[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table0[] = {-16.582, -16.263, -15.227, -11.879, -5.820, 0.000, 6.059, 12.038, 15.147, 16.263, 16.742};
    double pwm_table1[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table1[] = {-16.582, -16.263, -15.227, -11.879, -5.820, 0.000, 6.059, 12.038, 15.147, 16.263, 16.742};

    car.setCalibTable(11, pwm_table0, speed_table0, 11, pwm_table1, speed_table1);

    uart.set_baud(9600);
    // xbee.set_baud(9600);
    // pc.set_baud(9600);

    char buf[256], outbuf[256];
    FILE *devin = fdopen(&xbee, "r");
    FILE *devout = fdopen(&xbee, "w");
    while(1) {
        memset(buf, 0, 256);
        for (int i = 0; ; i++) {
            char recv = fgetc(devin);
            if (recv == '\n') {
                // printf("OK\r\n");
                break;
            }
            buf[i] = fputc(recv, devout);
        }
        RPC::call(buf, outbuf);
        break;
    }

    ThisThread::sleep_for(2s);

    char buf2[256], outbuf2[256];
    FILE *devin2 = fdopen(&uart, "r");
    FILE *devout2 = fdopen(&uart, "w");
    while(1) {
        memset(buf2, 0, 256);
        for (int i = 0; ; i++) {
            char recv2 = fgetc(devin2);
            if (recv2 == '\n') {
                // printf("OK\r\n");
                break;
            }
            buf2[i] = fputc(recv2, devout2);
        }
        RPC::call(buf2, outbuf2);
        break;
    }

    ThisThread::sleep_for(1s);

    while(1) {

        if(uart.readable()){
            char recv3[1];
            uart.read(recv3, sizeof(recv3));
            if(recv3[0] == 'l'){
                car.turn_my_carlib(6, -1);
            }
            else if(recv3[0] == 'r'){
                car.turn_my_carlib(6, 1);
            }
            else{
                car.goStraightCalib(6);
                ThisThread::sleep_for(3000ms);
                car.stop();
                xbee.write("OK\n", 4);
                break;
            }
            ThisThread::sleep_for(1000ms);
        }

    }


}