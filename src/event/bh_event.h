#ifndef __BH_EVENT_H
#define __BH_EVENT_H

#include <time.h>
#include <sys/time.h>

#define FD_NONE 0
#define FD_READABLE 1
#define FD_WRITABLE 2

#define FILE_EVENTS 1
#define TIME_EVENTS 2
#define ALL_EVENTS 3
#define DONT_WAIT 4

//declare type : event loop struct
typedef struct bh_event_loop_s bh_event_loop_t;
//declare type : file event struct
typedef struct bh_file_event_s bh_file_event_t;
//declare type : fired event struct
typedef struct bh_fired_event_s bh_fired_event_t;
//declare type : time event struct
typedef struct bh_time_event_s bh_time_event_t;
//declare function type
typedef void bh_file_process_func(bh_event_loop_t *event_loop, int fd, void *clientData, int mask);
//declare function type
typedef void bh_time_process_func(bh_event_loop_t *event_loop, long long id, void *clientData);


//define struct event loop
struct bh_event_loop_s {
	int maxfd;
	int setsize;
	bh_file_event_t *register_events;
	bh_fired_event_t *fired_events; 
	bh_time_event_t *time_event_list_head;
	time_t last_time;
	int stop;
	void *api_data;
	long long next_time_event_id;
};

struct bh_file_event_s {
	int mask;
	bh_file_process_func *read_proc;
	bh_file_process_func *write_proc;
	void *clientData;
};

struct bh_fired_event_s {
	int mask;
	int fd;
};

struct bh_time_event_s {
	long long id;
	long when_sec;
	long when_milli_sec;
	bh_time_process_func *time_proc;
	bh_time_event_t *next_time_event;
	void *client_data;
};


//function protypes of dealing  event loop
bh_event_loop_t* bh_create_event_loop(int setsize);
void bh_delete_event_loop(bh_event_loop_t *event_loop);
void bh_stop_event_loop(bh_event_loop_t *event_loop);

//function protypes of deal file event
int bh_create_file_event(bh_event_loop_t *event_loop, int fd, bh_file_process_func *proc,
				int mask, void *clientData);
bh_file_event_t* bh_get_file_event(bh_event_loop_t *event_loop, int fd);
void bh_delete_file_event(bh_event_loop_t *event_loop, int fd, int mask);

//function protypes of dealing time event
long long bh_create_time_event(bh_event_loop_t *event_loop, long long milliseconds, 
				bh_time_process_func *proc, void *client_data);
void bh_delete_time_event(bh_event_loop_t *event_loop, long long id);


void bh_main(bh_event_loop_t *event_loop);
void bh_wait(int fd, int mask, long long milliseconds);

#endif
