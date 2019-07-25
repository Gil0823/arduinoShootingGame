#include "Target.h"

target::target() {
            gmPin = 0;
            cmPin = 0;
            sPin = 0;
            score = 0;
            for(int i = 0; i < 2; i++) posArray_gm[i] = 0;
            for(int i = 0; i < 2; i++) posArray_cm[i] = 0;
            stat = true;
        }

target::target(int gmP, int cmP, int sP, int lP, int num_score, int num_zero_gm, int num_act_gm, int num_zero_cm, int num_act_cm) {
            gmPin = gmP;
            cmPin = cmP;
            sPin =  sP;
            lPin = lP;
            score = num_score;
            posArray_gm[zero] = num_zero_gm;
            posArray_gm[act] = num_act_gm;
            posArray_cm[zero] = num_zero_cm;
            posArray_cm[act] = num_act_cm;
            stat = true;
            
            pinMode(sPin, INPUT);
            pinMode(lPin, OUTPUT);
            groudMotor.attach(gmPin);
            groudMotor.write(posArray_gm[zero]);
            coverMotor.attach(cmPin);
            coverMotor.write(posArray_cm[zero]);
        }

target::target(int infoArray[]) {
    gmPin = infoArray[gmP];
    cmPin = infoArray[cmP];
    sPin = infoArray[sP];
    lPin = infoArray[lP];
    score = infoArray[score_];
    for (int i = 0; i < 2; i++) posArray_gm[i] = infoArray[zero_gm + i];
    for (int i = 0; i < 2; i++) posArray_cm[i] = infoArray[zero_cm + i];
    stat = true;

    pinMode(sPin, INPUT);
    pinMode(lPin, OUTPUT);
    groudMotor.attach(gmPin);
    groudMotor.write(posArray_gm[zero]);
    coverMotor.attach(cmPin);
    coverMotor.write(posArray_cm[zero]);
}

int target::clipOut(int index) {
    switch(index) {
        case gmP:
            return gmPin;
            break;
        case cmP:
            return cmPin;
            break;
        case sP:
            return sPin;
            break;
        case lP:
            return lPin;
            break;
        case score_:
            return score;
            break;
        case zero_gm:
            return posArray_gm[zero];
            break;
        case act_gm:
            return posArray_gm[act];
            break;
        case zero_cm:
            return posArray_cm[zero];
            break;
        case act_cm:
            return posArray_cm[act];
            break;
        default:
            Serial.println("???");
    }
}

void target::showInfo() {
    if (gmPin >= 13) {
        Serial.print("Attached motor port : A"); Serial.println(gmPin - 14);
    }
    else {
        Serial.print("Attached motor port : "); Serial.println(gmPin);
    }
    
    if (sPin >= 13) {
        Serial.print("Attached sensor port : A"); Serial.println(sPin - 14);
    }
    else {
        Serial.print("Attached sensor port : "); Serial.println(sPin);
    }
    
    Serial.print("Registered score : "); Serial.println(score);
    Serial.println();
}

void target::activateGroundMotor_act() {
    groudMotor.write(posArray_gm[act]);
}

void target::activateGroundMotor_zero() {
    groudMotor.write(posArray_gm[zero]);
}

void target::activateCoverMotor_act() {
    coverMotor.write(posArray_cm[act]);
}

void target::activateCoverMotor_zero() {
    coverMotor.write(posArray_cm[zero]);
}

void target::ledOn() {
    digitalWrite(lPin, HIGH);
}

void target::ledOff() {
    digitalWrite(lPin, LOW);
}

void target::disableMotor() {
    groudMotor.detach();
    coverMotor.detach();
}

void target::enableMotor() {
    groudMotor.attach(gmPin);
    coverMotor.attach(cmPin);
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
