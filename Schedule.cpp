#include "Schedule.h"

Schedule::Schedule(uint8_t taskId, TaskTimeUnit unit) {
  this->taskId = taskId;
  this->unit = unit;
}

Schedule* Schedule::getChild() {
  return child;
}

void Schedule::setChild(Schedule* child) {
  this->child = child;
}

int Schedule::getTaskId() {
  return taskId;
}

bool Schedule::isSkip(time_t tm) {
  return tsSkip > tm;
}


SchedulePeriodTask::SchedulePeriodTask(time_t from, time_t to, uint8_t taskId, TaskTimeUnit unit): Schedule(taskId, unit) {
  this->from = from;
  this->to = to;
}

bool SchedulePeriodTask::execute(time_t tm) {
  bool result = false;
  time_t utStartPeriod, utEndPeriod = 0;

  switch (unit) {
    case ttuMonth:
      utStartPeriod = startOfYear(tm);
      utEndPeriod = endOfYear(tm);
      break;
    case ttuDay:
      utStartPeriod = startOfMonth(tm);
      utEndPeriod = endOfMonth(tm);
      break;
    case ttuHour:
      utStartPeriod = previousMidnight(tm);
      utEndPeriod = nextMidnight(tm);
      break;
  }

  if (from > to) {
    result = ((tm >= utStartPeriod) && (tm < (utStartPeriod + to))) || ((tm >= (utStartPeriod + from)) && (tm < utEndPeriod));
    if (result)
        tsSkip = utEndPeriod + from;

  } else {
    result = ((tm >= utStartPeriod + from) && (tm < utStartPeriod + to));
    if (result)
        tsSkip = utStartPeriod + to;   
  }

  return result;
}

ScheduleOnTimeTask::ScheduleOnTimeTask(time_t time, uint8_t taskId, TaskTimeUnit unit): Schedule(taskId, unit) {
  this->time = time;
}

bool ScheduleOnTimeTask::execute(time_t tm) {
  bool result = false;
  time_t utStartPeriod = startOfYear(tm);

  switch (unit) {
    case ttuMonth:
      utStartPeriod = startOfMonth(tm);
      break;
    case ttuDay:
      utStartPeriod = previousMidnight(tm);
      break;
    case ttuHour:
      utStartPeriod = previousHour(tm);
      break;
  }

  time_t utime = utStartPeriod + time;
  result = ((tm >= utime) && (tm < utime + SECS_PER_MIN));
  if ((result) && (tsSkip < tm))
    tsSkip = utime + (SECS_PER_MIN * 2);

  return result;
}

void Scheduler::addSchedulePeriodTask(time_t from, time_t to, uint8_t taskId, TaskTimeUnit unit) {
  Schedule* task = new SchedulePeriodTask(from, to, taskId, unit);
  if (task_root != NULL)
    task->setChild(task_root);
  task_root = task;
}

void Scheduler::begin(TaskHanlder handler) {
  this->taskHandler = handler;
}

void Scheduler::addSchedule(uint8_t month, uint8_t day, int8_t hour, uint8_t min, uint8_t taskId) {
  TaskTimeUnit unit = ttuHour;

  if (month != 0) {
    unit = ttuYear;
  } else if (day != 0) {
    unit = ttuMonth;
  } else if (hour != -1) {
    unit = ttuDay;
  }

  if (hour == -1)
    hour = 0;

  Schedule* task = new ScheduleOnTimeTask(makeTime(0, month, day, hour, min, 0), taskId, unit);
  if (task_root != NULL)
    task->setChild(task_root);
  task_root = task;
}


void Scheduler::addHourSchedule(uint8_t fromHour, uint8_t fromMin, uint8_t toHour, uint8_t toMin, uint8_t taskId) {
  addSchedulePeriodTask(makeTime(0, 0, 0, fromHour, fromMin, 0), makeTime(0, 0, 0, toHour, toMin, 0), taskId, ttuHour);
}

void Scheduler::addDaySchedule(uint8_t fromDay, uint8_t fromHour, uint8_t fromMin, uint8_t toDay, uint8_t toHour, uint8_t toMin, uint8_t taskId) {
  addSchedulePeriodTask(makeTime(0, 0, fromDay, fromHour, fromMin, 0), makeTime(0, 0, toDay, toHour, toMin, 0), taskId, ttuDay);
}

void Scheduler::addMonthSchedule(uint8_t fromMonth, uint8_t fromDay, uint8_t fromHour, uint8_t fromMin, uint8_t toMonth, uint8_t toDay, uint8_t toHour, uint8_t toMin, uint8_t taskId) {
  addSchedulePeriodTask(makeTime(0, fromMonth, fromDay, fromHour, fromMin, 0), makeTime(0, toMonth, toDay, toHour, toMin, 0), taskId, ttuMonth);
}

void Scheduler::maintenance(time_t time) { 
  this->time = time;
  
  if (tsCheckInterval > this->time)
	return;
  
  if (taskHandler == NULL)
    return;

  Schedule* task = task_root;
  while (task != NULL) {
    if ((!task->isSkip(this->time)) && (task->execute(this->time)))
      taskHandler(task->getTaskId());
    task = task->getChild();
  }
  tsCheckInterval = this->time + checkInterval;
}

bool Scheduler::isActive(int taskId){
  Schedule* task = task_root;
  while (task != NULL) {
    if (taskId == task->getTaskId()) {
      return task->execute(this->time);
    }
    task = task->getChild();
  }
  return false;
}
