#pragma once
#include <stdio.h>
#include <vector>
#include <map>
#include <sys/types.h>
#define SIGTERMTHRD SIGUSR1

extern const char *MAGISKTMP;

void prepare_modules();

extern std::map<int, std::vector<std::string>> uid_proc_map;

extern bool new_magic_mount;
extern bool trace_log;
extern int SDK_INT;
extern dev_t worker_dev;

#define APP_DATA_DIR (SDK_INT >= 24 ? "/data/user_de" : "/data/user")

bool is_hide_target(int uid);
void hide_daemon(int pid);
void hide_unmount(int pid = -1);
void proc_monitor();
void mount_daemon(int pid);

