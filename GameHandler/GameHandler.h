#ifndef _GameHandler_h_
#define _GameHandler_h_

#include <Arduino.h>
#include <Target.h>
#define STANDTERM 3000    // 과녁 기립쿨타임 
#define DEBOUNCETERM 200  // 센서 디바운스 텀
#define TIMELIMIT 20
#define STOP 0
#define MANUAL 1

typedef class gameHandler {
    private:
        target * objArr[10];  // 과녁객체
        int index;            // 과녁 색인
		
    public:
	    int standTerm;
		
        gameHandler();
        void clipOut();
        void showAllInfo();
        void statusMonitor();
        void addTarget(int, int, int, int, int, int, int, int, int);
        void addTarget();
        void delTarget();
        void gamemode0();
        void gamemode1();
        void gamemode2();
        void disableMotorAll();
        void enableMotorAll();
        void activateMotorAll();
        void downAll();
		void setStandTerm(int n);
        int checkCommend();
        int getNum_ObjArr();
        bool checkTimeOver();
        bool checkAllHit();
} gameHandler;

#endif
