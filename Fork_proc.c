#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

// ------------------------------------------------------------
// /proc を走査して Aprocess の PID を探す
// ------------------------------------------------------------
int find_aprocess_pid() {
    DIR *dp = opendir("/proc");
    if (!dp) return -1;

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL) {
        if (!isdigit(entry->d_name[0])) continue;

        char path[256];
        snprintf(path, sizeof(path), "/proc/%s/cmdline", entry->d_name);

        FILE *fp = fopen(path, "r");
        if (!fp) continue;

        char cmd[256] = {0};
        fread(cmd, 1, sizeof(cmd), fp);
        fclose(fp);

        if (strstr(cmd, "Aprocess") != NULL) {
            closedir(dp);
            return atoi(entry->d_name);
        }
    }

    closedir(dp);
    return -1;
}

// ------------------------------------------------------------
// 子スレッド：Aprocess を起動する
// ------------------------------------------------------------
void* child_thread(void* arg) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);

    struct sched_param sp;
    sp.sched_priority = 80;   // 親より高い優先度

    sched_setaffinity(0, sizeof(set), &set);
    sched_setscheduler(0, SCHED_FIFO, &sp);

    // Aprocess を起動（別プロセスとして）
    pid_t pid = fork();
    if (pid == 0) {
        execl("./Aprocess", "Aprocess", NULL);
        perror("exec");
        _exit(1);
    }

    return NULL;
}

// ------------------------------------------------------------
// メイン：親スレッドが Aprocess の起動確認を行う
// ------------------------------------------------------------
int main() {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);

    struct sched_param sp_parent;
    sp_parent.sched_priority = 60;

    sched_setaffinity(0, sizeof(set), &set);
    sched_setscheduler(0, SCHED_FIFO, &sp_parent);

    // 子スレッド作成
    pthread_t th;
    pthread_create(&th, NULL, child_thread, NULL);

    // 親スレッド：Aprocess の起動確認
    while (1) {
        int apid = find_aprocess_pid();
        if (apid > 0) {
            printf("Aprocess is running (PID=%d)\n", apid);
        } else {
            printf("Aprocess is NOT running\n");
        }

        usleep(200000);  // 子がブロックした瞬間に動く
    }
}
