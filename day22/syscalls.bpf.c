// SPDX-License-Identifier: GPL-2.0
/*
 * syscalls.bpf.c: an eBPF program. clang compiles it to BPF bytecode, the
 * kernel's verifier checks it, and then it runs inside the kernel on every
 * system call entry. It counts each program's calls in a map, a hash table
 * that the kernel keeps and user space reads.
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

char LICENSE[] SEC("license") = "GPL";
