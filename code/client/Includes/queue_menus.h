#ifndef QUEUE_MENUS_H
#define QUEUE_MENUS_H

typedef enum queue_op{Q_READ_FROM,Q_LOOK,Q_GET_TIME,Q_READ_TO,Q_CLEAN,Q_PRINT,Q_NA}queue_op;
typedef enum queue_look_op{Q_IS_FULL,Q_IS_EMPTY,Q_IS_ALMOST_FULL,Q_IS_ALMOST_EMPTY,Q_GET_PCT,Q_LOOK_NA}queue_look_op;

typedef struct q_op{

	queue_op main;
	queue_look_op look;

}q_op;

int perform_queue_op(chunk_queue* que,uint8_t* buff_if_insert,q_op op);
#endif
