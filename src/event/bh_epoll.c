#include <sys/epoll.h>

typedef struct bh_api_state_s bh_api_state_t;

struct bh_api_state_s {
	int epoll_fd;
	struct epoll_event *events;
};

int bh_api_create(bh_event_loop_t *event_loop) {
	
	bh_api_state_t *api_state = (bh_api_state_t*)bh_malloc(sizeof(bh_api_state_t));
	if(api_state == NULL)
		return -1;
	
	api_state->events = (struct epoll_event*)bh_malloc(
					sizeof(struct epoll_event)*event_loop->setsize);
	if(api_state->events == NULL) {
		bh_free(api_state);	
		return -1;
	}

	api_state->epoll_fd = epoll_create(1024);
	if(api_state->epoll_fd == -1){
		bh_free(api_state->events);
		bh_free(api_state);
		return -1;
	}
	
	event_loop->api_data = (void*)api_state;
	return 0;
}

void bh_api_free(bh_event_loop_t *event_loop) {
	if(event_loop->api_data) {
		bh_free(event_loop->api_data->events);
	}
	bh_free(event_loop->api_data);
}

int bh_api_add_event(bh_event_loop_t *event_loop, int fd, int mask) {
	bh_api_state_t *api_state = event_loop->api_data;
	struct epoll_event ee;
	int op = event_loop->register_events[fd].mask == FD_NONE 
			? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

	ee.events = 0;
	mask |= event_loop->register_events[fd].mask;
	if(mask & FD_READABLE) 
		ee.events = EPOLLIN;
	if(mask & FD_WRITABLE)
		ee.events = EPOLLOUT;
	ee.data.u64 = 0;
	ee.data.fd = fd;
	if(epoll_ctl(api_state->epoll_fd, op, fd, &ee) == -1) 
		return -1;
	return 0;
}

void bh_api_delete_event(bh_event_loop_t *event_loop, int fd, int delmask) {
	bh_api_state_t *api_state = event_loop->api_data;	
	
	struct epoll_event ee;
	int mask = event_loop->register_events[fd].mask & (~delmask);

	ee.events = 0;
	if(mask & FD_READABLE) ee.events = EPOLLIN;
	if(mask & FD_WRITABLE) ee.events = EPOLLOUT;
	ee.data.u64 = 0;
	ee.data.fd = fd;

	if(mask == FD_NONE) 
		epoll_ctl(api_state->epoll_fd, EPOLL_CTL_DEL, fd, &ee);
	else
		epoll_ctl(api_state->epoll_fd, EPOLL_CTL_MOD, fd, &ee);
}
