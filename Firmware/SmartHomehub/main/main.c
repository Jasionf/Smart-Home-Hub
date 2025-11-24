// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "nvs_flash.h"
// #include "nvs.h"
// #include "esp_log.h"
// #include "esp_err.h"
// #include "esp_check.h"
// #include "esp_memory_utils.h"
// #include "bsp/esp-bsp.h"
// #include "bsp/display.h"
// #include "bsp_board_extra.h"
// #include "lvgl.h"
// #include "ui.h"

// #include "scroll_bar.h"

// static void scroll_item_childs_create_cb(lv_obj_t * item, uint16_t index);
// static void scroll_item_childs_scale_cb(lv_obj_t * item, float scale);

// static struct scroll_bar_s scroll_bar = {
//     .content = NULL,
//     .child_nums = 7,
//     .spacing = 100,
//     .selected = 3,
//     .direction = 0,
//     .onesnap = 1,
//     .scale_min = 0.7,
//     .scale_max = 1.0,
//     .item_childs_create_cb = scroll_item_childs_create_cb,
//     .item_childs_scale_cb = scroll_item_childs_scale_cb,
//     .item_size = {100,100},
// };

// static void scroll_item_childs_create_cb(lv_obj_t * item, uint16_t index)
// {
//     lv_obj_set_style_bg_color(item,lv_color_hex(lv_rand(0,0xffffff)),0);
//     lv_obj_t * label = lv_label_create(item);
//     lv_obj_center(item);
//     lv_label_set_text_fmt(label,"L%d",index);
// }

// static void scroll_item_childs_scale_cb(lv_obj_t * item, float scale)
// {
//     lv_obj_t * label = lv_obj_get_child(item,0);
//     lv_obj_set_style_transform_scale(label,scale*255,0);
// }

// static void content_scroll_cb(lv_event_t * e)
// {
//     struct scroll_bar_s * scroll_bar = lv_event_get_user_data(e);
//     lv_obj_t * content = lv_event_get_target(e);
//     lv_ui * ui = lv_obj_get_user_data(content);
//     lv_label_set_text_fmt(ui->screen_label_1,"scroll_x:%d,scroll_y:%d,selected:%d",lv_obj_get_scroll_x(content) ,lv_obj_get_scroll_y(content),scroll_bar->selected);
// }

// void custom_init(lv_ui *ui)
// {
//     /* Add your codes here */
    
//     scroll_bar_create(ui->screen_cont,&scroll_bar);
//     scroll_bar.content->user_data = ui;
//     lv_obj_add_event_cb(scroll_bar.content,content_scroll_cb,LV_EVENT_SCROLL,&scroll_bar);
// }


// void app_main(void)
// {
//     bsp_display_cfg_t cfg = {
//         .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
//         .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
//         .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
//         .flags = {
//             .buff_dma = true,
//             .buff_spiram = false,
//             .sw_rotate = false, // must be set to true for software rotation
//         }
//     };

//     lv_display_t *disp = bsp_display_start_with_config(&cfg);

//     bsp_display_backlight_on();

//     bsp_display_lock(0);

//     bsp_display_unlock();
// }

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_memory_utils.h"
#include "bsp/esp-bsp.h"
#include "bsp/display.h"
#include "bsp_board_extra.h"
#include "lvgl.h"

// 图标数量
#define icon_count          20
// 图标之间的距离
#define icon_distance       220
// 小图标的尺寸
#define icon_size_small     120
// 大图标的尺寸
#define icon_size_big       200
 
// 定义图标结构体，包含图标对象指针和x坐标
typedef struct
{
  lv_obj_t * obj;
  int32_t x;
}icon_typedef;
 
// 声明图标结构体数组
static icon_typedef icon[icon_count];
// 触摸状态标志
static bool touched=false;
// 屏幕宽度
static int32_t scr_w;
// 触摸偏移量x
static int32_t t_offset_x;
// 屏幕对象指针
static lv_obj_t * screen;
 
// 按下事件回调函数声明
static void  pressing_cb(lv_event_t * e);
// 释放事件回调函数声明
static void  released_cb(lv_event_t * e);
// 设置x坐标回调函数声明
static void set_x_cb(void * var, int32_t v);
// 自定义动画创建函数声明
static void lv_myanim_creat(void * var,lv_anim_exec_xcb_t exec_cb,uint32_t time, uint32_t delay,lv_anim_path_cb_t path_cb,
                             int32_t start, int32_t end,lv_anim_ready_cb_t completed_cb,lv_anim_deleted_cb_t deleted_cb);
 
// 滚动图标功能函数
void scrollicon()
{
    int32_t i;
 
    // 获取默认显示器的垂直分辨率（这里可能是获取屏幕宽度的误写，根据后续使用推测）
    scr_w = lv_disp_get_ver_res(lv_disp_get_default());
    // 初始化图标结构体数组为0
    lv_memset(icon,0,sizeof(icon));
 
    // 创建一个平铺视图作为屏幕
    screen = lv_tileview_create(lv_scr_act());
    // 清除屏幕的可滚动标志
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
 
    // 为屏幕添加按下事件回调函数
    lv_obj_add_event_cb(screen,pressing_cb,LV_EVENT_PRESSING,0);
    // 为屏幕添加释放事件回调函数
    lv_obj_add_event_cb(screen,released_cb,LV_EVENT_RELEASED,0);
 
    for (i=0;i<icon_count;i++)
    {
        // 创建图标对象并添加到屏幕上
        icon[i].obj=lv_obj_create(screen);
        // 设置图标对象的用户数据为对应的图标结构体指针
        icon[i].obj->user_data=&icon[i];
        // 设置图标的背景颜色为红色
        lv_obj_set_style_bg_color(icon[i].obj,lv_color_hex(0xff0000), LV_PART_MAIN);
        // 在图标对象上创建一个标签
        lv_obj_t * l=lv_label_create(icon[i].obj);
        // 设置标签文本为图标索引
        lv_label_set_text_fmt(l,"%d",i);
        // 计算图标x坐标
        icon[i].x=(i-icon_count/2)*icon_distance;
        // 将图标居中显示
        lv_obj_center(icon[i].obj);
        // 如果是中间的图标，设置为大尺寸
        if(i == icon_count/2)
        {
            lv_obj_set_size(icon[i].obj,icon_size_big,icon_size_big);
        }
        else
        {
           // 否则设置为小尺寸
           lv_obj_set_size(icon[i].obj,icon_size_small,icon_size_small);
        }
 
        // 设置图标x坐标
        lv_obj_set_x(icon[i].obj,icon[i].x);
        // 为图标添加按下事件回调函数
        lv_obj_add_event_cb(icon[i].obj,pressing_cb,LV_EVENT_PRESSING,0);
        // 为图标添加释放事件回调函数
        lv_obj_add_event_cb(icon[i].obj,released_cb,LV_EVENT_RELEASED,0);
 
    }
}
 
// 按下事件回调函数
static void  pressing_cb(lv_event_t * e)
{
   static  lv_point_t click_point1,click_point2;
   int32_t v,i;
 
        // 如果当前未处于触摸状态
        if(touched == false)
        {
            for (i=0;i<icon_count;i++)
            {
               // 删除图标对象上的动画（如果有）
               lv_anim_del(icon[i].obj,set_x_cb);
            }
 
            // 获取当前输入设备的点击点坐标
            lv_indev_get_point(lv_indev_get_act(), &click_point1);
            // 设置触摸状态为已触摸
            touched = true;
            return;
         }
        else
        {
            // 如果已经处于触摸状态，获取当前点击点坐标
            lv_indev_get_point(lv_indev_get_act(), &click_point2);
        }
 
      // 计算触摸偏移量x
      t_offset_x = click_point2.x-click_point1.x;
      // 更新上一次点击点坐标
      click_point1.x = click_point2.x;
 
    for (int32_t i=0;i<icon_count;i++)
        {
            // 更新图标x坐标
            icon[i].x += t_offset_x;
            // 处理图标x坐标超出范围的情况（循环滚动）
            while(icon[i].x < (-icon_count/2)*icon_distance){icon[i].x += (icon_count)*icon_distance;}
            while(icon[i].x >  (icon_count/2)*icon_distance){icon[i].x -= (icon_count)*icon_distance;}
            // 设置图标对象的x坐标
            lv_obj_set_x(icon[i].obj,icon[i].x);
 
            // 如果图标x坐标超出一定范围，设置为小尺寸
            if(icon[i].x >= icon_distance || icon[i].x <= -icon_distance)
            {
                lv_obj_set_size(icon[i].obj,icon_size_small,icon_size_small);continue;
            }
 
            // 根据x坐标计算图标尺寸
            if(icon[i].x >= 0){v = icon[i].x;}
            else{
                v = -icon[i].x;
            }
            lv_obj_set_size(icon[i].obj,icon_size_small+(float)(icon_distance-v)/(float)icon_distance*(icon_size_big-icon_size_small),icon_size_small+(float)(icon_distance-v)/(float)icon_distance*(icon_size_big-icon_size_small));
        }
 
}
 
// 释放事件回调函数
static void  released_cb(lv_event_t * e)
{
    int32_t offset_x;
    offset_x=0;
    // 设置触摸状态为未触摸
    touched = false;
 
    for (int32_t i=0;i<icon_count;i++)
    {
        // 如果图标x坐标大于0
        if(icon[i].x>0)
        {
           // 根据x坐标与图标距离的关系计算偏移量
           if(icon[i].x % icon_distance > icon_distance/2)
           {
               offset_x=icon_distance-icon[i].x % icon_distance;
           }
           else
           {
               offset_x=-icon[i].x % icon_distance;
           }
            break;
        }
    }
 
    for (int32_t i=0;i<icon_count;i++)
    {
        // 创建动画，使图标回到合适位置
        lv_myanim_creat(icon[i].obj,set_x_cb,t_offset_x>0?300+t_offset_x*5:300-t_offset_x*5,0,lv_anim_path_ease_out,icon[i].x,icon[i].x+offset_x+t_offset_x/20*icon_distance,0,0);
        // 更新图标x坐标
        icon[i].x += offset_x+t_offset_x/20*icon_distance;
        // 处理图标x坐标超出范围的情况（循环滚动）
        while(icon[i].x < (-icon_count/2)*icon_distance){icon[i].x += (icon_count)*icon_distance;}
        while(icon[i].x >  (icon_count/2)*icon_distance){icon[i].x -= (icon_count)*icon_distance;}
    }
 
}
 
// 自定义动画创建函数
static void lv_myanim_creat(void * var,lv_anim_exec_xcb_t exec_cb,uint32_t time, uint32_t delay,lv_anim_path_cb_t path_cb,
                            int32_t start, int32_t end,lv_anim_ready_cb_t completed_cb,lv_anim_deleted_cb_t deleted_cb)
{
    lv_anim_t xxx;
    // 初始化动画对象
    lv_anim_init(&xxx);
    // 设置动画对象的变量
    lv_anim_set_var(&xxx,var);
    // 设置动画执行回调函数
    lv_anim_set_exec_cb(&xxx,exec_cb);
    // 设置动画时间
    lv_anim_set_time(&xxx,time);
    // 设置动画延迟
    lv_anim_set_delay(&xxx, delay);
    // 设置动画的起始值和结束值
    lv_anim_set_values(&xxx,start,end);
    // 如果有路径回调函数，设置路径回调函数
    if(path_cb) lv_anim_set_path_cb(&xxx,path_cb);
    // 如果有动画完成回调函数，设置动画完成回调函数
    if(completed_cb)lv_anim_set_ready_cb(&xxx,completed_cb);
    // 如果有动画删除回调函数，设置动画删除回调函数
    if(deleted_cb) lv_anim_set_deleted_cb(&xxx,deleted_cb);
    // 启动动画
    lv_anim_start(&xxx);
}
 
// 设置x坐标回调函数
static void set_x_cb(void * var, int32_t v)
{
    // 处理x坐标超出范围的情况（循环滚动）
    while(v < (-icon_count/2)*icon_distance){v += (icon_count)*icon_distance;}
    while(v >  (icon_count/2)*icon_distance){v -= (icon_count)*icon_distance;}
 
    // 设置对象的x坐标
    lv_obj_set_x(var,v);
 
    // 获取图标结构体指针
    icon_typedef * xxx=(icon_typedef *)(((lv_obj_t *)var)->user_data);
    // 更新图标结构体中的x坐标
    xxx->x=v;
 
    // 如果x坐标为0，设置图标为大尺寸
    if(v == 0)
    {
        lv_obj_set_size(var,icon_size_big,icon_size_big);return;
    }
 
    // 如果x坐标超出一定范围，设置图标为小尺寸
    if(v >= icon_distance || v <= -icon_distance)
    {
        lv_obj_set_size(var,icon_size_small,icon_size_small);return;
    }
 
    // 如果x坐标小于0，取绝对值
    if(v < 0)v = -v;
 
    // 根据x坐标计算图标尺寸并设置
    lv_obj_set_size(var,icon_size_small+(float)(icon_distance-v)/(float)icon_distance*(icon_size_big-icon_size_small),icon_size_small+(float)(icon_distance-v)/(float)icon_distance*(icon_size_big-icon_size_small));
 
}


void app_main(void)
{
    bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_DRAW_BUFF_SIZE,
        .double_buffer = BSP_LCD_DRAW_BUFF_DOUBLE,
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
            .sw_rotate = false, // must be set to true for software rotation
        }
    };

    lv_display_t *disp = bsp_display_start_with_config(&cfg);

    bsp_display_backlight_on();
    scrollicon();
    bsp_display_lock(0);

    bsp_display_unlock();
}