#ifndef _QQ_SCROLL_BAR_H_
#define _QQ_SCROLL_BAR_H_

#include "lvgl.h"

typedef struct scroll_bar_s {
    lv_obj_t *content;                  // 滚动容器
    lv_timer_t *timer;                  // 回正定时器
    uint16_t child_nums;                // 子对象数量
    int16_t spacing;                    // 子对象中心间距（必须 > 子对象最大宽度/高度）
    uint16_t selected;                  // 当前选中索引
    uint16_t direction : 1;             // 0=水平 1=垂直
    uint16_t onesnap   : 1;             // 是否只能一次滚动一个
    uint16_t optimized : 1;             // 性能优化（隐藏远端对象）

    float scale_min;                    // 最小缩放比例
    float scale_max;                    // 最大缩放比例（选中项）

    struct {
        uint16_t width;
        uint16_t height;
    } item_size;                        // 选中时的大小（最大尺寸）

    lv_obj_t **children;                // 子对象指针数组（顺序固定）

    /* 回调 */
    void (*item_childs_create_cb)(lv_obj_t *item, uint16_t index);
    void (*item_childs_scale_cb)(lv_obj_t *item, float scale);
} scroll_bar_t;

/* 创建 */
lv_obj_t *scroll_bar_create(lv_obj_t *parent, scroll_bar_t *sb);

/* 刷新（切换选中、改变参数时调用） */
void scroll_bar_refresh(scroll_bar_t *sb);

#endif