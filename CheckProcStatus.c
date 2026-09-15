char wchan_path[256];
snprintf(wchan_path, sizeof(wchan_path),
         "/proc/%d/task/%d/wchan", apid, apid);

FILE *fp = fopen(wchan_path, "r");
char wchan[128] = {0};
fgets(wchan, sizeof(wchan), fp);
fclose(fp);

int is_mq_wait = strstr(wchan, "mq") || strstr(wchan, "do_wait");

DIR *tdir = opendir(task_path);
struct dirent *tentry;

while ((tentry = readdir(tdir)) != NULL) {
    if (!isdigit(tentry->d_name[0])) continue;

    int tid = atoi(tentry->d_name);
    if (tid == apid) continue;  // main スレッドは除外

    char wchan_path[256];
    snprintf(wchan_path, sizeof(wchan_path),
             "/proc/%d/task/%d/wchan", apid, tid);

    FILE *fp = fopen(wchan_path, "r");
    char wchan[128] = {0};
    fgets(wchan, sizeof(wchan), fp);
    fclose(fp);

    if (strstr(wchan, "futex")) {
        mutex_waiting = 1;
        break;
    }
}

while (1) {
    int apid = find_aprocess_pid();
    if (apid > 0) {
        int is_mq_wait = check_main_mq_wait(apid);
        int mutex_waiting = check_worker_mutex_wait(apid);

        if (is_mq_wait && mutex_waiting) {
            send_trigger();
        }
    }

    usleep(200000);
}



