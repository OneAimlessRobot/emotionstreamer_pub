#include "../../Includes/preprocessor.h"
#include <alsa/asoundlib.h>
#include <pulse/error.h>
#include <pulse/simple.h>
#include "../../extra_funcs/Includes/sockio.h"
#include "../../extra_funcs/Includes/auxfuncs.h"
#include "../Includes/configs.h"
#include "../Includes/ripped_code.h"
#include "../Includes/chunk_queue.h"
#include "../Includes/queue_arithmetic.h"




static uint32_t pct_of_num(uint32_t pct,uint32_t num){

	double dnum= (double)num;
	double dpct= (double)pct;

	return round((dnum*dpct)/100.0);


}
static uint32_t pct_frac_of_nums(uint32_t num1,uint32_t num2){

	double dnum1= (double)num1;
	double dnum2= (double)num2;

	return round((dnum1/dnum2)*100.0);


}
void zero_chk_cache(chunk_queue* que){


        memset(que->chunk_buff,0,que->total_size);


}

uint32_t getQueueBufferedTime(chunk_queue* que){

	chunk_size_helper helper=(chunk_size_helper){cfg_freq,CHANNELS,SIZE*8,que->n_occupied*que->chunk_size};
	return getChunkTimeMilliseconds(&helper);


}
uint32_t circular_int_inc(uint32_t size,uint32_t a){


	return ((a+1)%size);


}

uint32_t circular_int_dec(uint32_t size,uint32_t a){

	return (a>0) ? a-1:size-1;

}

int que_is_empty(chunk_queue* que){

	return !(que->n_occupied);



}
int que_is_almost_empty(chunk_queue* que){

	return pct_frac_of_nums(que->n_occupied,que->max_occupied)<=(cfg_cache_almost_empty_pct);



}
int que_is_full(chunk_queue* que){

	return ((que->n_occupied)==(que->max_occupied));



}
int que_is_almost_full(chunk_queue* que){

	return pct_frac_of_nums(que->n_occupied,que->max_occupied)>=(cfg_cache_almost_full_pct);



}
int que_pct_full(chunk_queue* que){

	return round(((double)que->n_occupied/(double)que->max_occupied)*100.0);




}
