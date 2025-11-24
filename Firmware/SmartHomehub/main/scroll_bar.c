#include "scroll_bar.h"
#include "esp_log.h"


/* 前向声明 */
static void scroll_bar_timer_cb(lv_timer_t *timer);
static void scroll_bar_scroll_cb(lv_event_t *e);
static void scroll_bar_scroll_end_cb(lv_event_t *e);
static void scroll_bar_delete_cb(lv_event_t *e);
static void reorder_children_for_circular(scroll_bar_t *sb);

/* 600ms 后自动吸附到最近项 */
static void scroll_bar_timer_cb(lv_timer_t *timer)
{
    scroll_bar_t *sb = timer->user_data;
    if (sb->selected < sb->child_nums) {
        lv_obj_scroll_to_view(sb->children[sb->selected], LV_ANIM_ON);
    }
    lv_timer_pause(timer);
}

/* 滚动结束 → 重启回正定时器 */
static void scroll_bar_scroll_end_cb(lv_event_t *e)
{
    scroll_bar_t *sb = lv_event_get_user_data(e);
    if (sb->timer) {
        lv_timer_reset(sb->timer);
        lv_timer_resume(sb->timer);
    }
}

/* 删除时清理资源 */
static void scroll_bar_delete_cb(lv_event_t *e)
{
    scroll_bar_t *sb = lv_event_get_user_data(e);
    if (sb->children) {
        lv_mem_free(sb->children);
        sb->children = NULL;
    }
    if (sb->timer) {
        lv_timer_del(sb->timer);
        sb->timer = NULL;
    }
}

/* 关键：无限循环滚动核心 —— 通过 swap + move_to_index 实现 */
static void reorder_children_for_circular(scroll_bar_t *sb)
{
    uint16_t mid = sb->child_nums / 2;
    uint16_t idx = sb->selected;

    for (uint16_t i = 0; i < sb->child_nums; i++) {
        if (sb->child_nums % 2 == 0) {
            if (i < mid)
                lv_obj_move_to_index(sb->children[idx], i + mid);
            else
                lv_obj_move_to_index(sb->children[idx], i - mid);
        } else {
            if (i <= mid)
                lv_obj_move_to_index(sb->children[idx], i + mid);
            else
                lv_obj_move_to_index(sb->children[idx], i - (mid + 1));
        }
        if (++idx >= sb->child_nums) idx = 0;
    }
}

/* SCROLL 事件：无限滚动 + 缩放 + 选中检测 */
static void scroll_bar_scroll_cb(lv_event_t *e)
{
    scroll_bar_t *sb = lv_event_get_user_data(e);
    lv_obj_t *cont = lv_event_get_target(e);

    /* 暂停回正定时器 */
    if (sb->timer) lv_timer_pause(sb->timer);

    lv_coord_t scroll_pos = sb->direction ? lv_obj_get_scroll_y(cont)
                                          : lv_obj_get_scroll_x(cont);

    lv_area_t cont_area;
    lv_obj_get_coords(cont, &cont_area);
    lv_coord_t cont_center = sb->direction ?
        cont_area.y1 + lv_area_get_height(&cont_area) / 2 :
        cont_area.x1 + lv_area_get_width(&cont_area) / 2;

    /* ---------- 无限滚动：前后补位 ---------- */
    if (scroll_pos > sb->spacing * (lv_coord_t)sb->child_nums / 2 + sb->spacing) {
        for (int i = 0; i < (int)sb->child_nums - 1; i++) {
            lv_obj_t *a = lv_obj_get_child(cont, i);
            lv_obj_t *b = lv_obj_get_child(cont, i + 1);
            lv_obj_swap(a, b);
        }
        scroll_pos -= sb->spacing;
        if (sb->direction) lv_obj_scroll_to_y(cont, scroll_pos, LV_ANIM_OFF);
        else               lv_obj_scroll_to_x(cont, scroll_pos, LV_ANIM_OFF);
    }
    else if (scroll_pos < sb->spacing * (lv_coord_t)sb->child_nums / 2 - sb->spacing) {
        for (int i = (int)sb->child_nums - 2; i >= 0; i--) {
            lv_obj_t *a = lv_obj_get_child(cont, i);
            lv_obj_t *b = lv_obj_get_child(cont, i + 1);
            lv_obj_swap(a, b);
        }
        scroll_pos += sb->spacing;
        if (sb->direction) lv_obj_scroll_to_y(cont, scroll_pos, LV_ANIM_OFF);
        else               lv_obj_scroll_to_x(cont, scroll_pos, LV_ANIM_OFF);
    }

    /* ---------- 缩放 + 选中 + 优化隐藏 ---------- */
    uint16_t new_selected = sb->selected;

    for (uint16_t i = 0; i < sb->child_nums; i++) {
        lv_obj_t *child = lv_obj_get_child(cont, i);
        lv_area_t coords;
        lv_obj_get_coords(child, &coords);

        lv_coord_t child_center = sb->direction ?
            coords.y1 + lv_area_get_height(&coords) / 2 :
            coords.x1 + lv_area_get_width(&coords) / 2;

        lv_coord_t dist = LV_ABS(child_center - cont_center);
        float scale = sb->scale_min;
        if (dist < sb->spacing) {
            scale = sb->scale_min + (sb->scale_max - sb->scale_min) *
                    (float)(sb->spacing - dist) / (float)sb->spacing;
        }

        lv_obj_set_size(child,
                        (lv_coord_t)(sb->item_size.width  * scale),
                        (lv_coord_t)(sb->item_size.height * scale));

        if (sb->item_childs_scale_cb) {
            sb->item_childs_scale_cb(child, scale);
        }

        /* 找到指针对应的原始索引 */
        uint16_t real_idx = 0;
        for (uint16_t j = 0; j < sb->child_nums; j++) {
            if (sb->children[j] == child) {
                real_idx = j;
                break;
            }
        }

        if (dist < sb->spacing / 2) {
            new_selected = real_idx;
            lv_obj_add_flag(child, LV_OBJ_FLAG_CLICKABLE);
            if (sb->optimized) lv_obj_clear_flag(child, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(child, LV_OBJ_FLAG_CLICKABLE);
            if (sb->optimized) {
                lv_coord_t limit = (sb->direction ? lv_obj_get_height(cont) : lv_obj_get_width(cont)) / 2 + sb->spacing;
                if (dist > limit) lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
                else              lv_obj_clear_flag(child, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    sb->selected = new_selected;
}

/* 创建函数 */
lv_obj_t *scroll_bar_create(lv_obj_t *parent, scroll_bar_t *sb)
{
    sb->content = lv_obj_create(parent);
    lv_obj_remove_style_all(sb->content);
    lv_obj_set_size(sb->content, LV_PCT(100), LV_PCT(100));

    /* 吸附设置 */
    lv_obj_set_scroll_snap_x(sb->content, sb->direction ? LV_SCROLL_SNAP_NONE : LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scroll_snap_y(sb->content, sb->direction ? LV_SCROLL_SNAP_CENTER : LV_SCROLL_SNAP_NONE);
    if (sb->onesnap) lv_obj_add_flag(sb->content, LV_OBJ_FLAG_SCROLL_ONE);

    /* 分配指针数组 */
    sb->children = lv_mem_alloc(sizeof(lv_obj_t *) * sb->child_nums);
    if (!sb->children) return NULL;

    /* 回正定时器 */
    sb->timer = lv_timer_create(scroll_bar_timer_cb, 600, sb);
    lv_timer_pause(sb->timer);

    /* 创建所有子对象 */
    for (uint16_t i = 0; i < sb->child_nums; i++) {
        lv_obj_t *item = lv_obj_create(sb->content);
        sb->children[i] = item;

        lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(item, LV_OBJ_FLAG_EVENT_BUBBLE);

        if (sb->item_childs_create_cb) {
            sb->item_childs_create_cb(item, i);
        }

        float init_scale = (i == sb->selected) ? sb->scale_max : sb->scale_min;
        lv_obj_set_size(item,
                        (lv_coord_t)(sb->item_size.width  * init_scale),
                        (lv_coord_t)(sb->item_size.height * init_scale));
        if (sb->item_childs_scale_cb) {
            sb->item_childs_scale_cb(item, init_scale);
        }
    }

    /* 初始循环排列 */
    reorder_children_for_circular(sb);

    /* 初始位置 */
    for (uint16_t i = 0; i < sb->child_nums; i++) {
        lv_obj_t *child = lv_obj_get_child(sb->content, i);
        if (sb->direction)
            lv_obj_set_y(child, i * sb->spacing);
        else
            lv_obj_set_x(child, i * sb->spacing);
    }

    /* 滚动到选中项 + 添加事件 */
    lv_obj_scroll_to_view(sb->children[sb->selected], LV_ANIM_OFF);
    lv_obj_add_flag(sb->children[sb->selected], LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(sb->content, scroll_bar_scroll_cb,      LV_EVENT_SCROLL,     sb);
    lv_obj_add_event_cb(sb->content, scroll_bar_scroll_end_cb, LV_EVENT_SCROLL_END, sb);
    lv_obj_add_event_cb(sb->content, scroll_bar_delete_cb,     LV_EVENT_DELETE,     sb);

    return sb->content;
}

/* 刷新函数（比如切换选中项） */
void scroll_bar_refresh(scroll_bar_t *sb)
{
    /* 重新排列实现循环 */
    reorder_children_for_circular(sb);

    /* 重新设置位置 */
    for (uint16_t i = 0; i < sb->child_nums; i++) {
        lv_obj_t *child = lv_obj_get_child(sb->content, i);
        if (sb->direction)
            lv_obj_set_y(child, i * sb->spacing);
        else
            lv_obj_set_x(child, i * sb->spacing);
    }

    /* 清除所有点击标志 */
    for (uint16_t i = 0; i < sb->child_nums; i++) {
        lv_obj_clear_flag(sb->children[i], LV_OBJ_FLAG_CLICKABLE);
    }

    /* 滚动到新选中项 */
    lv_obj_scroll_to_view(sb->children[sb->selected], LV_ANIM_OFF);
    lv_obj_add_flag(sb->children[sb->selected], LV_OBJ_FLAG_CLICKABLE);
}