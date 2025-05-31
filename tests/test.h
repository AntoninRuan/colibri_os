typedef enum { OK, ERROR } status;

char *status_str[2] = {
    "[\033[32mPASSED\033[0m]",
    "[\033[31mFAILED\033[0m]",
};

static inline void log_test(status s, char *msg, ...) {
    va_list ap;
    va_start(ap, msg);

    char buf[1024];
    vsprintf(buf, msg, ap);
    va_end(ap);

    printf("%s %s\n", status_str[s], buf);
}
