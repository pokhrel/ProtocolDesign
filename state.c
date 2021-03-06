#include"state.h"
void initState(struct State * state){
	memset(state,0,sizeof(struct State));
	srand(time(NULL));	
	state->seq = rand();
	state->ack = state->seq;
	state->ackreq = false;
	state->rack = 0;
	state->fd = 0;
	state->sentup = 0;
	state->sentdown = 0;
	state->status = RC;
	state->window = 10000;
	state->clean = false;
	INIT_LIST_HEAD(&(state->out.list));
	INIT_LIST_HEAD(&(state->in.list));
	INIT_LIST_HEAD(&(state->racks.list));
}

struct State * findState_addr(struct State * state,struct sockaddr * addr){
	struct list_head * pos;
	struct State * tmp;
	list_for_each(pos,&(state->list)){
			tmp = list_entry(pos,struct State,list);
			if(memcmp(addr,&(tmp->addr),sizeof(struct sockaddr)) == 0)
				return tmp;
	}
	return NULL;
}

struct State * findState_fd(struct State * state,int fd){
	struct list_head * pos;
	struct State * tmp;
	list_for_each(pos,&(state->list)){
			tmp = list_entry(pos,struct State,list);
			if(tmp->fd == fd)
				return tmp;
	}
	return NULL;
}

unsigned short getDouble(unsigned short s){
	if(s < 32768)
		return (s+32768);
	else
		return (s-32768);
}

void ackThis(struct State * state,unsigned short a,unsigned char ackbit,unsigned char cackbit){
	struct Queue * tmp;
	struct list_head * pos,*q;
	if(ackbit && cackbit){
		if(state->ack > 32768){
			if(a < state->ack && a > (state->ack-32768))
				return;
		} 
		else if(state->ack <= 32768){
			if( a < state->ack || a > (32767+state->ack) )
				return;
		}
		while(state->ack <= a){
			list_for_each_safe(pos,q,&(state->out.list)){
				tmp = list_entry(pos,struct Queue,list);
				if(tmp->seq == state->ack){
					(state->window) += 2*(tmp->size);
					list_del(pos);
					free(tmp);
					break;
				}
			}
			state->ack = (state->ack == 65535)?1:(state->ack)+1;
		}
	}
	else if(ackbit){
		list_for_each_safe(pos,q,&(state->out.list)){
			tmp = list_entry(pos,struct Queue,list);
			if(tmp->seq == a){
				(state->window) += (tmp->size);
				list_del(pos);
				free(tmp);
				break;	
			}
		}	
	}
}	

int ackThat(struct State * state,unsigned short a){
	struct Int * tmp;
	struct list_head *pos,*q;
	unsigned short t;
	bool cnt;
	t = (state->rack==65535)?1:(state->rack)+1;
	if(t == a){
		state->rack = t;
		while(1){
			t = (state->rack==65535)?1:(state->rack)+1;
			cnt = false;
			list_for_each_safe(pos,q,&(state->racks.list)){
				tmp = list_entry(pos,struct Int,list);
				if(tmp->seq == t){
					state->rack = t;
					cnt = true;
					break;				
				}				
			}
			if(!cnt)
				break;
		}		
		state->ackreq = true;
		return 1;	
	}

	else {
		list_for_each(pos,&((state->racks).list)){
			tmp = list_entry(pos,struct Int,list);
			if(tmp->seq == a)
				return -1;
		}
		tmp = (struct Int *)malloc(sizeof(struct Int));
		tmp->seq = a;
		list_add_tail(&(tmp->list),&((state->racks).list));
		return 0;
	}
}

void addOutPacketToState(struct State * state,unsigned char * packet,unsigned short seq,
							int size,unsigned char frag){
	struct Queue * tmp;
	tmp = (struct Queue *)malloc(sizeof(struct Queue));
	memset(tmp,0,sizeof(struct Queue));
	memcpy(tmp->buffer,packet,size);
	tmp->seq = seq;
	tmp->frag = frag;
 	tmp->size = (unsigned short)size;
	tmp->sent = false;
	gettimeofday(&(tmp->st),NULL);
	list_add_tail(&(tmp->list),&((state->out).list));		
}

void addInPacketToState(struct State * state,unsigned char * packet,unsigned short seq,
							int size,unsigned char frag){
	struct Queue * tmp;
	tmp = (struct Queue *)malloc(sizeof(struct Queue));
	memset(tmp,0,sizeof(struct Queue));
	memcpy(tmp->buffer,packet,size);
	tmp->seq = seq;
	tmp->size = (unsigned short)size;
	tmp->frag = frag;
	list_add_tail(&(tmp->list),&((state->in).list));	
}

bool checktime(struct timeval *pt,struct timeval *ct,suseconds_t gap){
	struct timeval nt;	
	gettimeofday(&nt,NULL);
	if( ((nt.tv_sec) - (pt->tv_sec)) > 0)
		return true;
	if(gap == 0)
		return false;
	if( (nt.tv_sec) == (pt->tv_sec) )
		if( ((nt.tv_usec) - (pt->tv_usec)) > gap)
			return true;			
	return false;
}	 


void releaseState(struct State * state){
	struct list_head *pos,*idx;
	struct Queue * queue;
	struct Int * i;

	list_for_each_safe(pos,idx,&(state->in.list)){
		queue = list_entry(pos,struct Queue,list);
		list_del(pos);
		free(queue);
	}
	
	list_for_each_safe(pos,idx,&(state->out.list)){
		queue = list_entry(pos,struct Queue,list);
		list_del(pos);
		free(queue);	
	}

	list_for_each_safe(pos,idx,&(state->racks.list)){
		i = list_entry(pos,struct Int,list);
		list_del(pos);
		free(i);	
	}
}


void printState(struct State * state){
	printf("%d %d %d %d %d\n",state->seq,state->ack,state->rack,state->sentup,state->sentdown);
}
