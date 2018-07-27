#include <SoftwareSerial.h>
#include <Servo.h>              
#define NUM_T 1  // 과녁수 지정
using namespace std;

const String GAMEMODE0 = "gamemode0";
const String GAMEMODE1 = "gamemode1";
const String GAMESTOP = "gamestop";
const String STANDALL = "standall";
const String SHOWALLINFO = "showallinfo";
const int timeLimit = 10;  // 제한시간(초)
const int mPins[] = {3};
const int sPins[] = {A0};
const int scores[] = {100};
const int zeros[] = {0};
const int stands[] = {160, };

typedef class transmisson {
    private:
        String * buf;
        bool recieveCheck;
        int check;
        int count;
    public:
        transmisson() {
            recieveCheck = false;
            check = 0;
            count = 0;
        }
        void clearBuf();
        String getCommend();
        bool recieveProcess_waitting();
        bool recieveProcess_once();
        bool compare(String parm, String tar);
} transmisson;

bool transmisson::recieveProcess_waitting() {  // 명령 대기 (무한대기 버전)
    String buff;
    
    for(this->check = 0, this->count = 0, this->recieveCheck = false;;) {
        if(Serial.available()) {
            this->recieveCheck = true;
            char c = Serial.read();
            buff += c;
            this->check++;
            delay(10);  // Serial이 씹힘
        }
        if(this->recieveCheck) {
            this->count++;
        }
        if(this->check != this->count) {
          buf = new String;
          *(this->buf) = buff;
          Serial.println(*(this->buf));
          return true;
        }
    }
}

bool transmisson::recieveProcess_once() {  // 명령 대기 (확인버전)(스레드점유X)
    String buff;
  
  if(Serial.available()) {
    for(this->check = 0, this->count = 0, this->recieveCheck = false;;) {
            if(Serial.available()) {
                this->recieveCheck = true;
                char c = Serial.read();
                buff += c;
                this->check++;
                delay(10);  // Serial이 씹힘
            }
            if(this->recieveCheck) {
                this->count++;
            }
            if(this->check != this->count) {
                this->buf = new String;
                *(this->buf) = buff; 
                Serial.println(*(this->buf));
                return true;
            }
        } 
    }
    return false; 
}

void transmisson::clearBuf() {
  delete buf;
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

transmisson serial;

////////////////////////////////////////////////////////////////

typedef class target {
  private:
    int mPin;  // 해당 과녁에 구성된 모터 핀 
      int sPin;  // 해당 과녁에 구성된 센서 핀  
    int score;  // 해당 과녁의 점수
      int zero;  // 과녁 영점계수 (다운상태) 
      int stand;  // 과녁 영점계수 (업상태)
      unsigned int downTime;  // 피격시점 시간저장 변수  
      unsigned int downTime_prev;  // 이전 피격시점 시간저장 변수  
      bool stat;  // 과녁 상태  
      Servo moter;
  public:
      target() {
        mPin = 0;
        sPin = 0;
        score = 0;
        zero = 0;
        stand = 0;
      }
      target(int p1, int p2, int input1, int input2, int input3) {
        mPin = p1;
        sPin = p2;
        score = input1;
        zero = input2;
        stand = input3;
        moter.attach(mPin);
        moter.write(zero);
        stat = false;
      }
      void showInfo();
      void standup();
      void standown();
      void moterActivate_stand();
      void moterActivate_zero();
      void moterSleep();
      void moterWake();
      void updateStatus();
      void recordDownTime(unsigned int t);  // 피격 시점 기억
      void recordPreviousDownTime();  // 이전 피격 시점 기억  
      int getDownTime();
      int getPreviousDownTime();
      int getScore();
      bool getStat();
      bool checking();  // 센서 감지 여부 
} target;

void target::showInfo() {
  if(this->mPin > 13) {
    Serial.print("Attached moter port : A"); Serial.println((this->mPin)-13);
  }
  else {
    Serial.print("Attached moter port : "); Serial.println(this->mPin);
  }
  Serial.print("Attached moter port : "); Serial.println(this->mPin);
  Serial.print("Attached senser port : "); Serial.println(this->sPin);
  Serial.print("Registered score : "); Serial.println(this->score);
}

void target::standup() {
    this->moter.write(this->stand);
    delay(250);
    this->moter.write(this->zero);
    delay(250);
} 

void target::standown() {
  
}

void target::moterActivate_stand() {
  this->moter.write(this->stand);
}

void target::moterActivate_zero() {
  this->moter.write(this->zero);
}

void target::moterSleep() {
  this->moter.detach();
}

void target::moterWake() {
  this->moter.attach(this->mPin);
}

void target::updateStatus() {
  if(checking()) {
      this->stat = false;
  }
  else {
      this->stat = !(this->stat);
  }
}

void target::recordDownTime(unsigned int t) {
  this->downTime = t;
}

void target::recordPreviousDownTime() {
  this->downTime_prev = this->downTime;
}

int target::getDownTime() {
  return this->downTime;
}

int target::getPreviousDownTime() {
  return this->downTime_prev;
}

int target::getScore() {
  return this->score;
}

bool target::getStat() {
  return this->stat;
}

bool target::checking() {
  if(digitalRead(this->sPin) == LOW) {
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
    public:
      gameHandler() {
          for(int i = 0; i < 10; i++) {
              objArr[i] = NULL;
          }
      }
      void showAllInfo();
      void addTarget();
      void addTarget(int n1, int n2, int n3, int n4, int n5);
      void delTarget();
      void gamemode0();
      void gamemode1();
      void gamemode2();
      void moterAllSleep();
      void moterAllWake();
      void standAll();
      bool checkDownTerm();
      bool checkTimeOver();
      bool checkAllDown();
      bool checkStopCommend();
} gameHandler;

void gameHandler::showAllInfo() {
  for(int i = 0; i < index; i++) {
      objArr[i]->showInfo();
  }
}

void gameHandler::addTarget() {
  objArr[this->index] = new target(mPins[index], sPins[index], scores[index], zeros[index], stands[index]);
  this->index++;
}

void gameHandler::addTarget(int n1, int n2, int n3, int n4, int n5) {
  objArr[this->index] = new target(n1, n2, n3, n4, n5);
  this->index++;
}

void gameHandler::delTarget() {
  
}

void gameHandler::gamemode0() {
    static int i = 0;
    
    for(;;) {
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
          break;
        }
        for(i = 0; i < this->index; i++) {
            objArr[i]->recordDownTime(millis());
            
            if(objArr[i]->checking()) {  // 피격여부 확인  
                if(objArr[i]->getStat()) {
                  Serial.print(i+1); Serial.println("번째 피격해서");
                    Serial.print(scores[i]); Serial.println("점 흭득!");
                }
                objArr[i]->updateStatus();
                if((objArr[i]->getDownTime() - objArr[i]->getPreviousDownTime()) >= 3000) {
                  objArr[i]->standup() ;
                  objArr[i]->recordPreviousDownTime();
                  objArr[i]->updateStatus();
                }
            }
        }
    }
}

void gameHandler::gamemode1() {
    static int i = 0;
  int result = 0;
  this->startTime = millis();
    
    for(;;) {
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
          break;
        }
        if(checkTimeOver() == true) {
          break;
        }
        for(i = 0; i < this->index; i++) {
            objArr[i]->recordDownTime(millis());
            
            if(objArr[i]->checking()) {  // 피격여부 확인  
                if(objArr[i]->getStat()) {
                    Serial.print(i+1); Serial.println("번째 피격해서");
                    Serial.print(scores[i]); Serial.println("점 흭득!");
                    result += objArr[i]->getScore();
                }
                objArr[i]->updateStatus();
                if((objArr[i]->getDownTime() - objArr[i]->getPreviousDownTime()) >= 3000) {
                  objArr[i]->standup() ;
                  objArr[i]->recordPreviousDownTime();
                  objArr[i]->updateStatus();
                }
            }
        }
    }
    Serial.print("최종점수는 "); Serial.print(result); Serial.println("점!");
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
}

void loop() {
    static int i = 0;
    String obj;

    Serial.print("명령 대기중...(명령어를 쓰세요) : "); 
    if(serial.recieveProcess_waitting()) {
        obj = serial.getCommend();
        
        handler.moterAllWake(); // 모든 모터 등록 (절전모드해제)
        if(serial.compare(obj, GAMEMODE0)) {
            Serial.println("과녁 조율 모드");
            handler.gamemode0();
        }
        else if(serial.compare(obj, GAMEMODE1)) {
            Serial.println("게임모드1");
            handler.gamemode1();
        }
        else if(serial.compare(obj, STANDALL)) {
            Serial.println("모든 과녁 스탠바이");
            handler.standAll();
        }
        else if(serial.compare(obj, SHOWALLINFO)) {
            Serial.println("모든 과녁 정보");
            handler.showAllInfo();
        }
        else {
            Serial.println("리스트에 없는 명령어 입니다");
        }
        handler.moterAllSleep();  // 모든 모터 등록 해제(절전모드)
        serial.clearBuf();
    }
    delay(100);
}
