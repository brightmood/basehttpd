#include "bh_event.h"
#include "common/bh_malloc.h"

#ifdef HAVE_EVPORT
#include "bh_evport.c"
#else
	#ifdef HAVE_EPOLL
	#include "bh_epoll.c"
	#else
		#ifdef HAVE_KQUEUE
		#include "bh_kqueue.c"
		#else
			#ifdef HAVE_POLL
			#include "bh_poll.c"
			#else
			#include "bh_select.c"
			#endif
		#endif
	#endif
#endif

bh_event_loop_t* bh_create_event_loop(int setsize) {
	
	bh_event_loop_t* event_loop = 
			(bh_event_loop_t*)bh_malloc(sizeof(bh_event_loop_t));

	if(event_loop == NULL)
		goto err;

	event_loop->register_events = 
			(bh_file_event_t*)bh_malloc(sizeof(bh_file_event_t)*setsize);
	event_loop->fired_events = 
			(bh_fired_event_t*)bh_malloc(sizeof(bh_fired_event_t)*setsize);
	if(event_loop->fired_events == NULL 
					|| event_loop->register_events == NULL)
		goto err;

	event_loop->setsize = setsize;
	event_loop->time_event_list_head = NULL;
	event_loop->last_time = time(NULL);
	event_loop->maxfd = -1;
	event_loop->stop = 0;
	event_loop->next_time_event_id = 0;
	if(bh_api_create(event_loop) == -1)
		goto err;
	for(int i = 0; i < setsize; i++) {
		event_loop->register_events[i].mask = FD_NONE; 
	}
	return event_loop;

err:
	if(event_loop) {
		bh_free(event_loop->fired_events);
		bh_free(event_loop->register_events);
		bh_free(event_loop);
	}
	return NULL;
}

void bh_delete_event_loop(bh_event_loop_t *event_loop) {
	
	bh_api_free(event_loop);
	bh_free(event_loop->register_events);
	bh_free(event_loop->fired_events);
	bh_free(event_loop);	
}

void bh_stop_event_loop(bh_event_loop_t *event_loop) {
	event_loop->stop = -1;
}

int bh_create_file_event(bh_event_loop_t *event_loop, int fd, 
				bh_file_process_func *proc, int mask, void *clientData) {
	
	if(fd >= event_loop->setsize) 
		return -1;
	
	bh_file_event_t * file_event = &event_loop->register_events[fd];
	if(bh_api_add_event(event_loop, fd, mask) == -1) 
		return -1;
	file_event->mask |= mask;
	if(mask & FD_READABLE) file_event->read_proc = proc;
	if(mask & FD_WRITABLE) file_event->write_proc = proc;
	file_event->clientData = clientData;
	if(fd > event_loop->maxfd) 
		event_loop->maxfd = fd;
	return 0;
}

bh_file_event_t* bh_get_file_event(bh_event_loop_t *event_loop, int fd) {
	if(fd >= event_loop->setsize)
		return NULL;
	return &event_loop->register_events[fd];
}

void bh_delete_file_event(bh_event_loop_t *event_loop, int fd, int mask) {
	if(fd >= event_loop->setsize) 
		return;
	bh_file_event_t file_event = event_loop->register_events[fd];
	if(file_event.mask == FD_NONE) 
		return;
	bh_api_delete_event(event_loop, fd, mask);
	file_event.mask = file_event.mask & (~mask);
	
	//update maxfd if the deleted fd is the current max fd
	if(fd == event_loop->maxfd && file_event.mask == FD_NONE) {
		int j;
		for(j = event_loop->maxfd-1; j >= 0; j--) {
			if(event_loop->register_events[j].mask != FD_NONE)
				break;
		}
		event_loop->maxfd = j;
	}
}

static void bh_get_time(long *sec, long *mill_sec) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	*sec = tv.tv_sec;
	*mill_sec = tv.tv_usec/1000;
}

static void bh_add_millisecond_to_now(long long milliseconds, long *when_sec, long *when_milli_sec) {

	long curr_sec, curr_milli_sec, sec, milli_sec;
	bh_get_time(&curr_sec, &curr_milli_sec);
	sec = curr_sec + milliseconds/1000;
	milli_sec = curr_milli_sec + milliseconds%1000;
	if(milli_sec >= 1000) {
		sec +=1;
		milli_sec -=1000;
	}
	*when_sec = sec;
	*when_milli_sec = milli_sec;
}

long long bh_create_time_event(bh_event_loop_t *event_loop, long long milliseconds,
				bh_time_process_func *proc, void *client_data) {
	
	bh_time_event_t *te = bh_malloc(sizeof(bh_time_event_t));
	if(te == NULL)
		return -1;
	long long id = event_loop->next_time_event_id++;
	te->id = id;
	bh_add_millisecond_to_now(milliseconds, &te->when_sec, &te->when_milli_sec);
	te->client_data = client_data;
	te->time_proc = proc;
	te->next_time_event = event_loop->time_event_list_head;
	event_loop->time_event_list_head = te;
	return id;	
}

static bh_time_event_t* search_nearest_time_event(bh_event_loop_t *event_loop) {
	
	bh_time_event_t *temp = event_loop->time_event_list_head;
	if(temp == NULL)
		return NULL;
	long sec = temp->when_sec;
	long milli_sec = temp->when_milli_sec;
	bh_time_event_t *next, *nearest = temp;
	while((next = temp->next_time_event) != NULL) {
		if(next->when_sec < sec || 
						(next->when_sec == sec && next->when_milli_sec < milli_sec)) {
			sec = next->when_sec;
			milli_sec = next->when_milli_sec;
			nearest = next;
		}
		temp = next;
	}
	return nearest;
}

int bh_process_events(bh_event_loop_t *event_loop, int flags) {
	
}

void bh_main(bh_event_loop_t *event_loop) {
	event_loop->stop = 0;
	while(!event_loop->stop) {
		bh_process_events(event_loop,ALL_EVENTS); 
	}
}

