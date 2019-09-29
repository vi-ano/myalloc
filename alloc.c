#include  <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <arpa/inet.h>
#include <errno.h>

void print_bytes(char * ty, char * val, unsigned char * bytes, size_t num_bytes) {
	printf("(%*s) %*s = [ ", 15, ty, 16, val);
	for (size_t i = 0; i < num_bytes; i++) {
		printf("%*u ", 3, bytes[i]);
	}
	printf("]\n");
}

#define SHOW(T,V) do { T x = V; print_bytes(#T, #V, (unsigned char*) &x, sizeof(x)); } while(0)

void printbincharpad(char c)
{
	for (int i = 7; i >= 0; --i) {
		putchar( (c & (1 << i)) ? '1' : '0' );
	}
	putchar('\n');
}

#define AVAILABLE_MEMORY 16*(1<<20)     // 16 megabytes  == 16*(1024*1024)  it doesn't matter how to write it.. 
// You can find this type of writing for powers of two : (1<<N) == 2^N
//  00000000000000000000000000000001 = 1    
//  00000000000000000000000000000010 = 2    = 1<<1  == 2<<0
//  00000000000000000000000000000100 = 4    = 1<<2  == 2<<1 == 2^2
//  00000000000000000000000000001000 = 8    = 1<<3  == 2<<2 == 4<<1 = ....   == 2^3
//  00000000000000000000000000010000 = 16
//  00000000000000000000000000100000 = 32
//  00000000000000000000000001000000 = 64
//  00000000000000000000000010000000 = 128
//  00000000000000000000000100000000 = 256
//  ....
//  00000000000100000000000000000000 = 1<<20 == 2^20 = 1048576

static char _mem[AVAILABLE_MEMORY];
void *mem_head = &_mem[0];

struct chunk{
	int memory_size;
	struct chunk* next;
	struct chunk* prev;
}__attribute__((packed));

typedef struct chunk chunk;

chunk* init_memory(int bytes) {
	chunk *first_elem;

	//point to the beginning of our array(aka available memory)
	first_elem = mem_head;

	//fill structure fields
	first_elem->memory_size = bytes;
	first_elem->next = NULL;
	first_elem->prev = NULL;

	for(int i = 0; i<bytes; i++)
		*((char*)((void*)first_elem+sizeof(chunk)+i))=1;

	return(first_elem);
}


chunk* add_memory(chunk *previous_chunk, chunk *start_address, int bytes)
{

	chunk *new, *p;
	printf("prev=%p, start=%p\n", previous_chunk, start_address);

	new = start_address;

	printf("\n");

	p = previous_chunk->next;
	previous_chunk->next = new;

        //printf("memory size is %p\n", &new->memory_size);
	new->memory_size = bytes;
        //printf("next field is %p\n", &new->next);
	new->next = p;
        //printf("previous field is %p\n", &new->prev);
	new->prev = previous_chunk;

	if (p != NULL)
		p->prev = new;


	for(int i = 0; i<bytes; i++)
		*((char*)((void*)new+sizeof(chunk)+i))=1;

	return(new);
}


void delete_memory(chunk *del_el)
{
	chunk* next;
	chunk* prev;
	int bytes = del_el->memory_size;

        //printf("Delete from %p\n", del_el);
	for(int i = 0; i < bytes + sizeof(chunk); i++)
		*((char*)((void*)del_el+i))=0;

	prev = del_el->prev;
	next = del_el-> next;


	if (next != NULL)
		next->prev = del_el->prev;

	if (prev != NULL)
	 prev->next = del_el->next;
}

chunk* find_free_space (int bytes)
{
	chunk* list;

	list = mem_head;

	if ( list->next == NULL && (((mem_head + AVAILABLE_MEMORY) - list->memory_size - sizeof(chunk)) >= bytes+sizeof(chunk)) )
		return mem_head+sizeof(chunk)+list->memory_size+1;

	if ( list->prev == NULL && ( list - mem_head >= bytes+sizeof(chunk)) )
		return mem_head;

	do {
		next = list->next;
		if (list->next - (list + sizeof(chunk) + list->memory_size) >= bytes)
			return list + sizeof(chunk) + list->memory_size + 1;

		list=list->next;
	}
	while (mem_head != NULL);
}

chunk * emalloc(int bytes) {

	chunk* pointer;

	chunk* pointer1;
	static int status = 0;

	if (status == 0) {
		pointer = init_memory(bytes);
		printf("Init first %d bytes of memory with address %p\n", bytes, pointer);
		status = 1;
	}

	pointer1 = add_memory(pointer, mem_head+sizeof(chunk)+4, 2);

	printf("Added another memory at %p\n", pointer1);

	delete_memory(pointer);

	return 123;
}


int main(void)
{
	int memory =4;
	printf ("Mem_head %p\n", mem_head);
	emalloc(memory);

	for(int i=0; i<48; i++){
		printf("%x\n",_mem[i]);
	}


	return 0;
}