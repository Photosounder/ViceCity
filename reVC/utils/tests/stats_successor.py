"""Verify exact later projections against immutable historical source digests."""
from pathlib import Path
import hashlib,importlib
_verified_sources=set()

def integration(source):
    # Isolate the obsolete Windows qualifier macros while retaining FAR's original empty expansion
    old='#include <rouziclib/rouziclib.c>'
    assert source.count(old)==1
    return source.replace(old,'''//+ rouz edit (ChatGPT)
// Preserve rouziclib's local far variable and the empty Windows FAR qualifier
#ifdef _WIN32
#pragma push_macro("far")
#pragma push_macro("FAR")
#undef FAR
#define FAR
#undef far
#endif
#include <rouziclib/rouziclib.c>
#ifdef _WIN32
#pragma pop_macro("FAR")
#pragma pop_macro("far")
#endif
//- rouz edit (ChatGPT)''')

def current_path(root,path):
    # Resolve only the explicit producer renames recorded by completed conversions
    current=root/path
    if not current.exists():
        from profile_power_migration import RENAMES
        from lists_migration import RENAMES as LIST_RENAMES
        from transmission_migration import RENAMES as TRANSMISSION_RENAMES
        from damage_migration import RENAMES as DAMAGE_RENAMES
        from ped_routes_migration import RENAMES as ROUTE_RENAMES
        from rect_migration import RENAMES as RECT_RENAMES
        from col_triangle_migration import RENAMES as TRIANGLE_RENAMES
        from col_line_migration import RENAMES as LINE_RENAMES
        from col_box_migration import RENAMES as BOX_RENAMES
        from col_sphere_migration import RENAMES as SPHERE_RENAMES
        from col_point_migration import RENAMES as POINT_RENAMES
        from quaternion_migration import RENAMES as QUATERNION_RENAMES
        from anim_node_migration import RENAMES as ANIM_NODE_RENAMES
        from anim_clump_migration import RENAMES as ANIM_CLUMP_RENAMES
        from anim_sequence_migration import RENAMES as ANIM_SEQUENCE_RENAMES
        from anim_hierarchy_migration import RENAMES as ANIM_HIERARCHY_RENAMES
        from anim_association_migration import RENAMES as ANIM_ASSOCIATION_RENAMES
        from anim_group_migration import RENAMES as ANIM_GROUP_RENAMES
        from anim_manager_migration import RENAMES as ANIM_MANAGER_RENAMES
        from bones_migration import RENAMES as BONES_RENAMES
        from frame_update_migration import RENAMES as FRAME_UPDATE_RENAMES
        from anim_plugin_migration import RENAMES as ANIM_PLUGIN_RENAMES
        from node_name_migration import RENAMES as NODE_NAME_RENAMES
        from col_model_migration import RENAMES as COL_MODEL_RENAMES
        from memory_mgr_migration import RENAMES as MEMORY_MGR_RENAMES
        from memory_heap_migration import RENAMES as MEMORY_HEAP_RENAMES
        from txd_store_migration import RENAMES as TXD_STORE_RENAMES
        from tex_read_migration import RENAMES as TEX_READ_RENAMES
        from clump_read_migration import RENAMES as CLUMP_READ_RENAMES
        from lights_migration import RENAMES as LIGHTS_RENAMES
        from texture_pools_migration import RENAMES as TEXTURE_POOLS_RENAMES
        from mblur_migration import RENAMES as MBLUR_RENAMES
        from postfx_migration import RENAMES as POSTFX_RENAMES
        from rw_helper_migration import RENAMES as RW_HELPER_RENAMES
        from shadow_cameras_migration import RENAMES as SHADOW_CAMERAS_RENAMES
        from sprite_migration import RENAMES as SPRITE_RENAMES
        from sprite2d_migration import RENAMES as SPRITE2D_RENAMES
        from weapon_effects_migration import RENAMES as WEAPON_EFFECTS_RENAMES
        from shot_info_migration import RENAMES as SHOT_INFO_RENAMES
        current=root/({**RENAMES,**LIST_RENAMES,**TRANSMISSION_RENAMES,**DAMAGE_RENAMES,**ROUTE_RENAMES,**RECT_RENAMES,**TRIANGLE_RENAMES,**LINE_RENAMES,**BOX_RENAMES,**SPHERE_RENAMES,**POINT_RENAMES,**QUATERNION_RENAMES,**ANIM_NODE_RENAMES,**ANIM_CLUMP_RENAMES,**ANIM_SEQUENCE_RENAMES,**ANIM_HIERARCHY_RENAMES,**ANIM_ASSOCIATION_RENAMES,**ANIM_GROUP_RENAMES,**ANIM_MANAGER_RENAMES,**BONES_RENAMES,**FRAME_UPDATE_RENAMES,**ANIM_PLUGIN_RENAMES,**NODE_NAME_RENAMES,**COL_MODEL_RENAMES,**MEMORY_MGR_RENAMES,**MEMORY_HEAP_RENAMES,**TXD_STORE_RENAMES,**TEX_READ_RENAMES,**CLUMP_READ_RENAMES,**LIGHTS_RENAMES,**TEXTURE_POOLS_RENAMES,**MBLUR_RENAMES,**POSTFX_RENAMES,**RW_HELPER_RENAMES,**SHADOW_CAMERAS_RENAMES,**SPRITE_RENAMES,**SPRITE2D_RENAMES,**WEAPON_EFFECTS_RENAMES,**SHOT_INFO_RENAMES}).get(path,path)
    return current


def verify(root,path,digest):
    # Accept only a full explicit successor projection of a byte-identical historical snapshot
    current=current_path(root,path)
    currentDigest=hashlib.sha256(current.read_bytes()).hexdigest();proof=(str(current.resolve()),digest,currentDigest)
    if currentDigest==digest or proof in _verified_sources:return
    for stage in ['events','wanted','darkel','stats','roadblocks','debug-display','autopilot','profile-power','lists','streaming-info','input-state','transmission','doors','damage','ped-routes','stored-car','rect','compressed-vector','col-triangle','col-line','col-box','col-sphere','col-point','stored-poly','quaternion','keyframes','anim-node','anim-clump','anim-frame','anim-sequence','anim-hierarchy','anim-association','anim-group','anim-manager','bones','frame-update','anim-plugin','node-name','col-model','memory-mgr','memory-heap','txd-store','tex-read','clump-read','lights','texture-pools','mblur','postfx','rw-helper','shadow-cameras','sprite','sprite2d','weapon-effects','shot-info']:
        original=root/f'.diagnostics/{stage}-c-tests/before'/path
        if original.exists() and hashlib.sha256(original.read_bytes()).hexdigest()==digest:
            expected=importlib.import_module(stage.replace('-','_')+'_migration').calls(original.read_text())
            if expected==current.read_text():
                # Reuse a proven transformation only while both the historical and current byte digests match
                _verified_sources.add(proof);return
    if path=='src/core/rl.c':
        original=root/'.diagnostics/stats-c-tests/integration-before'/path
        assert original.exists() and hashlib.sha256(original.read_bytes()).hexdigest()==digest,path
        assert integration(original.read_text())==current.read_text(),path
        # Key the integration proof by actual bytes so later source edits cannot reuse stale evidence
        _verified_sources.add(proof)
        return
    raise AssertionError(path)
