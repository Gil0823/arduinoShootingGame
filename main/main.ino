#include <Target.h>
#include <GameHandler.h>
#define NUM_T 1

/*  명령어 리스트
    (명령어) [단축명령어]    : (설명)
    gamemode0 [ gm0 ]         : 조율모드로 들어간다 ( 제한시간 없음, 점수 카운트 안함 )
    gamemode1 [ gm1 ]         : 게임모드1로 들어간다 ( 제한시간 있음, 점수 카운트 함 )
    stop [  ]                             : 게임 강제중지 시킨다 ( 게임을 강제로 종료함 )
    standall [  ]                        : 모든 과녁을 일으킨다
    downall [  ]                        : 모든 과녁을 눕힌다
    showallinfo [  ]                   : 모든 과녁의 정보를 보여준다
    monitor [ mon ]                         : 명령어 입력후 원하는 과녁의 번호를 입력하면, 해당 과녁에 부착된 센서값을 실시간으로 보여준다
    manual [  ]                         : 명령어 입력후 원하는 과녁의 번호를 입력하면, 해당 과녁을 강제로 일으킨다
    clipOut[   ]                         : 현재 하드웨어 정보를 코드서식대로 출력한다.
*/

//////////////////////////////////////////////////////////////// ( 하드웨어 정보 )

const int bPin = A0;
const int gmPin[] = {3, 5, 6};
const int cmPin[] = {9, 10, 11};
const int sPin[] = {A0, A1, A2};
const int lPin[] = {52, 53, 54};
const int score[] = {50, 100, 50};
const int gm_zeroList[] = {0, 0, 0};
const int gm_actList[] = {100, 100, 100};
const int cm_zeroList[] = {0, 0, 0};
const int cm_actList[] = {30, 30, 30};

////////////////////////////////////////////////////////////////

void setHWInfo();

////////////////////////////////////////////////////////////////

gameHandler handler;

////////////////////////////////////////////////////////////////

void setup() {
    Serial.begin(115200);
    Serial.setTimeout(100);

    setHWInfo();
    handler.disableMotorAll();
    handler.showAllInfo();
}

void loop() {
    Serial.print("명령 대기중...(명령어를 쓰세요) : ");
    for (;;) {
        if (Serial.available()) {
            String recieve = Serial.readStringUntil('\r'); Serial.println(recieve);
            int buf = 0;

            if(!recieve.compareTo("help")) {
                Serial.println("(명령어) [단축명령어])");
                Serial.println("gamemode0 [ gm0 ]");
                Serial.println("gamemode1 [ gm1 ]");
                Serial.println("standAll [  ]");
                Serial.println("downAll [ gm0 ]");
                Serial.println("showAllInfo [  ]");
                Serial.println("monitor [ mon ]");
                Serial.println("stop [  ]");
                Serial.println("manual [ man ]");
                Serial.println("clipOut [  ]");
            }
            else if (!recieve.compareTo("addTarget")) {
                Serial.println("addTarget OK.");
                handler.addTarget();
            }
            else if (!recieve.compareTo("delTarget")) {
                Serial.println("delTarget OK.");
                // 코드 수정 중...
            }
            else if (!recieve.compareTo("gamemode0") || !recieve.compareTo("gm0")) {
                Serial.println("gamemode0 OK.");
                handler.gamemode0();
            }
            else if (!recieve.compareTo("gamemode1") || !recieve.compareTo("gm1")) {
                Serial.println("gamemode1 OK.");
                handler.gamemode1();
            }
            else if (!recieve.compareTo("standAll")) {
                Serial.println("standAll OK.");
                handler.activateMotorAll(); 
            }
            else if (!recieve.compareTo("downAll")) {
                Serial.println("downAll OK.");
                // 현재 모터 아직 이식 안함.
            }
            else if (!recieve.compareTo("showAllInfo") || !recieve.compareTo("showallinfo")) {
                Serial.println("showAllInfo OK.");
                handler.showAllInfo();
            }
            else if (!recieve.compareTo("monitor") || !recieve.compareTo("mon")) {
                Serial.println("monitor OK.");
                handler.statusMonitor();
            }
            else if (!recieve.compareTo("clipOut")) {
                Serial.println("clipOut OK.");
                handler.clipOut();
            }
            else if(sscanf(recieve.c_str(), "set standTerm %d", &buf) == 1) {
                Serial.println("set Term OK.");
                handler.setStandTerm(buf);
            }
            else {
                Serial.println("ERROR OK.");
            }
            handler.disableMotorAll();
            break;
        }
    }
}

void setHWInfo() {
    for (int i = 0; i < NUM_T; i++) {
        handler.addTarget(gmPin[i], cmPin[i], sPin[i], lPin[i], score[i], gm_zeroList[i], gm_actList[i], cm_zeroList[i], cm_actList[i]);
    }
}
