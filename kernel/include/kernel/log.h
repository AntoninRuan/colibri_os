#ifndef LOG_H
#define LOG_H

enum log_level {
  DEBUG,
  INFO,
  WARNING,
  ERROR,
};

typedef enum log_level log_level_t;

void enable_tty_log();
void disable_tty_log();
void log(log_level_t, char*);

#endif // LOG_H
