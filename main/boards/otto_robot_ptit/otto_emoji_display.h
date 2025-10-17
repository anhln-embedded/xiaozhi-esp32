#pragma once

#include <libs/gif/lv_gif.h>

#include "display/lcd_display.h"
#include "otto_emoji_gif.h"

/**
 * @brief Otto机器人GIF表情显示类
 * 继承LcdDisplay，添加GIF表情支持
 */
class OttoEmojiDisplay : public SpiLcdDisplay {
public:
    /**
     * @brief 构造函数，参数与SpiLcdDisplay相同
     */
    OttoEmojiDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, int width,
                     int height, int offset_x, int offset_y, bool mirror_x, bool mirror_y,
                     bool swap_xy);

    virtual ~OttoEmojiDisplay() = default;

    // Ghi đè phương thức thiết lập biểu cảm
    virtual void SetEmotion(const char* emotion) override;

    // Ghi đè phương thức thiết lập tin nhắn chat
    virtual void SetChatMessage(const char* role, const char* content) override;

private:
    void SetupGifContainer();

    lv_obj_t* emotion_gif_;  ///< Thành phần GIF biểu cảm

    // Ánh xạ biểu cảm
    struct EmotionMap {
        const char* name;
        const lv_img_dsc_t* gif;
    };

    static const EmotionMap emotion_maps_[];
};