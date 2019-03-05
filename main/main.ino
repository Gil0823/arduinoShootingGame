#include <SoftwareSerial.h>
#include <Servo.h>
#include <SimpleTimer.h>
#define NUM_T 2  // 과녁수 지정
#define STANDTERM 3000  // 과녁 기립쿨타임 
#define DEBOUNCETERM 200  // 센서 디바운스 텀
using namespace std;

/*  명령어 리스트
    (명령어) [단축명령어]    : (설명)
    gamemode0 [ ga ]         : 조율모드로 들어간다 ( 제한시간 없음, 점수 카운트 안함 )
    gamemode1 [ gamemode1 ]  : 게임모드1로 들어간다 ( 제한시간 있음, 점수 카운트 함 )
    gamestop [ ga ]          : 게임 강제중지 시킨다 ( 게임을 강제로 종료함 )
    standall [ s ]           : 모든 과녁을 일으킨다
    downall [ d ]            : 모든 과녁을 눕힌다
    showallinfo [ show ]     : 모든 과녁의 정보를 보여준다
    monitor [ mon ]          : 명령어 입력후 원하는 과녁의 번호를 입력하면, 해당 과녁에 부착된 센서값을 실시간으로 보여준다
    manual [ ma ]            : 명령어 입력후 원하는 과녁의 번호를 입력하면, 해당 과녁을 강제로 일으킨다
    cls [ cls ]              : 시리얼 모니터창을 비워(?)준다
*/

const String ADDTARGET = "addTarget";
const String DELETETARGET = "deleteTarget";
const String GAMEMODE0 = "gamemode0";
const String GAMEMODE1 = "gamemode1";
const String GAMESTOP = "gamestop";
const String STANDALL = "standall";
const String DOWNALL = "downall";
const String SHOWALLINFO = "showallinfo";
const String MONITOR = "monitor";
const String CLEAR = "cls";
const String MANUAL = "manual";
const int timeLimit = 20;  // 제한시간(초)
const int bPin = A15;
const int mgPin[] = {2, 3, 4};
const int mwPin[] = {7, 8, 9};
const int sPin[] = {22, 24, 26};
const int lPin[] = {52, 53, 54};
const int score[] = {10, 30, 50};
const int zero[] = {0, 0, 0};
const int stand[] = {90, 90, 90};
const int wait[] = {0, 0, 0};
const int down[] = {30, 30, 30};

///////////////////////////////////////////

boolean motorTrig[NUM_T] = {false, };
boolean reverse[NUM_T] = {false, };
unsigned int standPoint[NUM_T] = {0, };
void motorThread();

///////////////////////////////////////////

boolean timeOverChecker = false;
int progress = 0;
void gameTimerThread();

///////////////////////////////////////////

typedef class transmisson {  // 소프트웨어 시리얼과 하드웨어 시리얼을 통합하는 클래스
    private:
        String * buf;
        SoftwareSerial * sender;
    public:
        transmisson() {
            sender = NULL;
        }
        transmisson(SoftwareSerial * serial) {
            sender = serial;
            sender->begin(115200);
            sender->setTimeout(100);
        }
        // void sendProcess();  // 매개변수를 통해 보내온 데이터는 전송 블가했었음(아마 미리 지정해둔거는 안돼는듯.)
        void clearMonitor();
        String getCommend();
        bool recieveProcess();
} transmisson;

bool transmisson::recieveProcess() {
    if (Serial.available()) {
        String recieve = Serial.readStringUntil('\r');

        buf = new String;
        *(buf) = recieve;
        return true;
    }
    if (sender != NULL) {
        if (sender->available()) {
            String recieve = sender->readStringUntil('\r');

            buf = new String;
            *(buf) = recieve;
            return true;
        }
    }
    return false;
}
/*
void transmisson::sendProcess() {
    if (sender == NULL) {
        Serial.println("No sender!");
    }
    else {
        sender->print(*buf);
        Serial.println("send complete!");
    }
}
*/

void transmisson::clearMonitor() {
    for (int i = 0; i < 16; i++) {
        Serial.println();
    }
}

String transmisson::getCommend() {
    return *(buf);
}

////////////////////////////////////////////////////////////////

SimpleTimer timer;  // 쓰레드 구동 주체
SoftwareSerial BT(6, 7);
transmisson serial(&BT);

////////////////////////////////////////////////////////////////

typedef class target {
    private:
        int mgPin;  // 해당 과녁에 구성된 과녁 기립제어 모터 핀
        int mwPin;  // 해당 과녁에 구성된 과녁 기립제어 모터 핀
        int sPin;   // 해당 과녁에 구성된 센서 핀
        int lPin;   // 해당 과녁에 구성된 LED 핀
        int score;  // 해당 과녁의 점수
        int zero;   // 과녁 영점계수 (원점계수)
        int stand;  // 과녁 영점계수 (기립계수)
        int wait;   // 과녁 영점계수 (대기계수)
        int down;   // 과녁 영점계수 (강제다운계수)
        unsigned int hitTime;  // 피격시점 저장 변수
        bool stat;  // 과녁 상태
        Servo motor_gnd;   // Ground Motor
        Servo motor_wall;  // Wall Motor
    public:
        target() {
            mgPin = 0;
            mwPin = 0;
            sPin = 0;
            score = 0;
            zero = 0;
            stand = 0;
            stat = true;
        }
        // 요구 인자 : 기립제어모터(직립)포트, 기립제어모터포트, 인식센서 포트, 표시led 포트, 과녁점수, 원점계수, 기립계수, 대기계수, 강제다운계수
        target(int p_mg, int p_mw, int p_s, int p_l, int num_score, int num_zero, int num_stand, int num_wait, int num_down) {
            mgPin = p_mg;
            mwPin = p_mw;
            sPin =  p_s;
            lPin = p_l;
            score = num_score;
            zero = num_zero;
            stand = num_stand;
            wait = num_wait;
            down = num_down;
            stat = true;
            pinMode(sPin, INPUT);
            pinMode(lPin, OUTPUT);
            motor_gnd.attach(mgPin);
            motor_gnd.write(zero);
            motor_wall.attach(mwPin);
            motor_wall.write(wait);
        }
        void showInfo();            // 과녁 하드웨어 정보 표시
        void activateMotor_stand(); // 과녁대 기립
        void activateMotor_zero();  // 모터 정위치
        void activateMotor_down();  // 과녁대 강제 다운
        void activateMotor_wait();  // 모터 정위치
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

void target::showInfo() {
    if (mgPin > 13) {
        Serial.print("Attached motor port : A"); Serial.println(mgPin - 14);
    }
    else {
        Serial.print("Attached motor port : "); Serial.println(mgPin);
    }
    Serial.print("Attached sensor port : "); Serial.println(sPin);
    Serial.print("Registered score : "); Serial.println(score);
}

void target::activateMotor_stand() {
    motor_gnd.write(stand);
}

void target::activateMotor_zero() {
    motor_gnd.write(zero);
}

void target::activateMotor_down() {
    motor_wall.write(down);
}

void target::activateMotor_wait() {
    motor_wall.write(wait);
}

void target::ledOn() {
    digitalWrite(lPin, HIGH);
}

void target::ledOff() {
    digitalWrite(lPin, LOW);
}

void target::disableMotor() {
    motor_gnd.detach();
    motor_wall.detach();
}

void target::enableMotor() {
    motor_gnd.attach(mgPin);
    motor_wall.attach(mwPin);
}

void target::updateStatus() {
    stat = getSensorValue();
}

void target::recordHitTime() {
    hitTime = millis();
}

int target::getHitTime() {
    return hitTime;
}

int target::getScore() {
    return score;
}

int target::getSensorValue() {
    return digitalRead(sPin);
}

bool target::getStat() {
    return stat;
}

///////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////

typedef class gameHandler {
    private:
        target * objArr[10];  // 과녁객체
        int index;            // 과녁 색인
        int timerID[10];      // 쓰레드 함수 색인
        int buzzer;
    public:
        gameHandler() {
            for (int i = 0; i < 10; i++) {
                objArr[i] = NULL;
            }
            buzzer = bPin;
            pinMode(buzzer, OUTPUT);
            timerID[0] = timer.setInterval(50, motorThread); 
            timerID[1] = timer.setInterval(1000, gameTimerThread);
        }
        void showAllInfo();
        void statusMonitor(int index);
        void addTarget();
        void addTarget(int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8, int n9);
        void delTarget();
        void gamemode0_new();
        void gamemode1_new();
        void gamemode2();
        void disableMotorAll();
        void enableMotorAll();
        void activateMotorAll();
        void downAll();
        void buzzerOn();
        void buzzerOff();
        bool checkTimeOver();
        bool checkAllHit();
        bool checkStopCommend();
        bool checkManualControl();
        target * getObjAddress(int i);
} gameHandler;

void gameHandler::showAllInfo() {
    for (int i = 0; i < index; i++) {
        objArr[i]->showInfo();
    }
}

void gameHandler::statusMonitor(int index) {
    for (;;) {
        if (checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인
            break;
        }
        
        serial.clearMonitor();
        Serial.println("---과녁 계기판---");
        Serial.print("센서값 : "); Serial.println(objArr[index - 1]->getSensorValue());
        Serial.print("(VAL1) : "); Serial.println(1023);
        Serial.print("(VAL2) : "); Serial.println(1023);
        delay(250);
    }
}

void gameHandler::addTarget() {
    objArr[index] = new target(mgPin[index], mwPin[index], sPin[index], lPin[index], score[index], zero[index], stand[index], wait[index], down[index]);
    index++;
}

void gameHandler::addTarget(int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8, int n9) {
    objArr[index] = new target(n1, n2, n3, n4, n5, n6, n7, n8, n9);
    index++;
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

void gameHandler::gamemode0_new() {
    boolean entryTrig[NUM_T] = {false, };
    boolean lastState[NUM_T] = {false, };
    unsigned int debouncePoint[NUM_T] = {0, };
    unsigned int lastStandPoint[NUM_T] = {0, };
    unsigned int c = 0;
    int lastScore = 0;
    int i = 0;

    timer.restartTimer(timerID[0]);  // 모터 쓰레드 초기화 ( 비정상적 구동 방지 )
    timer.disable(timerID[1]);       // 게임 타이머 쓰레드 초기화 ( 비정상적 구동 방지 )
    for (;;) {
        c = millis();

        if (checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인
            break;
        }
        if (checkManualControl() == true) {
            Serial.print("어떤 과녁? : ");

            for (;;) {
                if (Serial.available()) {
                    int recieve = Serial.parseInt();

                    Serial.println(recieve);
                    // motorTrig[recieve - 1] = true;
                    // standPoint[recieve - 1] = c;
                }
            }

        }

        for (i = 0; i < NUM_T; i++) {
            boolean targetState = objArr[i]->getStat();

            if (targetState != lastState[i]) {
                debouncePoint[i] = c;
            }

            if (c - debouncePoint[i] >= DEBOUNCETERM) {             // 디바운스텀 이상
                if (targetState == LOW && entryTrig[i] == false) {  //  피격 상태 그리고 최초 상태 변화 감지 시
                    entryTrig[i] = true;
                    objArr[i]->recordHitTime();          // 피격 시점 저장
                    lastScore += objArr[i]->getScore();  // 점수 계산
                    lastState[i] = targetState;
                    Serial.println("Hit!!");
                }
            }

            if (entryTrig[i] == true) {
                if (c - objArr[i]->getHitTime() >= STANDTERM || c - lastStandPoint[i] >= 5000) {  // 피격 3초 후 또는 저번 피격 으로부터 5초 후 
                    Serial.println("ACT!!");
                    motorTrig[i] = true;     // 모터 쓰레드에 걸린 트리거 활성화
                    standPoint[i] = c;       // 기립 시점 저장( 쓰레드 용 )
                    lastStandPoint[i] = c;   // 저번 기립 시점 저장
                    entryTrig[i] = false;
                    debouncePoint[i] = c;    // 기립 하면서도 생기는 플로탕을 위한 디바운스 
                }
            }

            lastState[i] = targetState;
            objArr[i]->updateStatus();  // 과녁 상태 업데이트
        }
        timer.run();   // 쓰레드 구동
    }
    Serial.print("Last score is "); Serial.println(lastScore);  // 점수 출력
    timer.enable(timerID[1]);
}

void gameHandler::gamemode1_new() {
    boolean entryTrig[NUM_T] = {false, };
    boolean lastState[NUM_T] = {false, };
    unsigned int debouncePoint[NUM_T] = {0, };
    unsigned int lastStandPoint[NUM_T] = {0, };
    unsigned int c = 0;
    int lastScore = 0;
    int i = 0;

    timer.restartTimer(timerID[0]);  // 모터 쓰레드 초기화 ( 비정상적 구동 방지 )
    timer.restartTimer(timerID[1]);  // 게임 타이머 쓰레드 초기화 ( 비정상적 구동 방지 )
    for (;;) {
        c = millis();

        if (checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인
            Serial.println("StopCheck.");
            break;
        }
        if (checkTimeOver() == true) {
            Serial.println("TimeOver!!");
            break;
        }

        for (i = 0; i < NUM_T; i++) {
            boolean targetState = objArr[i]->getStat();

            if (targetState != lastState[i]) {
                debouncePoint[i] = c;
            }

            if (c - debouncePoint[i] >= DEBOUNCETERM) {             // 디바운스텀 이상
                if (targetState == LOW && entryTrig[i] == false) {  // 피격 상태 그리고 최초 상태 변화 감지 시
                    entryTrig[i] = true;
                    objArr[i]->recordHitTime();          // 피격 시점 저장
                    lastScore += objArr[i]->getScore();  // 점수 계산
                    lastState[i] = targetState;
                    Serial.println("Hit!!");
                }
            }

            if (entryTrig[i] == true) {
                if (c - objArr[i]->getHitTime() >= STANDTERM || c - lastStandPoint[i] >= 5000) {  // 피격 3초 후 또는 저번 피격 으로부터 5초 후 
                    Serial.println("ACT!!");
                    motorTrig[i] = true;    // 모터 쓰레드에 걸린 트리거 활성화
                    standPoint[i] = c;      // 기립 시점 저장( 쓰레드 용 )
                    lastStandPoint[i] = c;  // 저번 기립 시점 저장
                    entryTrig[i] = false; 
                    debouncePoint[i] = c;   // 기립 하면서도 생기는 플로탕을 위한 디바운스 
                }
            }

            lastState[i] = targetState;
            objArr[i]->updateStatus();  // 과녁 상태 업데이트
        }
        timer.run();  // 쓰레드 구동
    }
    Serial.print("Last score is "); Serial.println(lastScore);  // 점수 출력
    timeOverChecker = false;
    progress = 0;  // 게임 타이머 초기화
}

/*
void gameHandler::gamemode2() {
    static int i = 0;
    static int randomNumTarget = 0;
    static int randomIndex[NUM_T] = {0, };
    static int randomTerm[NUM_T] = {0, };
    static int result = 0;
    unsigned int c = 0;
    unsigned int p = 0;

    for (;;) {
        if (checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인
            break;
        }
        if (checkTimeOver() == true) {
            break;
        }
        randomNumTarget = random(1, index + 1);
        for (int i = 0; i < randomNumTarget; i++) {
            randomIndex[i] = random(0, index);
            randomTerm[i] = random(250, 500);
        }
        for (int i = 0; i < randomNumTarget; i++) {  // 수정바람
            if (objArr[i] == NULL) {  // 해제된 과녁에 접근할시 생기는 오류 처리
                randomNumTarget--;
                for (int j = 0; j < randomNumTarget - i; j++) {
                    randomIndex[i + j] = randomIndex[i + j + 1];
                }

            }
        }
        for (;;) {
            c = millis();

            if (c - p < 1500) {
                break;
            }
            for (int i = 0; i < randomNumTarget; i++) {
                if (c - p >= randomTerm[i]) {
                    objArr[randomIndex[i]]->standown();
                    randomIndex[i] = 5;  // 모터 뻘짓 방지용 초기화
                }
                else if (objArr[randomIndex[i]]->checking()) {
                    objArr[randomIndex[i]]->standup();
                    result += objArr[randomIndex[i]]->getScore();
                }
            }
        }
    }
    Serial.print("최종점수는 "); Serial.print(result); Serial.println("점!");  // 점수 출력
    result = 0;
}
*/

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
    for (int i = 0; i < index; i++) {
        objArr[i]->activateMotor_stand();
    }
    delay(500);
    for (int i = 0; i < index; i++) {
        objArr[i]->activateMotor_zero();
    }
    delay(500);
}

void gameHandler::downAll() {
    for (int i = 0; i < index; i++) {
        objArr[i]->activateMotor_down();
    }
    delay(500);
    for (int i = 0; i < index; i++) {
        objArr[i]->activateMotor_wait();
    }
    delay(500);
}

void gameHandler::buzzerOn() {
    analogWrite(buzzer, 400);
}

void gameHandler::buzzerOff() {
    analogWrite(buzzer, 0);
}

bool gameHandler::checkTimeOver() {
    if (timeOverChecker == true) {
        return true;
    }
    else {
        return false;
    }
}

bool gameHandler::checkAllHit() {
    static int i = 0;
    static int count = 0;

    for (i = 0; i < index; i++) {
        if (objArr[i]->getStat()) {  // 기립상태 확인
            count++;  // 다운된 과녁수를 계산함
        }
    }
    if (count >= index) {  // 다운된 과녁수가 NUM_T개이상인지 확인
        count = 0;  // 다음 턴을 위해 카운트수를 초기화
        return true;
    }
    else {
        return false;
    }
}

bool gameHandler::checkStopCommend() {
    if (serial.recieveProcess()) {
        String recieve = serial.getCommend();

        if (!recieve.compareTo(GAMESTOP)) {
            return true;
        }
    }
    else {  // 이 함수는 명령이 수신됐는지 한번만 확인하면 돼므로 한번만 확인하고 그냥 넘어감
        return false;
    }
    return false;
}

bool gameHandler::checkManualControl() {
    if (serial.recieveProcess()) {
        String recieve = serial.getCommend();

        if (!recieve.compareTo(MANUAL)) {
            return true;
        }
    }
    else {  // 이 함수는 명령이 수신됐는지 한번만 확인하면 돼므로 한번만 확인하고 그냥 넘어감
        return false;
    }
}

target * gameHandler::getObjAddress(int i) {
    return objArr[i];
}

////////////////////////////////////////////////////////////////

gameHandler handler;

////////////////////////////////////////////////////////////////

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(100);

    for (int i = 0; i < NUM_T; i++) {
        handler.addTarget();
    }
    handler.disableMotorAll();
    handler.showAllInfo();
}

void loop() {
    static int i = 0;
    String recieve;

    Serial.print("명령 대기중...(명령어를 쓰세요) : ");
    for (;;) {
        if (serial.recieveProcess()) {
            String recieve = serial.getCommend();

            Serial.println(recieve);
            handler.enableMotorAll(); // 모든 모터 등록 (절전모드해제)
            if (!recieve.compareTo(GAMEMODE0)) {
                Serial.println("과녁 조율 모드");
                handler.gamemode0_new();
            }
            else if (!recieve.compareTo(GAMEMODE1)) {
                Serial.println("게임모드1");
                handler.gamemode1_new();
            }
            else if (!recieve.compareTo(STANDALL)) {
                Serial.println("모든 과녁 스탠바이");
                handler.activateMotorAll();
            }
            else if (!recieve.compareTo(DOWNALL)) {
                Serial.println("모든 과녁 스탠바이");
                // handler.downAll();
            }
            else if (!recieve.compareTo(SHOWALLINFO)) {
                Serial.println("모든 과녁 정보");
                handler.showAllInfo();
            }
            else if (!recieve.compareTo(DELETETARGET)) {
                Serial.print("어떤 과녁? : ");
                // handler.delTarget();
            }
            else if (!recieve.compareTo(ADDTARGET)) {
                // handler.addTarget();
            }
            else if (!recieve.compareTo(CLEAR)) {
                serial.clearMonitor();
            }
            else if (!recieve.compareTo(MONITOR)) {
                String temp;

                Serial.print("어떤 과녁? : ");
                for (;;) {
                    if (Serial.available()) {
                        String temp = Serial.readStringUntil('\r');

                        handler.statusMonitor(temp.toInt());
                        break;
                    }
                }
            }
            else {
                Serial.println("리스트에 없는 명령어 입니다");
            }
            handler.disableMotorAll();  // 모든 모터 등록 해제(절전모드)
            break;
        }
    }
    delay(100);
}

void motorThread() {
    unsigned int c = millis();

    for (int i = 0; i < NUM_T; i++) {
        if (motorTrig[i] == true) {
            handler.getObjAddress(i)->enableMotor();  // 모터 활성화

            if (reverse[i] == true) {  // 해당 인덱스 모터 작동 방향
                handler.getObjAddress(i)->activateMotor_zero();  // 모터 정위치
            }
            else {
                handler.getObjAddress(i)->activateMotor_stand();  // 과녁대 기립
            }

            if (c - standPoint[i] >= 250) {  // 모터 활성화 시간이 0.25초 이상
                if (reverse[i] == true) {                      // 방향이 참(정위치)
                    motorTrig[i] = false;                      // 모터 트리거 비활성화
                    reverse[i] = false;                        // 방향 초기화
                    handler.getObjAddress(i)->disableMotor();  // 모터 비활성화
                }
                else {
                    reverse[i] = true;  // 모터 방향 정위치로
                    standPoint[i] = c;  // 기립 시점 저장
                }
            }
        }
    }
}

void gameTimerThread() {
    if (progress >= timeLimit) {
        timeOverChecker = true;
    }
    Serial.print("남은 시간은 "); Serial.print(timeLimit - progress); Serial.println("초.");
    progress++;
}
