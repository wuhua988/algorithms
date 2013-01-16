/*******************************************************************************
 * DANIEL'S PRIVATE ALGORITHM IMPLEMENTAIONS
 *
 * Dijkstra Algorithm
 * 
 * Features:
 * 
 *   Dijkstra's algorithm, conceived by Dutch computer scientist Edsger Dijkstra
 * in 1956 and published in 1959,[1][2] is a graph search algorithm that
 * solves the single-source shortest path problem for a graph with nonnegative
 * edge path costs, producing a shortest path tree. This algorithm is often
 * used in routing and as a subroutine in other graph algorithms.
 *
 ******************************************************************************/
#ifndef __DIJKSTRA_H__
#define __DIJKSTRA_H__

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>

#include "heap.h"
#include "directed_graph.h"
#include "stack.h"
#include "perfect_hash.h"

#define undefined (void*)-1

struct DijkWorkspace {
	struct Heap * Q;
	struct PerfHT * dist; 
	struct PerfHT * previous; 
	uint32_t num_vertex;	
	uint32_t vertex_ids[1];
};

static inline void dijkstra_reorder(struct Heap * heap, uint32_t id, uint32_t new_weight)
{	
	int index;
	int key = new_weight;
	if ((index=heap_find_data(heap, (void *)id))!=-1) {
		heap_modify_key(heap, index, key);
	}
}

static inline void dijkstra_init(struct DirectedGraph * g, struct Adjacent * source, struct DijkWorkspace * dr)
{

	// binary heap init
	struct Heap * Q = heap_init(g->num_vertex);
	struct Adjacent * a;
	heap_insert(Q, 0, (void*)source->v.id);	// weight->id binary heap

	int i=1;
	list_for_each_entry(a, &g->a_head, a_node){
		if (source->v.id != a->v.id) {
			dr->vertex_ids[i++] = a->v.id;
			heap_insert(Q, INT_MAX, (void*)a->v.id);
		}
	}

	dr->num_vertex = g->num_vertex;
	dr->vertex_ids[0] = source->v.id;

	// hash table for id->dist , set inital distance to INT_MAX
	struct PerfHT * dist = perfect_hash_init(dr->vertex_ids, g->num_vertex);

	perfect_hash_set(dist, dr->vertex_ids[0], 0);
	for(i=1;i<dr->num_vertex;i++) {
		perfect_hash_set(dist, dr->vertex_ids[i], (void*)INT_MAX);
	}

	// hash table for node -> previous node, for trackback
	struct PerfHT * previous = perfect_hash_init(dr->vertex_ids, g->num_vertex);
	// set initial value to undefined
	for(i=0;i<dr->num_vertex;i++) {
		perfect_hash_set(previous, dr->vertex_ids[i], undefined);
	}

	dr->Q = Q;
	dr->dist = dist;
	dr->previous = previous;
}

inline struct DijkWorkspace * dijkstra_run(struct DirectedGraph * g, struct Adjacent * source)
{
	struct DijkWorkspace * dr =
		 (struct DijkWorkspace *)malloc(sizeof(struct DijkWorkspace) + sizeof(uint32_t) * g->num_vertex);

	dijkstra_init(g, source, dr);
	struct Heap * Q = dr->Q;
	struct PerfHT * dist = dr->dist;
	struct PerfHT * previous = dr->previous;

	while(!heap_is_empty(Q)) {    // The main loop
		struct Adjacent * u = directed_graph_lookup(g, (uint32_t)HEAP_MIN_VALUE(Q));
		int dist_u = HEAP_MIN_KEY(Q);
		heap_delete_min(Q);

		if (dist_u == INT_MAX){
			break;	
		}
	
		struct Vertex * v;
		list_for_each_entry(v, &u->v_head, v_node){
			uint32_t alt = dist_u + v->weight;
			uint32_t dist_v = (uint32_t)perfect_hash_get(dist, v->id);
			if (alt < dist_v) {
				perfect_hash_set(dist, v->id, (void*)alt);
				dijkstra_reorder(Q, v->id, alt);
				perfect_hash_set(previous, v->id, (void *)u->v.id);
			}
		}
	}

	return dr;
}

#endif //
