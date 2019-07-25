#include "GameHandler.h"

gameHandler::gameHandler() {
    for (int i = 0; i < 10; i++) {
        objArr[i] = NULL;
    }
	standTerm = STANDTERM;
}

void gameHandler::clipOut() {
    Serial.print("const int gmPin[] = {");
    for(int i = 0; i < index; i++) {
        Serial.print(objArr[i]->clipOut(gmP));
        Serial.print(", ");
    }
    Serial.println("};");
    
    Serial.print("const int cmPin[] = {");
    for(int i = 0; i < index; i++) {
        Serial.print(objArr[i]->clipOut(cmP));
        Serial.print(", ");
    }
    Serial.println("};");
    
    Serial.print("const int sPin[] = {");
    for(int i = 0; i < index; i++) {
        Serial.print(objArr[i]->clipOut(sP));
        Serial.print(", ");
    }
    Serial.println("};");
    
    Serial.print("const int lPin[] = {");
    for(int i = 0; i < index; i++) {
        Serial.print(objArr[i]->clipOut(lP));
        Serial.print(", ");
    }
    Serial.println("};");
    
    Serial.print("const int score[] = {");
    for(int i = 0; i < index; i++) {
        Serial.print(objArr[i]->clipOut(score_));
        Serial.print(", ");
    }
    Serial.println("};");
    
    Serial.print("const int gm_zeroList[] = {");
    for(int i = 0; i < index; i++) {
        Serial.print(objArr[i]->clipOut(zero_gm));
        Serial.print(", ");
    }
    Serial.println("};");
    
    Serial.print("const int gm_actList[] = {");
    for(int i = 0; i < index; i++) {
        Serial.print(objArr[i]->clipOut(act_gm));
        Serial.print(", ");
    }
    Serial.println("};");
    
    Serial.print("const int cm_zeroList[] = {");
    for(int i = 0; i < index; i++) {
        Serial.print(objArr[i]->clipOut(zero_cm));
        Serial.print(", ");
    }
    Serial.println("};");
    
    Serial.print("const int cm_actList[] = {");
    for(int i = 0; i < index; i++) {
        Serial.print(objArr[i]->clipOut(act_cm));
        Serial.print(", ");
    }
    Serial.println("};");
}

void gameHandler::showAllInfo() {
    for (int i = 0; i < index; i++) {
        Serial.print("*********Index "); Serial.print(i); Serial.println("*********");
        objArr[i]->showInfo();
    }
}

void gameHandler::statusMonitor() {
	boolean breakTrig = false;
	
    for (;;) {
		if(breakTrig == true) break;
		if(Serial.available()) {
			switch(checkCommend()) {
				case STOP:
				    breakTrig = true;
					break;
				default:
				    Serial.println("???");
			}
	    }
        Serial.println("---Target Monitor---");
        for (int i = 0; i < index; i++) {
            Serial.print("SensorValue : "); Serial.println(objArr[i]->getSensorValue());
        }
        delay(250);
    }
}

void gameHandler::addTarget(int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8, int n9) {
    objArr[index] = new target(n1, n2, n3, n4, n5, n6, n7, n8, n9);
    index++;
}

void gameHandler::addTarget() {    
    Serial.println("*****************************과녁의 정보를 입력해주세요*****************************");
    Serial.println("입력서식(포맷) : (groundMotor핀), (coverMotor핀), (sensor핀), (led핀), (부여점수), (zero계수_gm), (act계수_gm), (zero계수_cm), (act계수_cm)");
    Serial.print(">> ");

    for (;;) {
        if (Serial.available()) {
            String recieve = Serial.readStringUntil('\r'); Serial.println(recieve);
            int buf[9] = {0, };

            int checker = sscanf(recieve.c_str(), "%d, %d, %d, %d, %d, %d, %d, %d, %d", &buf[0], &buf[1], &buf[2], &buf[3], &buf[4], &buf[5], &buf[6], &buf[7], &buf[8]);

            if (checker == 9) {
                Serial.print("Ground 핀 : "); Serial.println(buf[0]);
                Serial.print("Cover 핀 : "); Serial.println(buf[1]);
                Serial.print("Sensor 핀 : "); Serial.println(buf[2]);
                Serial.print("LED 핀 : "); Serial.println(buf[3]);
                Serial.print("부여 점수 : "); Serial.println(buf[4]);
                Serial.print("GroundMotor_Zero 계수 : "); Serial.println(buf[5]);
                Serial.print("GroundMotor_Act 계수 : "); Serial.println(buf[6]);
                Serial.print("CoverMotor_Zero 계수 : "); Serial.println(buf[7]);
                Serial.print("CoverMotor_Act 계수 : "); Serial.println(buf[8]);
                objArr[index] = new target(buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8]);
                index++;
                break;
            }
        }
    }
}

/*
    void gameHandler::delTarget() {
    int target = 0;
    int target_index = 0;
    String recieve;

    if (serial.recieveProcess_waitting()) { // 이 부분 수정 바람
        recieve = serial.getCommend();

        target = recieve.toInt();
        target_index = target - 1;  // 실제 인덱스 넘버
    }
    delete objArr[target_index];
    index--;

    for (int i = 0; i < index - target; i++) {
        objArr[target_index + i] = objArr[target_index + i + 1];
    }
    objArr[index - 1] = NULL; // 스왑 마지막 줄에 있던 공간을 비워 오류 제거
    }
*/

void gameHandler::gamemode0() {
    boolean breakTrig = false;
    boolean checkEntryTrig[] = {false, };
    boolean motorActTrig[] = {false, };
    boolean lastState[] = {false, };
    boolean reverseState[] = {false, };
    unsigned int debouncePoint[] = {0, };
    unsigned int lastStandPoint[] = {0, };
    unsigned int motorActPoint[] = {0, };
    unsigned int c = 0;
    int lastScore = 0;
    int i = 0;

    for (;;) {
        c = millis();
        
        if(breakTrig == true) break;
		
        switch(checkCommend()) {
            case STOP:
                breakTrig = true;
                break;
            case MANUAL:
                Serial.print("Which target? : ");
                
                for (;;) {
                    c = millis();
                    
                    if (Serial.available()) {
                        int recieve = Serial.parseInt();

                        Serial.print("Target is "); Serial.println(recieve);
                        checkEntryTrig[recieve - 1] = true;
                        break;
                    }
                }
				break;
			default:
			    ;
        }

        for (i = 0; i < index; i++) {
            boolean targetState = objArr[i]->getStat();

            if (targetState != lastState[i]) {
                debouncePoint[i] = c;
            }

            if (c - debouncePoint[i] >= DEBOUNCETERM) {             // 디바운스텀 이상
                if (targetState == LOW && checkEntryTrig[i] == false) {  //  피격 상태 그리고 최초 상태 변화 감지 시
                    checkEntryTrig[i] = true;
                    objArr[i]->recordHitTime();          // 피격 시점 저장
                    lastScore += objArr[i]->getScore();  // 점수 계산
                    Serial.print("you get "); Serial.println(objArr[i]->getScore());
                    lastState[i] = targetState;
                }
            }

            if (checkEntryTrig[i] == true) {
                if (c - objArr[i]->getHitTime() >= standTerm || c - lastStandPoint[i] >= 5000) {  // 피격 3초 후 또는 저번 피격 으로부터 5초 후
                    if (motorActTrig[i] == false) { // 작동 시작 시점 저장
                        motorActPoint[i] = c;
                        motorActTrig[i] = true;
                        objArr[i]->enableMotor();
                    }

                    if (reverseState[i] == false) {   // act 위치로 이동
                        if (c - motorActPoint[i] >= 250) {
                            reverseState[i] = true;
                            motorActTrig[i] = false;
                        }
                        else {
                            objArr[i]->activateGroundMotor_act();
                        }
                    }
                    else {    // zero 위치로 이동
                        if (c - motorActPoint[i] >= 250) {
                            reverseState[i] = false;
                            motorActTrig[i] = false;
                            // zero 위치로 이동하는 과정 까지 마친 후 트리거 해체
                            checkEntryTrig[i] = false;
                            lastStandPoint[i] = c;   // 저번 기립 시점 저장
                            debouncePoint[i] = c;    // 기립 하면서도 생기는 플로팅을 위한 디바운스
                            objArr[i]->disableMotor();
                        }
                        else {
                            objArr[i]->activateGroundMotor_zero();
                        }
                    }
                }
            }
            lastState[i] = targetState;
            objArr[i]->updateStatus();  // 과녁 상태 업데이트
        }
    }
    Serial.print("Last score is "); Serial.println(lastScore);  // 점수 출력
}

void gameHandler::gamemode1() {
    boolean checkEntryTrig[] = {false, false, };
    boolean motorActTrig[] = {false, false, };
    boolean lastState[] = {false, false, };
    boolean reverseState[] = {false, false, };
    unsigned int debouncePoint[] = {0, };
    unsigned int lastStandPoint[] = {0, };
    unsigned int motorActPoint[] = {0, };
    unsigned int c = 0;
    int lastScore = 0;
    int i = 0;

    for (;;) {
        c = millis();

        if (checkTimeOver() == true) {  //
            break;
        }

        for (i = 0; i < index; i++) {
            boolean targetState = objArr[i]->getStat();

            if (targetState != lastState[i]) {
                debouncePoint[i] = c;
            }

            if (c - debouncePoint[i] >= DEBOUNCETERM) {             // 디바운스텀 이상
                if (targetState == LOW && checkEntryTrig[i] == false) {  //  피격 상태 그리고 최초 상태 변화 감지 시
                    checkEntryTrig[i] = true;
                    objArr[i]->recordHitTime();          // 피격 시점 저장
                    lastScore += objArr[i]->getScore();  // 점수 계산
                    Serial.print("you get "); Serial.println(objArr[i]->getScore());
                    lastState[i] = targetState;
                }
            }

            if (checkEntryTrig[i] == true) {
                if (c - objArr[i]->getHitTime() >= standTerm || c - lastStandPoint[i] >= 5000) {  // 피격 3초 후 또는 저번 피격 으로부터 5초 후
                    if (motorActTrig[i] == false) { // 작동 시작 시점 저장
                        motorActPoint[i] = c;
                        motorActTrig[i] = true;
                        objArr[i]->enableMotor();
                    }

                    if (reverseState[i] == false) {   // act 위치로 이동
                        if (c - motorActPoint[i] >= 250) {
                            reverseState[i] = true;
                            motorActTrig[i] = false;
                        }
                        else {
                            objArr[i]->activateGroundMotor_act();
                        }
                    }
                    else {    // zero 위치로 이동
                        if (c - motorActPoint[i] >= 250) {
                            reverseState[i] = false;
                            motorActTrig[i] = false;
                            // zero 위치로 이동하는 과정 까지 마친 후 트리거 해체
                            checkEntryTrig[i] = false;
                            lastStandPoint[i] = c;   // 저번 기립 시점 저장
                            debouncePoint[i] = c;    // 기립 하면서도 생기는 플로팅을 위한 디바운스
                            objArr[i]->disableMotor();
                        }
                        else {
                            objArr[i]->activateGroundMotor_zero();
                        }
                    }
                }
            }
            lastState[i] = targetState;
            objArr[i]->updateStatus();  // 과녁 상태 업데이트
        }
    }
    Serial.print("Last score is "); Serial.println(lastScore);  // 점수 출력
}

void gameHandler::disableMotorAll() {
    for (int i = 0; i < index; i++) {
        objArr[i]->disableMotor();
    }
}

void gameHandler::enableMotorAll() {
    for (int i = 0; i < index; i++) {
        objArr[i]->enableMotor();
    }
}

void gameHandler::activateMotorAll() {
	enableMotorAll();
    for (int i = 0; i < index; i++) {
        objArr[i]->activateGroundMotor_act();
    }
    delay(500);
    for (int i = 0; i < index; i++) {
        objArr[i]->activateGroundMotor_zero();
    }
    delay(500);
}

void gameHandler::downAll() {
    for (int i = 0; i < index; i++) {
        objArr[i]->activateCoverMotor_act();
    }
    delay(500);
    for (int i = 0; i < index; i++) {
        objArr[i]->activateCoverMotor_zero();
    }
    delay(500);
}

void gameHandler::setStandTerm(int n) {
	standTerm = n;
}

bool gameHandler::checkTimeOver() {
    static boolean entryTrig = false;
    static int clock = 0;
    static unsigned int c = 0;
    static unsigned int p = 0;

    c = millis();
    if (entryTrig == false) {
        clock = TIMELIMIT;
        entryTrig = true;
        p = c;
    }
    else {
        if (c - p >= 1000) {
            Serial.print("Rest TIME : "); Serial.println(clock);
            clock--;
            p = c;
        }
        if (clock < 0 ) {
            entryTrig = false;
            return true;
        }
    }
    return false;
}

bool gameHandler::checkAllHit() {
    static int i = 0;
    static int count = 0;

    for (i = 0; i < index; i++) {
        if (objArr[i]->getStat()) {  // 기립상태 확인
            count++;  // 다운된 과녁수를 계산함
        }
    }
    if (count >= index) {  // 다운된 과녁수가 index개이상인지 확인
        count = 0;  // 다음 턴을 위해 카운트수를 초기화
        return true;
    }
    else {
        return false;
    }
}

int gameHandler::checkCommend() {
    if (Serial.available()) {
            String recieve = Serial.readStringUntil('\r');

            if (!recieve.compareTo("stop")) {
                return STOP;
            }
            else if (!recieve.compareTo("manual")) {
                return MANUAL;
            }
            else {
                ;
            }
        }
		return -1;
}

int gameHandler::getNum_ObjArr() {
    return index;
}
