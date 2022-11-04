#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <bits/types/siginfo_t.h>
#include <linux/prctl.h>
#include <slurm/slurm_errno.h>
#include <sys/wait.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <stdio.h>

#include <optional>
#include <string>

// // root version
// #include "mount.hpp"

#include <atomic>
#include <err.h>
#include <fcntl.h>
#include <optional>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdexcept>
#include <string.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <vector>
#include <wait.h>

extern "C" {
// #include "rootless.h"
#include <slurm/spank.h>
}

//
// Forward declare the implementation of the plugin callbacks.
//

struct shared_memory {
  pthread_mutex_t mutex;
  std::atomic<int> init_tasks{0};
  std::atomic<int> completed_tasks{0};
  std::atomic<int> started_tasks{0};
  // atomic_uint completed_tasks;
  // pid_t pid;
  pid_t ns_pid;
};

static shared_memory *shm;

static struct shared_memory *shm_init(void) {
  struct shared_memory *shm = NULL;
  pthread_mutexattr_t mutex_attr;
  int ret;

  shm = (shared_memory *)mmap(0, sizeof(*shm), PROT_READ | PROT_WRITE,
                              MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (shm == MAP_FAILED) {
    shm = NULL;
    goto fail;
  }

  ret = pthread_mutexattr_init(&mutex_attr);
  if (ret < 0)
    goto fail;

  ret = pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
  if (ret < 0)
    goto fail;

  ret = pthread_mutexattr_setrobust(&mutex_attr, PTHREAD_MUTEX_ROBUST);
  if (ret < 0)
    goto fail;

  ret = pthread_mutex_init(&shm->mutex, &mutex_attr);
  if (ret < 0)
    goto fail;

  shm->ns_pid = -1;
  shm->init_tasks = 0;
  shm->started_tasks = 0;
  shm->completed_tasks = 0;

  return shm;

fail:
  if (shm != NULL)
    munmap(shm, sizeof(*shm));
  return (NULL);
}

inline static void print_s_ctx(const char label[], int pid) {
  auto context = spank_context();
  switch (context) {
  case spank_context_t::S_CTX_ALLOCATOR: {
    slurm_spank_log("[ PID %4d ] [%14s] in %s", pid, "CTX_ALLOCATOR", label);
    break;
  }
  case spank_context_t::S_CTX_LOCAL: {
    slurm_spank_log("[ PID %4d ] [%14s] in %s", pid, "CTX_LOCAL", label);
    break;
  }
  case spank_context_t::S_CTX_REMOTE: {
    slurm_spank_log("[ PID %4d ] [%14s] in %s", pid, "CTX_REMOTE", label);
    break;
  }
  case spank_context_t::S_CTX_SLURMD: {
    slurm_spank_log("[ PID %4d ] [%14s] in %s", pid, "CTX_SLURMD", label);
    break;
  }
  case spank_context_t::S_CTX_JOB_SCRIPT: {
    slurm_spank_log("[ PID %4d ] [%14s] in %s", pid, "CTX_JOB_SCRIPT", label);
    break;
  }

  default:
    break;
  }
}

namespace impl {
int slurm_spank_init(spank_t sp, int ac, char **av);
int slurm_spank_init_post_opt(spank_t sp, int ac, char **av);
int slurm_spank_local_user_init(spank_t sp, int ac, char **av);
int slurm_spank_user_init(spank_t sp, int ac, char **av);
int slurm_spank_task_init(spank_t sp, int ac, char **av);
int slurm_spank_task_init_privileged(spank_t sp, int ac, char **av);
int slurm_spank_task_exit(spank_t sp, int ac, char **av);
} // namespace impl

//
// Implement the SPANK plugin C interface.
//

extern "C" {

extern const char plugin_name[] = "TODO-assign-a-name";
extern const char plugin_type[] = "spank";
#ifdef SLURM_VERSION_NUMBER
extern const unsigned int plugin_version = SLURM_VERSION_NUMBER;
#endif
extern const unsigned int spank_plugin_version = 1;

// Called from both srun and slurmd.
int slurm_spank_init(spank_t sp, int ac, char **av) {
  return impl::slurm_spank_init(sp, ac, av);
}

int slurm_spank_init_post_opt(spank_t sp, int ac, char **av) {
  return impl::slurm_spank_init_post_opt(sp, ac, av);
}

// int slurm_spank_local_user_init(spank_t sp, int ac, char **av) {
//   return impl::slurm_spank_local_user_init(sp, ac, av);
// }

int slurm_spank_user_init(spank_t sp, int ac, char **av) {
  return impl::slurm_spank_user_init(sp, ac, av);
}

int slurm_spank_task_init_privileged(spank_t sp, int ac, char **av) {
  return impl::slurm_spank_task_init_privileged(sp, ac, av);
}

int slurm_spank_task_init(spank_t sp, int ac, char **av) {
  return impl::slurm_spank_task_init(sp, ac, av);
}

// int slurm_spank_job_epilog(spank_t sp, int ac, char **av) {
//   print_s_ctx(__func__, getpid());
//   return ESPANK_SUCCESS;
// }

int slurm_spank_task_exit(spank_t sp, int ac, char **av) {
  print_s_ctx(__func__, getpid());

  return impl::slurm_spank_task_exit(sp, ac, av);
}

} // extern "C"

//
// Implementation
//

namespace impl {

void test_unshare_user() {
  if (unshare(CLONE_NEWUSER)) {
    slurm_spank_log("unshare(CLONE_NEWUSER) %s", strerror(errno));
  } else {
    slurm_spank_log("unshare(CLONE_NEWUSER) worked");
  }
}

void test_unshare_user_mount() {
  if (unshare(CLONE_NEWUSER | CLONE_NEWNS)) {
    slurm_spank_log("unshare(CLONE_NEWUSER | CLONE_NEWNS) %s", strerror(errno));
  } else {
    slurm_spank_log("unshare(CLONE_NEWUSER | CLONE_NEWNS) worked");
  }
}

struct arg_pack {
  std::string mount_point = "/user-environment";
  std::optional<std::string> file;
  bool run_prologue = false;
};

static arg_pack args{};

int enter_user_and_mount_namespace(int pid) {
  // enter usernamespace
  {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/ns/user", shm->ns_pid);
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
      slurm_spank_log("opening %s failed", path);
    }
    if (setns(fd, CLONE_NEWUSER) != 0) {
      slurm_spank_log("setns newuser failed: %s", strerror(errno));
      return ESPANK_ERROR;
    }
  }

  // fails with operation not permitted
  {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/ns/mnt", shm->ns_pid);
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
      slurm_spank_log("opening %s failed", path);
    }
    if (setns(fd, CLONE_NEWNS) != 0) {
      slurm_spank_log("setns newns failed: %s", strerror(errno));
      return ESPANK_ERROR;
    }
  }
  return ESPANK_SUCCESS;
}

/// fork + exec('squashfs-mount-rootless') + attempt to enter namespaces
int calls_sqfs_mount_rootless() {
  if (pthread_mutex_lock(&shm->mutex) == EOWNERDEAD) {
    pthread_mutex_consistent(&shm->mutex);
    shm->ns_pid = -1;
    throw std::runtime_error("mutex lock failed");
  }

  shm->init_tasks++;

  // only the process which acquires the lock first starts fuse
  if (shm->init_tasks.load() == 1) {
    int pid = fork();
    if (pid == 0) {

      if (prctl(PR_SET_PDEATHSIG, SIG_IGN) == -1) {
        slurm_spank_log("disconneting forked process didn't work: %s",
                        strerror(errno));
      }
      // signal(SIGSTOP, SIG_IGN);
      // signal(SIGHUP, SIG_IGN);
      // int _pid = getpid();
      // slurm_spank_log("pid of squashfs-mount-rootless  %d", _pid);

      // slurm_spank_log("hint: nsenter -t %d -m -S 1000 -G 1000 sh -c 'ls "
      //                 "/user-environment'  to check /user-environment is "
      //                 "mounted correctly",
      //                 _pid);
      execlp("squashfs-mount-rootless", "squashfs-mount-rootless",
             args.file->c_str(), "/user-environment", "sh", "-c",
             "kill -STOP $$; exit 0", NULL);

      // execlp("sleep", "sleep", "100000", NULL);
      slurm_spank_log("execl failed");
      return ESPANK_ERROR;
    } else if (pid < 0) {
      slurm_spank_log("squashfs-mount-rootless: fork error");
      _exit(EXIT_FAILURE);
    } else {
      slurm_spank_log("fuse_pid=%d", pid);
      shm->ns_pid = pid;
    }
    // wait for the mount process to have started (=it suspendend itself)
    // slurm_spank_log("DEBUG: waitpid %d", shm->ns_pid);
    siginfo_t sig_info;
    int waitpid = shm->ns_pid;
    int ret = waitid(P_PID, waitpid, &sig_info, WSTOPPED);
    if (ret < 0) {
      slurm_spank_log("failed to start squashfs-mount-rootless: %s (pid=%d)",
                      strerror(errno), waitpid);
    }
  }

  pthread_mutex_unlock(&shm->mutex);

  shm->started_tasks++;

  return enter_user_and_mount_namespace(shm->ns_pid);
}

int slurm_spank_init(spank_t sp, int ac, char **av) {
  print_s_ctx(__func__, getpid());
  static spank_option mount_point_arg{
      (char *)"uenv-mount-point",
      (char *)"<path>",
      (char *)"path whence the environment is mounted: default "
              "/user-environment",
      1, // requires an argument
      0, // plugin specific value to pass to the callback (unnused)
      [](int val, const char *optarg, int remote) -> int {
        slurm_info("uenv-mount-point: val:%d optarg:%s remote:%d", val, optarg,
                   remote);
        if (!optarg) { // is this required if the has_arg flag == 1?
          return ESPANK_BAD_ARG;
        }
        // todo: parse string to validate that the path exists
        // todo: parse string to validate that it is a valid and allowed path
        args.mount_point = optarg;
        return ESPANK_SUCCESS;
      }};

  static spank_option file_arg{
      (char *)"uenv-mount-file",
      (char *)"<path>",
      (char *)"the squashfs file with the image to mount:",
      1, // requires an argument
      0, // plugin specific value to pass to the callback (unnused)
      [](int val, const char *optarg, int remote) -> int {
        slurm_info("uenv-mount-point: val:%d optarg:%s remote:%d", val, optarg,
                   remote);
        if (!optarg) { // is this required if the has_arg flag == 1?
          return ESPANK_BAD_ARG;
        }
        // todo: parse string to validate that the file exists
        // todo: parse string to validate that it is a valid and allowed path
        args.file = std::string{optarg};
        return ESPANK_SUCCESS;
      }};

  static spank_option prolog_arg{
      (char *)"uenv-skip-prologue",
      (char *)"none",
      (char *)"ignore any environment prologue script",
      0, // takes an argument
      0, // plugin specific value to pass to the callback (unnused)
      [](int val, const char *optarg, int remote) -> int {
        slurm_info("uenv-mount-point: val:%d optarg:%s remote:%d", val, optarg,
                   remote);
        if (optarg) { // is this required if the has_arg flag == 0?
          return ESPANK_BAD_ARG;
        }
        args.run_prologue = false;
        return ESPANK_SUCCESS;
      }};

  for (auto arg : {&mount_point_arg, &file_arg, &prolog_arg}) {
    if (auto status = spank_option_register(sp, arg)) {
      return status;
    }
  }

  return ESPANK_SUCCESS;
}

int slurm_spank_init_post_opt(spank_t sp, int ac, char **av) {
  print_s_ctx(__func__, getpid());
  // test_unshare_user();
  return ESPANK_SUCCESS;
}

int slurm_spank_user_init(spank_t sp, int ac, char **av) {
  // atexit([]() { slurm_spank_log("exit_hook: %s", "user_init"); });
  print_s_ctx(__func__, getpid());

  shm = shm_init();

  // if (args.file) {
  //   return calls_sqfs_mount_rootless();
  // }

  return ESPANK_SUCCESS;
}

int slurm_spank_local_user_init(spank_t sp, int ac, char **av) {
  // // atexit([]() { slurm_spank_log("exit_hook: %s", "local_user_init"); });
  // print_s_ctx(__func__, getpid());
  return ESPANK_SUCCESS;
}

int slurm_spank_task_init(spank_t sp, int ac, char **av) {
  // atexit([]() { slurm_spank_log("exit_hook: %s", "task_init"); });
  print_s_ctx(__func__, getpid());

  if (args.file) {
    return calls_sqfs_mount_rootless();
  }

  return ESPANK_SUCCESS;
}

int slurm_spank_task_init_privileged(spank_t sp, int ac, char **av) {
  // print_s_ctx(__func__, getpid());
  // return do_mount(sp, args.mount_point.c_str(), args.file->c_str());
  return ESPANK_SUCCESS;
}

int slurm_spank_task_exit(spank_t sp, int ac, char **av) {
  int task_count;
  if (spank_get_item(sp, S_JOB_LOCAL_TASK_COUNT, &task_count) !=
      ESPANK_SUCCESS) {
    throw std::runtime_error("couldn't get task count");
  }

  if (pthread_mutex_lock(&shm->mutex) == EOWNERDEAD) {
    pthread_mutex_consistent(&shm->mutex);
    throw std::runtime_error("mutex lock failed");
  }

  shm->completed_tasks++;

  slurm_spank_log("completed tasks: %d of %d (started %d)",
                  shm->completed_tasks.load(), task_count,
                  shm->started_tasks.load());

  // the last task sends sigcont and lets squashfs-mount-rootless terminate
  if (shm->completed_tasks.load() == task_count) {
    slurm_spank_log("sending kill -SIGCONT %d", shm->ns_pid);
    int ret = kill(shm->ns_pid, SIGCONT);
    if (ret < 0) {
      slurm_spank_log("couldn't send SIGCONT to squashfs-mount-rootless: %s",
                      strerror(errno));
    }
  }
  pthread_mutex_unlock(&shm->mutex);

  return ESPANK_SUCCESS;
}

} // namespace impl
