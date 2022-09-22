// Unshare filesystem attributes, so that this process no longer shares its root
// directory (chroot), current directory (chdir), or umask (umask) attributes
// with any other process.
unshare(CLONE_NEWNS)

//int mount(
//  source = NULL,  // ignored because of MS_SLAVE mount flag
//  target = "/",   // root directory
//  filesystemtype = NULL,
//                  // ignored because of MS_SLAVE mount flag
//  mountflags = MS_SLAVE|MS_REC,
//                  // 
//  data = NULL)    // ignored because of MS_SLAVE mount flag
//
//  this ensures that that any changes that we make to the filesystem,
//  by starting at root "/", are not propogated to the parent or sibling
//  processes, but are visible to all child processes.
unshare(CLONE_NEWNS)
mount(NULL, "/", NULL, MS_SLAVE | MS_REC, NULL)

// Set real user to root before creating the mount context, otherwise it fails.
setreuid(0, 0)

cxt = mnt_new_context();

// disable mtab
mnt_context_disable_mtab(cxt, 1)

//  set fstype to squashfs
mnt_context_set_fstype(cxt, "squashfs")

// set mount options
// TODO: define each of these
char const *mount_options = "loop,nosuid,nodev,ro";
mnt_context_append_options(cxt, mount_options)

// set source and target (squashfs file and mountpoint respectively)
mnt_context_set_source(cxt, squashfs_file)
mnt_context_set_target(cxt, mountpoint)

//  perform the mount
mnt_context_mount(cxt)

//  reset user to normal (not root)
setresuid(uid, uid, uid)

//  exit_with_error("PR_SET_NO_NEW_PRIVS failed\n");
// promise not to grant priveleges to do anything that could not
// have been done without the excve call.
prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)

