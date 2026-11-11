# Day 7 exercise, solved: run these in the VM (sh /lab/day07/solution/open_probes.sh).
# An entry probe logs each file name; a return probe logs what the kernel
# returned: a file descriptor, or a negative errno. do_sys_openat2() returns
# int, so the probe reads the return register as a signed 32-bit value.
cd /sys/kernel/tracing
echo > trace
echo 'p:lab_open do_sys_openat2 path=+0(%si):ustring' > kprobe_events
echo 'r:lab_open_ret do_sys_openat2 ret=$retval:s32' >> kprobe_events
echo 1 > events/kprobes/enable
cat /lab/day07/Makefile > /dev/null
cat /no/such/file 2> /dev/null
echo 0 > events/kprobes/enable
grep -A1 'lab_open:.*path="/\(lab/day07\|no/such\)' trace
echo > kprobe_events
