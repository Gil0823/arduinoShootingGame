#include <stdio.h>
#include <SoftwareSerial.h>
#include <Servo.h>           //  ********명령어리스트******** 
#define GAME_START 'a'      //   * a : 게임시작             *
#define GAME_STOP 'b'      //    * b : 게임중지             *
#define GAMEMODE_0 'c'    //     * c : 조율모드             *
#define GAMEMODE_1 'd'   //      * d : 솔로플레이           *      
#define NUM_T 3      //       *****************************

using namespace std;

const int timeLimit = 10;  // 제한시간(초)
int mPins[] = {0, 0};
int sPins[] = {0, 0};
int scores[] = {100, 100};
int zeros[] = {0, 0};
int stands[] = {160, 160};

typedef class target {
  private:
    int mPin;  // 해당 과녁에 구성된 모터 핀 
    int sPin;  // 해당 과녁에 구성된 센서 핀  
    int score;  // 해당 과녁의 점수
    int zero;  // 과녁 영점계수 (다운상태) 
    int stand;  // 과녁 영점계수 (업상태) 
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
    }
    void showInfo();
    void standup();
    void standown();
    void updateStatus();
    bool checking();  // 센서 감지 여부 
} target;

void target::showInfo() {
  Serial.print("Attached moter port : "); Serial.println(this->mPin);
  Serial.print("Attached senser port : "); Serial.println(this->sPin);
  Serial.print("Registered score : "); Serial.println(this->score);
  Serial.print("status of target : "); Serial.println(this->stat);
}

void target::standup() {
    this->moter.write(this->stand);
    delay(500);
    this->moter.write(this->zero);
    delay(500);
} 

void target::standown() {
  
}

void target::updateStatus() {
  this->stat = !(this->stat);
}

bool target::checking() {
  if(digitalRead(this->sPin) == LOW) {
    return true;
  }
  else {
    return false;
  }
}

typedef class gameHandler {
  private:
    target * objArr[10];  // 과녁객체
    int index;  // 과녁 색인   
  public:
      gameHandler() {
        for(int i = 0; i < 10; i++) {
          objArr[i] = NULL;
      }
    }
    void showAllInfo();
    void addTarget();
    void delTarget();
    void gamemode0();
    void gamemode1();
    bool checkTimeOver();
    bool checkAllDead();
    bool checkStopCommend();
} gameHandler;

void gameHandler::showAllInfo() {
  for(int i = 0; i < index+1; i++) {
    objArr[i]->showInfo();
  }
}

void gameHandler::addTarget() {
  objArr[index] = new target(mPins[index], sPins[index], scores[index], zeros[index], stands[index]);
}

void gameHandler::delTarget() {
  
}

void gameHandler::gamemode0() {
  static int i = 0;
    static unsigned int c[3] = {0, };
    static unsigned int p[3] = {0, };
    c[0] = 0;
    c[1] = 0;
    c[2] = 0;
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    
    for(;;) {
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
          break;
        }
        for(i = 0; i < index+1; i++) {
            if(objArr[i]->checking()) {  // 피격여부 확인  
                Serial.print(scores[i]); Serial.println("점 흭득!");
                objArr[i]->updateStatus();
                for(;;) {
                   c[i] = millis();
                   if(c[i] - p[i] >= 3000) {  // 3초 후에 ( 누적 )
                    objArr[i]->standup();  // 스탠드업
                    p[i] = c[i];
                    Serial.print(i); Serial.println("번째 업");
                    objArr[i]->updateStatus();
                    break;
                  }
                }
            }
        }
    }
}

void gameHandler::gamemode1() {
  /*static int i = 0;
    static int score = 0;  // 최종점수 
    static unsigned int c = 0;
    static unsigned int p = 0;
    c = 0;
    p = 0;
    
    for(;;) {
        if(checkTimeOver() == true) {
          Serial.println("타임오버");
          break;
        }
        if(checkStopCommend() == true) {  // 커맨더로부터 게임중지 명령 왔는지 확인 
            Serial.println("확인");
            break;
        }
        if(checkAllDead() == true) {  // 모든 과녁이 다운됐는지 확인 
            for(;;) {
                c = millis();
                if(c - p >= 3000) {  // 3초 후에 ( 누적 )
                    standup();  // 스탠드업
                    target[i].stat_stand = 1;
                    p = c;
                    Serial.print(i); Serial.println("번째 업");
                    break;
                 }
             }
             for(i = 0; i < NUM_T; i++) {  // 모든 과녁을 다 일으킴 
                 standup(i);
             }  
        }
        else {
            for(i = 0; i < index+1; i++) {
                if(objArr[i]->checking()) {  // 피격여부 확인
                    Serial.print(target[i].score); Serial.println("점 흭득!");
                    objArr[i]->updateStatus();
                    score += scores[i];
                }
            }
        }
    }
    Serial.println(score);  // 최종점수를 모니터에 출력(이부분은 세그먼트로 바꿀예정) 
    score = 0;  // 다음판을 위해 최종점수 초기화 */
}

bool gameHandler::checkTimeOver() {
  unsigned int c = 0;
    unsigned int p = 0;
    
    c = millis();
    if(c - p >= timeLimit*1000) {
      return true;
    }
    else {
      return false;
    }
}

bool gameHandler::checkAllDead() {
  static int i = 0;
    static int count = 0;
  
    for(i = 0; i < index+1; i++) {
        if(objArr[i]->checking()) {  // 기립상태 확인 
            count++;  // 다운된 과녁수를 계산함 
        }   
    }
    if(count >= index+1) {  // 다운된 과녁수가 NUM_T개이상인지 확인 ( 오류로 NUM_T개가 넘을시를 위해 조건을 이상으로 설정함 ) 
       count = 0;  // 다음 턴을 위해 카운트수를 초기화  
       return true;
    }
    else {
        return false;
    }
}

bool gameHandler::checkStopCommend() {
    static int i = 0;
    char recieve;
    
    if(Serial.available() > 0) {  // 커맨더로부터 명령대기 
        recieve = Serial.read();  // 수신된 명령을 recieve에 저장
        if(recieve == GAME_STOP) {  // 수신된 명령이 게임중지인지 확인  
            for(i = 0; i < index+1; i++) {
                objArr[i]->standown();  // 모든 과녁을 다운시킴 
            }
        return true;
        }
        else {
            return false;
        }
    }
    else {  // 이 함수는 명령이 수신됐는지 한번만 확인하면 돼므로 한번만 확인하고 그냥 넘어감 
        return false;
    }
}

void setup() {
  
}

void loop() {
  
}

