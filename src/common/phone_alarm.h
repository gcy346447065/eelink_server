//
// Created by jk on 16-11-10.
//

#ifndef ELECTROMBILE_PHONE_ALARM_H
#define ELECTROMBILE_PHONE_ALARM_H
#ifdef __cplusplus
    extern "C"{
#endif

int phone_alarm(const char* telphone);
int phone_alarmWithCaller(const char* telphone, const char* caller);
#ifdef __cplusplus
            }
#endif

#endif //ELECTROMBILE_PHONE_ALARM_H