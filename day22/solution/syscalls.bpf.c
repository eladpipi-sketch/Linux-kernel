// SPDX-License-Identifier: GPL-2.0
/*
 * Day 22 exercise: a second program, on the exit of write(), adds up the
 * bytes each program wrote. Its argument's layout is copied from
 * /sys/kernel/tracing/events/syscalls/sys_exit_write/format.
 */
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

#define TASK_COMM_LEN 16

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(max_entries, 1024);
	__type(key, char[TASK_COMM_LEN]);	/* program name, as in task_struct */
	__type(value, __u64);			/* its system calls so far */
} counts SEC(".maps");

struct {
	__uint(type, BPF_MAP_TYPE_HASH);
	__uint(max_entries, 1024);
	__type(key, char[TASK_COMM_LEN]);	/* program name */
	__type(value, __u64);			/* bytes it wrote so far */
} written SEC(".maps");

struct sys_exit_args {
	__u64 common;		/* offsets 0-7: common_type, flags, preempt_count, pid */
	int syscall_nr;		/* offset 8 */
	long ret;		/* offset 16: what write() returned */
};

SEC("tracepoint/raw_syscalls/sys_enter")
int count_syscall(void *ctx)
{
	char comm[TASK_COMM_LEN] = {};
	__u64 one = 1, *count;

	bpf_get_current_comm(comm, sizeof(comm));
	count = bpf_map_lookup_elem(&counts, comm);
	if (count)
		__sync_fetch_and_add(count, 1);	/* other CPUs may count it too */
	else
		bpf_map_update_elem(&counts, comm, &one, BPF_NOEXIST);
	return 0;
}

SEC("tracepoint/syscalls/sys_exit_write")
int count_written(struct sys_exit_args *args)
{
	char comm[TASK_COMM_LEN] = {};
	__u64 bytes, *total;

	if (args->ret <= 0)		/* an error, or nothing written */
		return 0;
	bytes = args->ret;
	bpf_get_current_comm(comm, sizeof(comm));
	total = bpf_map_lookup_elem(&written, comm);
	if (total)
		__sync_fetch_and_add(total, bytes);
	else
		bpf_map_update_elem(&written, comm, &bytes, BPF_NOEXIST);
	return 0;
}

char LICENSE[] SEC("license") = "GPL";
