//por alguma razão o loop muito alto no amd e com -O0 fica exato as médias
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#define CALIBRATE_REPS 1000
static inline uint64_t rdtsc()
{
	uint64_t lo, hi;
	//__asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi) );
	__asm__ __volatile__("rdtscp; " "shl $32,%%rdx; "  "or %%rdx,%%rax" : "=a"(lo)  : : "%rcx", "%rdx"); 
	//return( lo | (hi << 32) );
	return lo ;
}

static inline void busy(uint64_t count)
{
	uint64_t aux, before;
	before = rdtsc();
	while(rdtsc() - before < count)
		;
}

void calibrate_busy(uint32_t *single, uint32_t *loop)
{
	uint32_t start, end, acc = 0;
	uint64_t reps = 0;
	float single_f, loop_f;
	for (int i = 0; i < CALIBRATE_REPS; i++) {
		busy(20000);
		__asm__ __volatile__ ("" ::: "memory");
		start =  rdtsc();
		end = rdtsc();
		__asm__ __volatile__ ("" ::: "memory");
		if (end - start < 50) {
			acc += end - start;
			reps++;
		}
	}
	single_f = (float)acc/reps;
	*single = (int)(single_f + 0.5);
	acc = 0; reps = 0;

	for (int i = 0; i < CALIBRATE_REPS; i++) {
		busy(20000);
		__asm__ __volatile__ ("" ::: "memory");
		start =  rdtsc();
		busy(1);
		end = rdtsc();
		__asm__ __volatile__ ("" ::: "memory");
		if (end - start < 300) {
			acc += end - start;
			reps++;
		}
	}
	loop_f = (float)acc/reps - single_f;
	*loop = (int)(loop_f + 0.5);
	printf("\nsingle_f: %f, loop_f: %f", single_f, loop_f);
}

int main (int ac, char **av){
	uint32_t start, end;
	int reps = atoi(av[1]);
	uint32_t cyc_rdtsc, cyc_loop;

	calibrate_busy(&cyc_rdtsc, &cyc_loop);
	printf("\nsingle: %d, loop: %d", cyc_rdtsc, cyc_loop);

	
	if(reps < cyc_loop)
		reps = cyc_loop+1;
	else if(reps < 2*cyc_loop)
		reps = 2*cyc_loop+1;
	start =  rdtsc();
	busy(reps-cyc_loop-1);
	end = rdtsc();
	printf("\nwaited for %d cycles", end-start-cyc_rdtsc);
	printf("\n");
	return end-start;
}
