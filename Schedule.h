#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <stdlib.h>
#include "TimeLib.h"

typedef void (*TaskHanlder)(int taskId);
enum TaskTimeUnit {ttuYear, ttuMonth, ttuDay, ttuHour};
static uint32_t checkInterval = 30000; //msec 

class Schedule {
  protected:
    Schedule* child = NULL;
    time_t tsSkip = 0;
    uint8_t taskId;
    TaskTimeUnit unit = ttuYear;
  public:
    Schedule(uint8_t taskId, TaskTimeUnit unit) ;

    Schedule* getChild() ;
    void setChild(Schedule* child);
    int getTaskId();
    bool isSkip(time_t tm);
    virtual bool execute(time_t tm) = 0;
};


class SchedulePeriodTask: public Schedule {
  private:
    time_t from, to;
  public:
    SchedulePeriodTask(time_t from, time_t to, uint8_t taskId, TaskTimeUnit unit);
    bool execute(time_t tm);
};

class ScheduleOnTimeTask: public Schedule {
  private:
    time_t time;
  public:
    ScheduleOnTimeTask(time_t time, uint8_t taskId, TaskTimeUnit unit);
    bool execute(time_t tm);
};

class Scheduler {
  private:
    unsigned long tsCheckInterval = 0;
    TaskHanlder taskHandler = NULL;
    Schedule * task_root = NULL;
	time_t time;
    void addSchedulePeriodTask(time_t from, time_t to, uint8_t taskId, TaskTimeUnit unit);
  public:
    void begin(TaskHanlder handler);

    void addSchedule(uint8_t month, uint8_t day, int8_t hour, uint8_t min, uint8_t taskId);
    void addHourSchedule(uint8_t fromHour, uint8_t fromMin, uint8_t toHour, uint8_t toMin, uint8_t taskId);
    void addDaySchedule(uint8_t fromDay, uint8_t fromHour, uint8_t fromMin, uint8_t toDay, uint8_t toHour, uint8_t toMin, uint8_t taskId);
    void addMonthSchedule(uint8_t fromMonth, uint8_t fromDay, uint8_t fromHour, uint8_t fromMin, uint8_t toMonth, uint8_t toDay, uint8_t toHour, uint8_t toMin, uint8_t taskId);

    bool isActive(int taskId); 

    void maintenance(time_t time);
};

#endif
