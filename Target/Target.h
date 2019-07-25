#ifndef _Target_h_
#define _Target_h_

#include <Arduino.h>
#include <Servo.h>

enum {
    zero, act,
    gmP=0, cmP=1, sP=2, lP=3, score_=4, zero_gm=5, act_gm=6, zero_cm=7, act_cm=8
};

typedef class target {
    private:
        int gmPin;             // 해당 과녁에 구성된 과녁 기립제어 모터 핀
        int cmPin;             // 해당 과녁에 구성된 과녁 기립제어 모터 핀
        int sPin;              // 해당 과녁에 구성된 센서 핀
        int lPin;              // 해당 과녁에 구성된 LED 핀
        int score;             // 해당 과녁의 점수
        int posArray_gm[2];    // 과녁 영점계수 배열 (ground motor)
        int posArray_cm[2];    // 과녁 영점계수 배열 (cover motor)
        unsigned int hitTime;  // 피격시점 저장 변수
        bool stat;             // 과녁 상태
        Servo groudMotor;      // 바닥에 부착된 모터 
        Servo coverMotor;      // 방탄판에 부착된 모터
    public:
        target();
        target(int gmP, int cmP, int sP, int lP, int num_score, int num_zero_gm, int num_act_gm, int num_zero_cm, int num_act_cm);
        target(int infoArray[]);
        int clipOut(int index);
        void showInfo();                   // 과녁 하드웨어 정보 표시
        void activateGroundMotor_act();    // 과녁대 기립
        void activateGroundMotor_zero();   // 모터 정위치
        void activateCoverMotor_act();     // 과녁대 강제 다운
        void activateCoverMotor_zero();    // 모터 정위치
        void ledOn();               // LED 온
        void ledOff();              // LED 오프
        void disableMotor();        // 모터 비활성화
        void enableMotor();         // 모터 활성화
        void updateStatus();        // 해당 과녁 상태 업데이트
        void recordHitTime();       // 피격 시점 기억
        int getHitTime();           // 피격 시점 반환
        int getScore();             // 점수 반환
        int getSensorValue();       // 센서값 반환
        bool getStat();
} target;

#endif
