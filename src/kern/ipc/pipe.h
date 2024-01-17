#ifndef	__KERN_IPC_PIPE_H__
#define	__KERN_IPC_PIPE_H__


/*
 * A pipe is a one-way data path between a single consumer task
 * and one or more provider tasks.
 *
 * These pipes are primarily designed for kernel tasks - there's no
 * specific lookup table, and they are currently not refcounted.
 * They aren't automatically cleared up when a kernel task exits
 * either at the present moment, so be careful.
 *
 * They may be temporarily (!) used for say, the console IO task
 * if and when I write that - so I can experiment with console IO
 * being fed through a pipe, rather than the syscall doing direct
 * UART IO.
 *
 * The consumer task owns the pipe - they create it, set the
 * notification signal for more data to read, and they can close it.
 * There is (currently) no refcounting on the pipe in question,
 * so if the consumer closes the pipe, the senders will likely crash
 * the kernel.
 *
 * Senders can queue data into the pipe, and it will either be queued
 * or not be queued.  Notably (since there's no concept of a wait queue
 * in wtf-os!) there's only a signal for the receiver, and not for
 * multiple senders.  That would require tracking the sender tasks,
 * and refcounting the pipe - and I don't want to do that JUST yet.
 */

#include <kern/core/task_defs.h>
#include <kern/core/signal.h>

typedef enum {
	KERN_IPC_PIPE_STATE_NONE = 0,
	KERN_IPC_PIPE_STATE_OPEN = 1,
	KERN_IPC_PIPE_STATE_SHUTDOWN = 2,
	KERN_IPC_PIPE_STATE_CLOSED = 3,
} kern_ipc_pipe_state_t;

typedef struct kern_ipc_pipe kern_ipc_pipe_t;
typedef struct kern_ipc_msg kern_ipc_msg_t;

struct kern_ipc_pipe {
	/* owner / task to wake up upon data being pushed in */
	kern_task_id_t owner_task;
	kern_ipc_pipe_state_t state;

	char *buf_ptr;
	uint32_t buf_size;
	uint32_t buf_offset;
	uint32_t max_msg_size;
};

// XXX TODO: rename to kern_ipc_pipe_msg? */
struct kern_ipc_msg {
	uint16_t len;
	uint16_t id;
	char msg[0];
};

extern	void kern_ipc_pipe_init(void);

extern	void kern_ipc_pipe_setup(kern_ipc_pipe_t *pipe, char *buf, int len,
	    uint32_t max_msg_size);
extern	void kern_ipc_pipe_close(kern_ipc_pipe_t *pipe);
extern	void kern_ipc_pipe_shutdown(kern_ipc_pipe_t *pipe);
extern	kern_error_t kern_ipc_pipe_set_owner(kern_ipc_pipe_t *pipe,
	    kern_task_id_t task);
extern	kern_error_t kern_ipc_pipe_queue(kern_ipc_pipe_t *pipe,
	    const kern_ipc_msg_t *msg);
extern	kern_error_t kern_ipc_pipe_dequeue(kern_ipc_pipe_t *pipe,
	    kern_ipc_msg_t *msg);
extern	kern_error_t kern_ipc_pipe_consume(kern_ipc_pipe_t *pipe,
	    kern_ipc_msg_t *msg);
extern	kern_error_t kern_ipc_pipe_flush(kern_ipc_pipe_t *pipe, uint32_t *num);

/* ipc message routines */
extern	uint16_t kern_ipc_msg_payload_len(const kern_ipc_msg_t *msg);
extern	const char * kern_ipc_msg_payload_buf(const kern_ipc_msg_t *msg);

#endif /* __KERN_IPC_PIPE_H__ */
