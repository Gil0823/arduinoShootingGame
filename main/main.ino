#include <SoftwareSerial.h>
#include <Servo.h>              
#define NUM_T 3  // 과녁수 지정
#define STANDTERM 3000  // 과녁 기립쿨타임 
using namespace std;

/* 명령어 리스트 
 * (명령어) [단축명령어]    : (설명)
 * gamemode0 [ ga ]         : 조율모드로 들어간다 ( 제한시간 없음, 점수 카운트 안함 )
 * gamemode1 [ gamemode1 ]  : 게임모드1로 들어간다 ( 제한시간 있음, 점수 카운트 함 )
 * gamestop [ ga ]          : 게임 강제중지 시킨다 ( 게임을 강제로 종료함 )
 * standall [ s ]           : 모든 과녁을 일으킨다
 * downall [ d ]            : 모든 과녁을 눕힌다
 * showallinfo [ show ]     : 모든 과녁의 정보를 보여준다
 * monitor [ mon ]          : 명령어 입력후 원하는 과녁의 번호를 입력하면, 해당 과녁에 부착된 센서값을 실시간으로 보여준다
 * manual [ ma ]            : 명령어 입력후 원하는 과녁의 번호를 입력하면, 해당 과녁을 강제로 일으킨다
 * cls [ cls ]              : 시리얼 모니터창을 비워(?)준다
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
const int timeLimit = 10;  // 제한시간(초)
const int bPin = A4;
const int mgPins[] = {3, 5, 6};
const int mwPins[] = {9, 10, 11};
const int sPins[] = {A2, A1, A0};
const int lPins[] = {4, 7, 8};
const int scores[] = {10, 30, 50};
const int zeros[] = {0, 0, 0};
const int stands[] = {105, 150, 105};
const int waits[] = {0, 10, 10};
const int downs[] = {30, 90, 30};

typedef class transmisson {
    private:
        String * buf;
        bool recieveCheck;
        int check;
        int count;
        SoftwareSerial * sender;
    public:
        transmisson() {
            sender = NULL;
            recieveCheck = false;
            check = 0;
            count = 0;
        }
        transmisson(SoftwareSerial * serial) {
            sender = serial;
            recieveCheck = false;
            check = 0;
            count = 0;
            sender->begin(9600);
        }
        void clearBuf();
        void clearMonitor();
        String getCommend();
        bool process_hardware();
        bool process_software();
        bool recieveProcess_waitting();
        bool recieveProcess_once();
        bool compare(String parm, String tar);
} transmisson;

bool transmisson::process_hardware() {
    String buff;

    for(;;) {
        if(Serial.available()) {
            char c = Serial.read();

            this->recieveCheck = true;
            buff += c;
            this->check++;
            delay(10);  // Serial이 씹힘
        }
        if(this->recieveCheck) {
            this->count++;
            if(this->check != this->count) {
                this->buf = new String;
            
                *(this->buf) = buff;
                this->recieveCheck = false;
                this->count = 0;
                Serial.println(*(this->buf));
                return true;
            }
        } 
        else if(!(this->recieveCheck)) {
            return false;
        }
    }
}

bool transmisson::process_software() {
    String buff;

    for(;;) {
        if(sender->available()) {
            char c = sender->read();

            this->recieveCheck = true;
            buff += c;
            this->check++;
            delay(10);  // Serial이 씹힘
        }
        if(this->recieveCheck) {
            this->count++;
            if(this->check != this->count) {
                this->buf = new String;
            
                *(this->buf) = buff;
                this->recieveCheck = false;
                this->check -= 2;  // 블루투스 통신시에 생기는 쓰레기 버퍼 수는 제외
                this->count = 0;
                Serial.println(*(this->buf));
                return true;
            }
        } 
        else if(!(this->recieveCheck)) {
            return false;
        }
    }
}

bool transmisson::recieveProcess_waitting() {  // 명령 대기 (무한대기 버전)
    for(this->check = 0, this->count = 0;;) {
        if(sender == NULL) {
            if(process_hardware()) {
                return true;
            }
        }
        else {
            if(process_software()) {
                return true;
            }
        }
    }
    return false;
}

bool transmisson::recieveProcess_once() {  // 명령 대기 (확인버전)(스레드점유X)
    this->check = 0;
  
    if(sender == NULL) {
        if(Serial.available()) {
            if(process_hardware()) {
                return true;
            }
        }
    }
    else {
        if(sender->available()) {
            if(process_software()) {
                return true;
            }
        }
    }
    return false;
}

void transmisson::clearBuf() {
    delete this->buf;
}

void transmisson::clearMonitor() {
    for(int i = 0; i < 16; i++) {
        Serial.println();
    }
}

String transmisson::getCommend() {
    return *(this->buf);
}

bool transmisson::compare(String parm, String tar) {
    for(int i = 0; i < this->check; i++) {
        if(parm[i] != tar[i]) {
            return false;
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////

SoftwareSerial BT(12, 13);
transmisson serial(&BT);
//transmisson serial;

////////////////////////////////////////////////////////////////

typedef class target {
  private:
      int mgPin;  // 해당 과녁에 구성된 과녁 기립제어 모터 핀 
      int mwPin;  // 해당 과녁에 구성된 과녁 ? 모터 핀
      int sPin;   // 해당 과녁에 구성된 센서 핀 
      int lPin; 
      int score;  // 해당 과녁의 점수
      int zero;  // 과녁 영점계수 (다운상태) 
      int stand;  // 과녁 영점계수 (업상태)  
      int wait;  // 과녁 영점계수 (대기상태) 
      int down;  // 과녁 영점계수 (다운상태)  
      unsigned int downTime;  // 피격시점 시간저장 변수  
      bool stat;  // 과녁 상태  
      Servo moter_gnd;
      Servo moter_wall;
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
// 요구 인자 : 기립제어모터(직립)포트, 기립제어모터포트, 인식센서 포트, 표시led 포트, 과녁점수, 기립(초기)계수, 기립(활성화)계수, 기립(초기)계수, 기립(활성화)계수
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
          moter_gnd.attach(mgPin);
          moter_gnd.write(zero);
          moter_wall.attach(mwPin);
          moter_wall.write(wait);
      }
      void showInfo();
      void standup();
      void standown();
      void moterActivate_stand();
      void moterActivate_zero();
      void moterActivate_down();
      void moterActivate_wait();
      void ledOn();
      void ledOff();
      void moterSleep();
      void moterWake();
      void updateStatus();
      void recordDownTime();  // 피격 시점 기억  
      int getDownTime();
      int getScore();
      int getSenserValue();
      bool getStat();
      bool checking();  // 센서 감지 여부 
} target;

void target::showInfo() {
    if(this->mgPin > 13) {
        Serial.print("Attached moter port : A"); Serial.println((this->mgPin)-13);
    }
    else {
        Serial.print("Attached moter port : "); Serial.println(this->mgPin);
    }
    Serial.print("Attached senser port : "); Serial.println(this->sPin);
    Serial.print("Registered score : "); Serial.println(this->score);
}

void target::standup() {
    this->moter_gnd.write(this->stand);
    delay(300);
    this->moter_gnd.write(this->zero);
    delay(250);
} 

void target::standown() {
    this->moter_wall.write(this->down);
    delay(500);
    this->moter_wall.write(this->wait);
    delay(500);
}

void target::moterActivate_stand() {
    this->moter_gnd.write(this->stand);
}

void target::moterActivate_zero() {
    this->moter_gnd.write(this->zero);
}

void target::moterActivate_down() {
    this->moter_wall.write(this->down);
}

void target::moterActivate_wait() {
    this->moter_wall.write(this->wait);
}

void target::ledOn() {
	  digitalWrite(lPin, HIGH);
}

void target::ledOff() {
	  digitalWrite(lPin, LOW);
}

void target::moterSleep() {
    this->moter_gnd.detach();
    this->moter_wall.detach();
}

void target::moterWake() {
    this->moter_gnd.attach(this->mgPin);
    this->moter_wall.attach(this->mwPin);
}

void target::updateStatus() {
    if(checking()) {
        this->stat = false;
    }
    else {
        this->stat = true;
    }
}

void target::recordDownTime() {
    this->downTime = millis();
}

int target::getDownTime() {
    return this->downTime;
}

int target::getScore() {
    return this->score;
}

int target::getSenserValue() {
    return analogRead(this->sPin);
}

bool target::getStat() {
    return this->stat;
}

bool target::checking() {
    if(digitalRead(this->sPin) == LOW) {  // 과녁 피격 센서 조도센서로 변경함으로인한 인식 코드부 임시변경
        return true;
    }
    else {
        return false;
    }
}

///////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////

typedef class gameHandler {
    private:
      target * objArr[10];  // 과녁객체
      unsigned int startTime;
      int index;  // 과녁 색인  
      int busser; 
    public:
      gameHandler() {
          for(int i = 0; i < 10; i++) {
              objArr[i] = NULL;
          }
          this->busser = bPin;
          pinMode(this->busser, OUTPUT);
      }
      void showAllInfo();
      void statusMonitor(int index);
      void addTarget();
      void addTarget(int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8, int n9);
      void delTarget();
      void gamemode0();
      void gamemode1();
      void gamemode2();
      void moterAllSleep();
      void moterAllWake();
      void standAll();
      void downAll();
      void busserOn();
      void busserOff();
      void manualMoterControl(int index);
      bool checkDownTerm();
      bool checkTimeOver();
      bool checkAllDown();
      bool checkStopCommend();
      bool checkManualControl();
} gameHandler;

void gameHandler::showAllInfo() {
    for(int i = 0; i < index; i++) {
        objArr[i]->showInfo();
    }
}

void gameHandler::statusMonitor(int index) {
    for(;;) {
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
            break;
        }
        serial.clearMonitor();
        Serial.println("---과녁 계기판---");
        Serial.print("센서값 : "); Serial.println(objArr[index-1]->getSenserValue());
        Serial.print("(VAL1) : "); Serial.println(1023);
        Serial.print("(VAL2) : "); Serial.println(1023);
        delay(250);
    }    
}

void gameHandler::addTarget() {
    objArr[this->index] = new target(mgPins[index], mwPins[index], sPins[index], lPins[index], scores[index], zeros[index], stands[index], waits[index], downs[index]);
    this->index++;
}

void gameHandler::addTarget(int n1, int n2, int n3, int n4, int n5, int n6, int n7, int n8, int n9) {
    objArr[this->index] = new target(n1, n2, n3, n4, n5, n6, n7, n8, n9);
    this->index++;
}

void gameHandler::delTarget() {
	  int target = 0;
	  int target_index = 0;
	  String recieve;
	
	  if(serial.recieveProcess_waitting()) {  // 이 부분 수정 바람
		    recieve = serial.getCommend();
		
		    target = recieve.toInt(); 
		    target_index = target - 1;  // 실제 인덱스 넘버
	  }
	  delete objArr[target_index];
	  this->index--;
	
	  for(int i = 0; i < this->index - target; i++) {
		    objArr[target_index+i] = objArr[target_index+i+1];
	  }
	  objArr[this->index-1] = NULL;  // 스왑 마지막 줄에 있던 공간을 비워 오류 제거
}

void gameHandler::gamemode0() {
    static int i = 0;
    static unsigned int progress = 0;
    static unsigned int p = 0;
    for(;;) {
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
            break;
        }
        if(checkManualControl() == true) {
            if(serial.recieveProcess_waitting()) {
                String recieve;

                manualMoterControl(recieve.toInt());
            }
        }
        for(i = 0; i < this->index; i++) {
            progress = millis(); 
            if(objArr[i] == NULL) {  // 해제된 과녁에 접근할시 생기는 오류 처리
                break;
            }
            if(objArr[i]->checking()) {  // 피격여부 확인  
                if(objArr[i]->getStat()) {
                    Serial.print(i+1); Serial.println("번째 피격해서");
                    Serial.print(scores[i]); Serial.println("점 흭득!");
                    busserOn();
                    objArr[i]->ledOn();
                }
                if((progress - objArr[i]->getDownTime()) >= STANDTERM) {  // 2018.07.29 게임진행 최적화 완료
                    objArr[i]->ledOff();
                    objArr[i]->recordDownTime();
                    objArr[i]->standup();
                }
                if((progress - p) >= 50) {
                  busserOff();
                  p = progress;
                }
            }
            objArr[i]->updateStatus();
        }
    }
}

void gameHandler::gamemode1() {
    static int i = 0;
    static int result = 0;
    static unsigned int progress = 0;
    this->startTime = millis();
    
    for(;;) {
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
            break;
        }
        if(checkTimeOver() == true) {
            break;
        }
        for(i = 0; i < this->index; i++) {
            progress = millis();

            if(objArr[i] == NULL) {  // 해제된 과녁에 접근할시 생기는 오류 처리
                break;
            }
            if(objArr[i]->checking()) {  // 피격여부 확인  
                if(objArr[i]->getStat()) {
                    Serial.print(i+1); Serial.println("번째 피격해서");
                    Serial.print(scores[i]); Serial.println("점 흭득!");
                    objArr[i]->updateStatus();
                    result += objArr[i]->getScore();
                }
                if((progress - objArr[i]->getDownTime()) >= STANDTERM) {
                  objArr[i]->standup() ;
                  objArr[i]->recordDownTime();
                  objArr[i]->updateStatus();
                }
            }
        }
    }
    Serial.print("최종점수는 "); Serial.print(result); Serial.println("점!");
    result = 0;
}

void gameHandler::gamemode2() {
    static int i = 0;
    static int randomNumTarget = 0;
    static int randomIndex[NUM_T] = {0, };
    static int randomTerm[NUM_T] = {0, };
    static int result = 0;
    unsigned int c = 0;
    unsigned int p = 0;
    
    for(;;) {
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
           break;
        }
        if(checkTimeOver() == true) {
            break;
        }
        randomNumTarget = random(1, index+1);
        for(int i = 0; i < randomNumTarget; i++) {
            randomIndex[i] = random(0, index);
            randomTerm[i] = random(250, 500);
        }
        for(int i = 0; i < randomNumTarget; i++) {  // 수정바람
            if(objArr[i] == NULL) {  // 해제된 과녁에 접근할시 생기는 오류 처리
              randomNumTarget--;
                for(int j = 0; j < randomNumTarget - i; j++) {
                    randomIndex[i+j] = randomIndex[i+j+1];
                }
                
            }
        }
        for(;;) {
            c = millis();
      
            if(c - p < 1500) {
                break;
            }
            for(int i = 0; i < randomNumTarget; i++) {
                if(c - p >= randomTerm[i]) {
                    objArr[randomIndex[i]]->standown();
                    randomIndex[i] = 5;  // 모터 뻘짓 방지용 초기화  
                }
                else if(objArr[randomIndex[i]]->checking()) {
                    objArr[randomIndex[i]]->standup();
                    result += objArr[randomIndex[i]]->getScore();
                }
            }
        }
    }
    Serial.print("최종점수는 "); Serial.print(result); Serial.println("점!");
    result = 0;
}

void gameHandler::moterAllSleep() {
    for(int i = 0; i < index; i++) {
        objArr[i]->moterSleep();
    }
}

void gameHandler::moterAllWake() {
    for(int i = 0; i < index; i++) {
        objArr[i]->moterWake();
    }
}

void gameHandler::standAll() {
    for(int i = 0; i < index; i++) {
        objArr[i]->moterActivate_stand();
    }
    delay(500);
    for(int i = 0; i < index; i++) {
        objArr[i]->moterActivate_zero();
    }
    delay(500);
    /*for(int i = 0; i < index; i++) {
        objArr[i]->standup();
    }*/
}

void gameHandler::downAll() {
    for(int i = 0; i < index; i++) {
        objArr[i]->moterActivate_down();
    }
    delay(500);
    for(int i = 0; i < index; i++) {
        objArr[i]->moterActivate_wait();
    }
    delay(500);
}

void gameHandler::busserOn() {
	  analogWrite(this->busser, 250);
}

void gameHandler::busserOff() {
	  analogWrite(this->busser, 0);
}

void gameHandler::manualMoterControl(int index) {
    objArr[index-1]->standup();
}

bool gameHandler::checkDownTerm() {
  
}

bool gameHandler::checkTimeOver() {
    static unsigned int c = 0;
    static unsigned int p = 0;
    static int count = timeLimit;

    c = millis();
    if(c - p >= 1000) {
        Serial.print("남은 시간은 "); Serial.print(count); Serial.println("초.");
        count--;
        p = c;
    }
    if(c - (this->startTime) >= timeLimit*1000) {
        return true;
    }
    else {
        return false;
    }
}

bool gameHandler::checkAllDown() {
    static int i = 0;
    static int count = 0;
  
    for(i = 0; i < index; i++) {
        if(objArr[i]->getStat()) {  // 기립상태 확인 
            count++;  // 다운된 과녁수를 계산함 
        }   
    }
    if(count >= index) {  // 다운된 과녁수가 NUM_T개이상인지 확인 ( 오류로 NUM_T개가 넘을시를 위해 조건을 이상으로 설정함 ) 
      count = 0;  // 다음 턴을 위해 카운트수를 초기화  
      return true;
    }
    else {
      return false;
    }
}

bool gameHandler::checkStopCommend() {
    String recieve;
    
    if(serial.recieveProcess_once()) {
        recieve = serial.getCommend();

        if(serial.compare(recieve, GAMESTOP)) {
            return true;
        }
    }
    else {  // 이 함수는 명령이 수신됐는지 한번만 확인하면 돼므로 한번만 확인하고 그냥 넘어감 
        return false;
    }
}

bool gameHandler::checkManualControl() {
    String recieve;
    
    if(serial.recieveProcess_once()) {
        recieve = serial.getCommend();

        if(serial.compare(recieve, MANUAL)) {
            return true;
        }
    }
    else {  // 이 함수는 명령이 수신됐는지 한번만 확인하면 돼므로 한번만 확인하고 그냥 넘어감 
        return false;
    }
}

////////////////////////////////////////////////////////////////

gameHandler handler;

////////////////////////////////////////////////////////////////

void setup() {
    Serial.begin(9600);
    
    for(int i = 0; i < NUM_T; i++) {
        handler.addTarget();
    }
    handler.moterAllSleep();
    handler.showAllInfo();
    for(int i = 0; i < 3; i++) {
      handler.busserOn();
      delay(100);
      handler.busserOff();
      delay(100);
    }
    /*handler.moterAllWake();
    handler.gamemode0();*/
}

void loop() {
    static int i = 0;
    String recieve;
	
    Serial.print("명령 대기중...(명령어를 쓰세요) : "); 
    if(serial.recieveProcess_waitting()) {
        recieve = serial.getCommend();
        
        handler.moterAllWake(); // 모든 모터 등록 (절전모드해제)
        if(serial.compare(recieve, GAMEMODE0)) {
            Serial.println("과녁 조율 모드");
            handler.gamemode0();
        }
        else if(serial.compare(recieve, GAMEMODE1)) {
            Serial.println("게임모드1");
            handler.gamemode1();
        }
        else if(serial.compare(recieve, STANDALL)) {
            Serial.println("모든 과녁 스탠바이");
            handler.standAll();
        }
        else if(serial.compare(recieve, DOWNALL)) {
            Serial.println("모든 과녁 스탠바이");
            handler.downAll();
        }
        else if(serial.compare(recieve, SHOWALLINFO)) {
            Serial.println("모든 과녁 정보");
            handler.showAllInfo();
        }
        else if(serial.compare(recieve, DELETETARGET)) {
            Serial.print("어떤 과녁? : ");
            handler.delTarget();
        }
        else if(serial.compare(recieve, ADDTARGET)) {
            handler.addTarget();
        }
        else if(serial.compare(recieve, CLEAR)) {
            serial.clearMonitor();
        }
        else if(serial.compare(recieve, MONITOR)) {
            String tmp;

            Serial.print("어떤 과녁? : ");
            if(serial.recieveProcess_waitting()) {
                tmp = serial.getCommend();

                handler.statusMonitor(tmp.toInt());
            }
        }
        else {
            Serial.println("리스트에 없는 명령어 입니다");
        }
        handler.moterAllSleep();  // 모든 모터 등록 해제(절전모드)
        serial.clearBuf();
    }
    delay(100);
}

