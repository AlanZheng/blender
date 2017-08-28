/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software  Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) Blender Foundation
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Martin Felke
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file DNA_fracture_types.h
 *  \ingroup DNA
 */
 
#ifndef DNA_FRACTURE_TYPES_H
#define DNA_FRACTURE_TYPES_H

#include "BLI_utildefines.h"
#include "DNA_mesh_types.h"
#include "DNA_object_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct DerivedMesh;
struct KDTree;

enum {
	SHARD_INTACT   = 1 << 0,
	SHARD_FRACTURED = 1 << 1,
	SHARD_SKIP = 1 << 2,
	SHARD_DELETE = 1 << 3,
};

typedef struct Shard {
	struct Shard *next, *prev;
	struct MVert *mvert;
	struct MPoly *mpoly;
	struct MLoop *mloop;
	struct MEdge *medge;

	struct CustomData vertData;
	struct CustomData polyData;
	struct CustomData loopData;
	struct CustomData edgeData;

	int totvert, totpoly, totloop, totedge;
	int pad[2];
	
	int *cluster_colors;
	float min[3], max[3];
	float centroid[3];  /* centroid of shard, calculated during fracture */
	float raw_centroid[3];  /*store raw, unprocessed centroid here (might change when mesh shape changes via boolean / bisect) */
	int *neighbor_ids;  /* neighbors of me... might be necessary for easier compounding or fracture, dont need to iterate over all */
	int shard_id;       /* the identifier */
	int neighbor_count; /* counts of neighbor islands */
	int parent_id;      /* the shard from which this shard originates, we keep all shards in the shardmap */
	int flag;           /* flag for fracture state (INTACT, FRACTURED)*/
	int setting_id;     /* to which setting this shard belongs, -1 for none or no settings available*/
	float raw_volume;
	float impact_loc[3]; /* last impact location on this shard */
	float impact_size[3]; /* size of impact area (simplified) */
	//char pad2[4];
} Shard;

typedef struct FracMesh {
	struct KDTree *last_shard_tree;
	struct Shard **last_shards;
	ListBase shard_map;     /* groups mesh elements to islands, generated by fracture itself */
	int shard_count;        /* how many islands we have */
	short cancel;           /* whether the process is cancelled (from the job, ugly, but this way we dont need the entire modifier) */
	short running;          /* whether the process is currently in progress, so the modifier wont be touched from the main thread */
	int progress_counter;   /* counts progress */
	int last_expected_shards;
} FracMesh;

typedef struct SharedVertGroup {
	struct SharedVertGroup *next, *prev;
	float rest_co[3];
	float delta[3];
	int index, excession_frame;
	int exceeded, deltas_set, moved;
	char pad[4];
	ListBase verts;
} SharedVertGroup;

typedef struct SharedVert {
	struct SharedVert *next, *prev;
	float rest_co[3];
	float delta[3];
	int index, excession_frame;
	int exceeded, deltas_set, moved;
	char pad[4];
} SharedVert;

#ifdef __cplusplus
}
#endif

#endif /* DNA_FRACTURE_TYPES_H */