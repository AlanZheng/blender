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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file BKE_workspace.h
 *  \ingroup bke
 */

#ifndef __BKE_WORKSPACE_H__
#define __BKE_WORKSPACE_H__

#include "BLI_compiler_attrs.h"

struct bScreen;
struct ListBase;
struct Main;
struct TransformOrientation;
struct WorkSpace;

typedef struct WorkSpace WorkSpace;
typedef struct WorkSpaceInstanceHook WorkSpaceInstanceHook;
typedef struct WorkSpaceLayout WorkSpaceLayout;

/**
 * Plan is to store the object-mode per workspace, not per object anymore.
 * However, there's quite some work to be done for that, so for now, there is just a basic
 * implementation of an object <-> workspace object-mode syncing for testing, with some known
 * problems. Main problem being that the modes can get out of sync when changing object selection.
 * Would require a pile of temporary changes to always sync modes when changing selection. So just
 * leaving this here for some testing until object-mode is really a workspace level setting.
 */
#define USE_WORKSPACE_MODE

/* -------------------------------------------------------------------- */
/* Create, delete, init */

WorkSpace *BKE_workspace_add(struct Main *bmain, const char *name);
void BKE_workspace_free(WorkSpace *workspace);
void BKE_workspace_remove(struct Main *bmain, WorkSpace *workspace);

WorkSpaceInstanceHook *BKE_workspace_instance_hook_create(const struct Main *bmain);
void BKE_workspace_instance_hook_free(const struct Main *bmain, WorkSpaceInstanceHook *hook);

struct WorkSpaceLayout *BKE_workspace_layout_add(
        WorkSpace *workspace,
        struct bScreen *screen,
        const char *name) ATTR_NONNULL();
void BKE_workspace_layout_remove(
        struct Main *bmain,
        WorkSpace *workspace, WorkSpaceLayout *layout) ATTR_NONNULL();

/* -------------------------------------------------------------------- */
/* General Utils */

#define BKE_WORKSPACE_ITER_BEGIN(_workspace, _start_workspace) \
	for (WorkSpace *_workspace = _start_workspace, *_workspace##_next; _workspace; _workspace = _workspace##_next) { \
		_workspace##_next = BKE_workspace_next_get(_workspace); /* support removing workspace from list */
#define BKE_WORKSPACE_ITER_END } ((void)0)

void BKE_workspaces_transform_orientation_remove(
        const struct ListBase *workspaces,
        const struct TransformOrientation *orientation) ATTR_NONNULL();

WorkSpaceLayout *BKE_workspace_layout_find(
        const WorkSpace *workspace, const struct bScreen *screen) ATTR_NONNULL() ATTR_WARN_UNUSED_RESULT;
WorkSpaceLayout *BKE_workspace_layout_find_global(
        const struct Main *bmain, const struct bScreen *screen,
        WorkSpace **r_workspace) ATTR_NONNULL(1, 2) ATTR_WARN_UNUSED_RESULT;

#define BKE_WORKSPACE_LAYOUT_ITER_BEGIN(_layout, _start_layout) \
	for (WorkSpaceLayout *_layout = _start_layout, *_layout##_next; _layout; _layout = _layout##_next) { \
		_layout##_next = BKE_workspace_layout_next_get(_layout); /* support removing layout from list */
#define BKE_WORKSPACE_LAYOUT_ITER_BACKWARD_BEGIN(_layout, _start_layout) \
	for (WorkSpaceLayout *_layout = _start_layout, *_layout##_prev; _layout; _layout = _layout##_prev) { \
		_layout##_prev = BKE_workspace_layout_prev_get(_layout); /* support removing layout from list */
#define BKE_WORKSPACE_LAYOUT_ITER_END } ((void)0)

WorkSpaceLayout *BKE_workspace_layout_iter_circular(
        const WorkSpace *workspace, WorkSpaceLayout *start,
        bool (*callback)(const WorkSpaceLayout *layout, void *arg),
        void *arg, const bool iter_backward);


/* -------------------------------------------------------------------- */
/* Getters/Setters */

#define GETTER_ATTRS ATTR_NONNULL() ATTR_WARN_UNUSED_RESULT
#define SETTER_ATTRS ATTR_NONNULL(1)

WorkSpace *BKE_workspace_active_get(WorkSpaceInstanceHook *hook) GETTER_ATTRS;
void       BKE_workspace_active_set(WorkSpaceInstanceHook *hook, WorkSpace *workspace) SETTER_ATTRS;
struct ID *BKE_workspace_id_get(WorkSpace *workspace) GETTER_ATTRS;
const char *BKE_workspace_name_get(const WorkSpace *workspace) GETTER_ATTRS;
WorkSpace *BKE_workspace_next_get(const WorkSpace *workspace) GETTER_ATTRS;
WorkSpace *BKE_workspace_prev_get(const WorkSpace *workspace) GETTER_ATTRS;
WorkSpace *BKE_workspace_temp_store_get(WorkSpaceInstanceHook *hook) GETTER_ATTRS;
void       BKE_workspace_temp_store_set(WorkSpaceInstanceHook *hook, WorkSpace *workspace) SETTER_ATTRS;
WorkSpaceLayout *BKE_workspace_active_layout_get(const WorkSpaceInstanceHook *hook) GETTER_ATTRS;
void             BKE_workspace_active_layout_set(WorkSpaceInstanceHook *hook, WorkSpaceLayout *layout) SETTER_ATTRS;
WorkSpaceLayout *BKE_workspace_temp_layout_store_get(const WorkSpaceInstanceHook *hook) GETTER_ATTRS;
void             BKE_workspace_temp_layout_store_set(WorkSpaceInstanceHook *hook, WorkSpaceLayout *layout) SETTER_ATTRS;
struct bScreen *BKE_workspace_active_screen_get(const WorkSpaceInstanceHook *hook) GETTER_ATTRS;
void            BKE_workspace_active_screen_set(
        WorkSpaceInstanceHook *hook, struct WorkSpace *workspace, struct bScreen *screen) SETTER_ATTRS;
enum ObjectMode BKE_workspace_object_mode_get(const WorkSpace *workspace) GETTER_ATTRS;
#ifdef USE_WORKSPACE_MODE
void            BKE_workspace_object_mode_set(WorkSpace *workspace, const enum ObjectMode mode) SETTER_ATTRS;
#endif
struct SceneLayer *BKE_workspace_render_layer_get(const WorkSpace *workspace) GETTER_ATTRS;
void               BKE_workspace_render_layer_set(WorkSpace *workspace, struct SceneLayer *layer) SETTER_ATTRS;
struct ListBase *BKE_workspace_layouts_get(WorkSpace *workspace) GETTER_ATTRS;

const char *BKE_workspace_layout_name_get(const WorkSpaceLayout *layout) GETTER_ATTRS;
void        BKE_workspace_layout_name_set(WorkSpace *workspace, WorkSpaceLayout *layout, const char *new_name) ATTR_NONNULL();
struct bScreen *BKE_workspace_layout_screen_get(const WorkSpaceLayout *layout) GETTER_ATTRS;
void            BKE_workspace_layout_screen_set(WorkSpaceLayout *layout, struct bScreen *screen) SETTER_ATTRS;
WorkSpaceLayout *BKE_workspace_layout_next_get(const WorkSpaceLayout *layout) GETTER_ATTRS;
WorkSpaceLayout *BKE_workspace_layout_prev_get(const WorkSpaceLayout *layout) GETTER_ATTRS;

WorkSpaceLayout *BKE_workspace_hook_layout_for_workspace_get(
        const WorkSpaceInstanceHook *hook, const WorkSpace *workspace) GETTER_ATTRS;
void             BKE_workspace_hook_layout_for_workspace_set(
        WorkSpaceInstanceHook *hook, WorkSpace *workspace, WorkSpaceLayout *layout) ATTR_NONNULL();
struct ListBase *BKE_workspace_hook_layout_relations_get(WorkSpace *workspace) GETTER_ATTRS;

struct WorkSpaceDataRelation *BKE_workspace_relation_next_get(const struct WorkSpaceDataRelation *relation) GETTER_ATTRS;
void BKE_workspace_relation_data_get(
        const struct WorkSpaceDataRelation *relation,
        void **parent, void **data) ATTR_NONNULL();
void BKE_workspace_relation_data_set(struct WorkSpaceDataRelation *relation, void *parent, void *data) ATTR_NONNULL();

#undef GETTER_ATTRS
#undef SETTER_ATTRS

/* -------------------------------------------------------------------- */
/* Don't use outside of BKE! */

WorkSpace *BKE_workspace_alloc(void) ATTR_WARN_UNUSED_RESULT;

#endif /* __BKE_WORKSPACE_H__ */