/*
 ============================================================================
 Name        : GoogleTest.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>    // O_* 定数
#include <sys/stat.h> // mode 定数
#include <errno.h>
#include <spawn.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define QUEUE_NAME  "/my_queue"  // メッセージキュー名（先頭は / 必須）
#define MAX_SIZE    256          // メッセージ最大長
extern char **environ; // 環境変数

// スレッドで実行する関数
void *thread_func(void *arg) {
    pid_t pid = fork();
    struct sched_param param;

    param.sched_priority = 60;   // B の優先度

    if (pid < 0) {
        perror("fork failed");
        pthread_exit(NULL);
    }
    else if (pid == 0) {
        sched_setscheduler(0, SCHED_FIFO, &param);
        // 子プロセス: 新しいプログラムを実行
        execl("/home/user/eclipse-workspace/Receiver/Debug/Receiver", "Receiver", NULL);
        perror("execl failed");
        _exit(1); // exec失敗時のみ終了
    }
    else {
        // 親プロセス（スレッド側）: 子プロセス終了待ち
        int status;
        waitpid(pid, &status, 0);
        printf("[Thread] Child process finished with status %d\n", status);
    }

    pthread_exit(NULL);
}

int main(void) {
    mqd_t mq;
    char buffer[MAX_SIZE];
    struct mq_attr attr;
	pthread_t tid;

    struct sched_param param;
    param.sched_priority = 70;   // スレッドの優先度

    // int ret = pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    // if (ret != 0) {
    //     perror("pthread_setschedparam");
    // }

	// pid_t pid;
	// char *argv[] = {"Receiver"};

    // メッセージキュー属性設定
    attr.mq_flags = 0;           // ブロッキングモード
    attr.mq_maxmsg = 10;         // キューに入れられる最大メッセージ数
    attr.mq_msgsize = MAX_SIZE;  // 1メッセージの最大サイズ
    attr.mq_curmsgs = 0;         // 現在のメッセージ数（初期値）

    // メッセージキューを作成・オープン（書き込み専用）
    mq = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

	// 新しいプロセスを生成
    // int status = posix_spawnp(&pid, "./", NULL, NULL, argv, environ);
    // if (status != 0) {
    //     perror("posix_spawn");
    //     return EXIT_FAILURE;
    // }

	// スレッド作成
    if (pthread_create(&tid, NULL, thread_func, NULL) != 0) {
        perror("pthread_create failed");
        return 1;
    }

    printf("送信するメッセージを入力してください（終了は exit）:\n");

    while (1) {
        printf("> ");
        if (!fgets(buffer, MAX_SIZE, stdin)) {
            perror("fgets");
            break;
        }

        // 改行を削除
        buffer[strcspn(buffer, "\n")] = '\0';

		// メッセージ送信
        if (mq_send(mq, buffer, strlen(buffer) + 1, 0) == -1) {
            perror("mq_send");
        } else {
            printf("送信しました: %s\n", buffer);
        }

        // メッセージが読み取られ待ち
        while(1)
        {
            mq_getattr(mq, &attr);
            if (attr.mq_curmsgs == 0) {
                break;
            }
        }

        // "exit" で終了
        if (strcmp(buffer, "exit") == 0) {
            break;
        }
        usleep(1);
    }

	// スレッド終了待ち
    pthread_join(tid, NULL);

    // キューを閉じる
    if (mq_close(mq) == -1) {
        perror("mq_close");
    }

	// // 子プロセス終了待ち
    // if (waitpid(pid, &status, 0) == -1) {
    //     perror("waitpid");
    //     return EXIT_FAILURE;
    // }

    return 0;
}
