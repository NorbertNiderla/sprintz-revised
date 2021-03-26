#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "huffman.h"
#include "bitstream.h"
#include "distribution.h"

#define NUMBERS 256
#define BITS 8

typedef struct node_t {
	struct node_t* left;
	struct node_t* right;
	struct node_t* father;
	struct node_t* next;
	int freq;
	int sign;
	bool original;
}node;

typedef struct nodes_list_t{
	node* start;
	int size;
	node* top;
}nodes_list;

typedef struct {
	uint16_t symbol;
	int bits;
}lookup_element;

static int frequencies[NUMBERS] = {0};
static nodes_list tree = {.start = NULL, .size = 0, .top = NULL};
static nodes_list free_nodes = {.start = NULL, .size = 0, .top = NULL};
static lookup_element lookup_table[NUMBERS];

static void appendNode(nodes_list* list, node* new_node)
{
	if(list->size==0){
		list->start = new_node;
		list->size++;
	}
	else{
		new_node->next = list->start;
		list->start = new_node;
		list->size++;
	}
}

static node* newNode(nodes_list* list, int frequency, int sign, node* left, node* right, node* father, bool original)
{
	node* new = malloc(sizeof(node));
	new->father = father;
	new->freq = frequency;
	new->left = left;
	new->original = original;
	new->right = right;
	new->sign = sign;
	new->next = NULL;

	if(list->size==0){
		list->start = new;
		list->size++;
	}
	else if((list->size!=0) & (new->freq<list->start->freq)){
		new->next = list->start;
		list->start = new;
		list->size++;	
	}
	else{
		node* prev = list->start;
		node* temp = list->start->next;
		while(temp != NULL){
			if (frequency <= temp->freq){
				new->next = temp;
				prev->next = new;
				list->size++;
				break;
			}
			prev = temp;
			temp = temp->next;
		}
		
		if(temp == NULL){
			prev->next = new;
			list->size++;
		}
	}
	return new;
}

static node* popNode(nodes_list* list)
{
	list->size--;
	node* temp = list->start;
	list->start = list->start->next;
	return temp;
}

void buildTree(void)
{
	freeHuffmanObjects();
	for (int i = 0; i < NUMBERS; i++) newNode(&free_nodes, frequencies[i], i, NULL, NULL, NULL, true);

	while (free_nodes.size > 1)
	{
		node* node_left = popNode(&free_nodes);
		node* node_right = popNode(&free_nodes);

		node* new = newNode(&free_nodes, node_left->freq + node_right->freq, 0, node_left, node_right, NULL, false);


		node_left->father = new;
		node_right->father = new;
		appendNode(&tree, node_left);
		appendNode(&tree, node_right);
	}
	node* last = popNode(&free_nodes);
	appendNode(&tree, last);
	tree.top = last;
}

void buildLookUpTable(void)
{
	for (int i = 0; i < NUMBERS; i++){
		lookup_table[i].bits = 0;
		lookup_table[i].symbol = 0;
	}

	node* main = tree.start;
	for(int i =0; i<tree.size; i++){
		if (main->original == true){
			node* temp = main;
			int k = 0;
			while (temp->father != NULL) {
				if (temp == temp->father->left){
					lookup_table[main->sign].symbol |= (1<<k++);
					lookup_table[main->sign].bits++;
				}
				else if (temp == temp->father->right){
					lookup_table[main->sign].symbol |= (0 << k++);
					lookup_table[main->sign].bits++;
				}

				temp = temp->father;
			}
		}
		main = main->next;
	}
}

int huffmanEncodeWithTableDecision(unsigned char* input, int size, unsigned char* output, size_t output_buffer_size, bool new_table)
{
	if(new_table){
		setFrequencies(input, size, frequencies);
		buildTree();
		buildLookUpTable();
	}

	bitstream_state_t state;
	bitstream_state_t* state_p = &state;
	bitstream_init(state_p, output, output_buffer_size);

	int n_bits = 0;
	int n_bytes = 0;
	for (int i = 0; i < size; i++)
	{
		n_bytes += bitstream_append_bits(state_p, lookup_table[input[i]].symbol, lookup_table[input[i]].bits);
		n_bits += lookup_table[input[i]].bits;
	}
	n_bytes += bitstream_write_close(state_p);
	return n_bytes;
}



int huffmanEncode(unsigned char* input, int size, unsigned char* output, size_t output_buffer_size)
{
	bitstream_state_t state;
	bitstream_state_t* state_p = &state;
	bitstream_init(state_p, output, output_buffer_size);

	int n_bits = 0;
	int n_bytes = 0;
	for (int i = 0; i < size; i++)
	{
		n_bytes += bitstream_append_bits(state_p, lookup_table[input[i]].symbol, lookup_table[input[i]].bits);
		n_bits += lookup_table[input[i]].bits;
	}
	n_bytes += bitstream_write_close(state_p);
	return n_bytes;
}

void huffmanDecode(unsigned char* input,size_t input_buffer_size, unsigned char* output, int size)
{
	bitstream_state_t state;
	bitstream_state_t* state_p = &state;
	bitstream_init(state_p, input, input_buffer_size);

	int idx = 0;
	node* temp = tree.top;
	
	while (idx < size)
	{
		unsigned long long bit;
		bitstream_read_bits(state_p, &bit, 1);
		if (bit == 1)
		{
			temp = temp->left;
		}
		else if (bit == 0)
		{
			temp = temp->right;
		}

		if (temp->original == true)
		{
			output[idx++] = (unsigned char)temp->sign;
			temp = tree.top;
		}
	}
	bitstream_read_close(state_p);
}

void freeHuffmanObjects(void)
{
	while(tree.size!=0) free(popNode(&tree));
	while(free_nodes.size!=0) free(popNode(&free_nodes));
}
