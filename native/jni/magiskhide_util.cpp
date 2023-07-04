#include <signal.h>
#include <unistd.h>
#include <vector>
#include <sys/mount.h>
#include <iostream>
#include <string_view>
#include <string>
#include <sys/poll.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <algorithm>
#include <sys/prctl.h>
#include <sys/types.h>
#include <dirent.h>
#include <set>
#include <vector>

#include "magiskhide_util.hpp"
#include "utils.hpp"
#include "logging.hpp"

using namespace std;

#define CMD_UID_GRANTED_ROOT 12
#define CMD_UID_SHOULD_UMOUNT 13

std::vector<string> module_list;

bool is_hide_target(int uid) {
    int32_t result = -1;
    bool umount = false;
    prctl(0xdeadbeef, CMD_UID_SHOULD_UMOUNT, uid % 100000, &umount, &result);
    return umount;
}


bool is_granted_root_target(int uid) {
    int32_t result = -1;
    bool granted = false;
    prctl(0xdeadbeef, CMD_UID_GRANTED_ROOT, uid % 100000, &granted, &result);
    return granted;
}

static void lazy_unmount(const char* mountpoint) {
    if (umount2(mountpoint, MNT_DETACH) != -1)
        LOGD("hide_daemon: Unmounted (%s)\n", mountpoint);
}


void hide_unmount(int pid) {
    if (pid > 0) {
        if (switch_mnt_ns(pid))
            return;
        LOGD("hide: handling PID=[%d]\n", pid);
    }
    std::set<std::string> targets;

    // unmount KSU node
    for (auto &info: parse_mount_info("self")) {
        if (info.source == "KSU" || info.device == worker_dev) {
            targets.insert(info.target);
        }
    }
    // unmount everything under /data/adb
    for (auto &info: parse_mount_info("self")) {
        if (starts_with(info.target.data(), "/data/adb/")) {
            targets.insert(info.target);
        }
    }
    if (targets.empty()) return;

    auto last_target = *targets.cbegin() + '/';
    for (auto iter = next(targets.cbegin()); iter != targets.cend();) {
        if (starts_with((*iter).data(), last_target.data())) {
            iter = targets.erase(iter);
        } else {
            last_target = *iter++ + '/';
        }
    }
    for (auto &s : reversed(targets))
        lazy_unmount(s.data());
}

void hide_daemon(int pid) {
    if (fork_dont_care() == 0) {
        hide_unmount(pid);
        kill(pid, SIGCONT);
        _exit(0);
    }
}

int overlay_main(int argc, char **argv);

static int get_argc(char **argv) {
    int argc = 0;
    for (int i = 0; argv[i]; i++) argc++;
    return argc;
}

void mount_daemon(int pid) {
	if (worker_dev == 0)
	    return;
    struct stat st;
    int mount_child = fork();
    if (mount_child > 0) waitpid(mount_child, nullptr, 0);
    else if (mount_child == 0) {
        if (switch_mnt_ns(pid) == 0 &&
            !mount(nullptr, "/", nullptr, MS_PRIVATE | MS_REC, nullptr) &&
            !mount("tmpfs", "/mnt", "tmpfs", 0, nullptr) &&
            !chdir("/mnt") &&
            !mknod("/mnt/KSU", S_IFBLK, worker_dev) &&
            !mount("KSU", "/data/adb/modules", "ext4", 0, nullptr)) {
            umount2("/mnt", MNT_DETACH);
            const char *parts[] = { "/system", "/vendor", "/product", "/system_ext", nullptr };
            for (int pos = 0; parts[pos] != nullptr; pos++) {
                const char *part = parts[pos];
                if (lstat(part, &st) || !S_ISDIR(st.st_mode))
                    continue;
                int child = fork();
                if (child > 0) waitpid(child, nullptr, 0);
                else if (child == 0) {
                    std:vector<string> mount_list; {
                        for (auto i = 0; i < module_list.size(); i++) {
                            std::string mmnt = "/data/adb/modules/"s + module_list[i] + part;
                            if (lstat(mmnt.data(), &st) || !S_ISDIR(st.st_mode))
                                continue;
                            mount_list.push_back(mmnt);
                        }
                        if (mount_list.size() >= 1) {
                            // /data/adb/modules/KSU_overlay_unmount/magic-mount -rn KSU MNT PART PART
                            char *argc_exec[7+mount_list.size()];
                            argc_exec[0] = "magic-mount";
                            argc_exec[1] = "-rn";
                            argc_exec[2] = "KSU";
                            for (int i = 0; i < mount_list.size(); i++)
                                argc_exec[3 + i] = (mount_list[i]).data();
                            argc_exec[3 + mount_list.size()] = (char*)part;
                            argc_exec[4 + mount_list.size()] = (char*)part;
                            argc_exec[5 + mount_list.size()] = nullptr;
                            LOGD("mount_daemon: PID=[%d] partition=[%s]\n", pid, part);
                            overlay_main(get_argc(argc_exec), argc_exec);
                        }
                    }
                    _exit(0);
                }
            }
        }
        _exit(0);
    }
}

void prepare_modules(){
    const char *MODULEDIR = "/data/adb/modules";
    DIR *dirfp = opendir(MODULEDIR);
    if (dirfp != nullptr){
        dirent *dp;
        while ((dp = readdir(dirfp))) {
            if (dp->d_name == "."sv || dp->d_name == ".."sv) continue;
            char buf[strlen(MODULEDIR) + 1 + strlen(dp->d_name) + 20];
            snprintf(buf, sizeof(buf), "%s/%s/", MODULEDIR, dp->d_name);
            char *z = buf + strlen(MODULEDIR) + 1 + strlen(dp->d_name) + 1;
            strcpy(z, "disable");
            if (access(buf, F_OK) == 0) {
                LOGD("module: [%s] disabled\n", dp->d_name);
                continue;
            }
            strcpy(z, "remove");
            if (access(buf, F_OK) == 0) {
                LOGD("module: [%s] removed\n", dp->d_name);
                continue;
            }
            strcpy(z, "skip_mount");
            if (access(buf, F_OK) == 0) {
                LOGI("module: [%s] enabled but skip mount\n", dp->d_name);
                continue;
            }
            LOGI("module: [%s] enabled\n", dp->d_name);
            module_list.emplace_back(string(dp->d_name));
        }
        closedir(dirfp);
    }
}

