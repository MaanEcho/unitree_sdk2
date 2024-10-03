#pragma once

#include <stdint.h>
#include <cmath>
#include "unitree/idl/go2/WirelessController_.hpp"
// #include <go2_idl/WirelessController_.hpp>

namespace unitree::common
{
    // union for keys
    // 定义遥控器键值和按钮之间的对应关系
    typedef union
    {
        struct
        {
            uint8_t R1 : 1;
            uint8_t L1 : 1;
            uint8_t start : 1;
            uint8_t select : 1;
            uint8_t R2 : 1;
            uint8_t L2 : 1;
            uint8_t F1 : 1;
            uint8_t F2 : 1;
            uint8_t A : 1;
            uint8_t B : 1;
            uint8_t X : 1;
            uint8_t Y : 1;
            uint8_t up : 1;
            uint8_t right : 1;
            uint8_t down : 1;
            uint8_t left : 1;
        } components;
        uint16_t value;
    } xKeySwitchUnion;

    // single button class
    // 按钮类，用于表示单个按钮的状态
    class Button
    {
    public:
        Button() {}

        void update(bool state)
        {
            on_press = state ? state != pressed : false;
            on_release = state ? false : state != pressed;
            pressed = state;
        }

        bool pressed = false;
        // 表示该按钮当前是否处于按下状态
        bool on_press = false;
        // 表示该按钮当前是否恰好处于按下状态
        bool on_release = false;
        // 表示该按钮当前是否处于松开状态
    };

    // full gamepad
    // 完整的遥控器类，用于表示遥控器的状态
    class Gamepad
    {
    public:
        Gamepad() {}

        void Update(unitree_go::msg::dds_::WirelessController_ &key_msg)
        {
            // update stick values with smooth and deadzone
            // 采用平滑和死区处理摇杆值
            lx = lx * (1 - smooth) + (std::fabs(key_msg.lx()) < dead_zone ? 0.0 : key_msg.lx()) * smooth;
            rx = rx * (1 - smooth) + (std::fabs(key_msg.rx()) < dead_zone ? 0.0 : key_msg.rx()) * smooth;
            ry = ry * (1 - smooth) + (std::fabs(key_msg.ry()) < dead_zone ? 0.0 : key_msg.ry()) * smooth;
            ly = ly * (1 - smooth) + (std::fabs(key_msg.ly()) < dead_zone ? 0.0 : key_msg.ly()) * smooth;
            // 通过这种方式，摇杆值不会突然变化，而是根据 smooth 参数逐渐过渡到新的位置，同时小幅度的无意识移动会被忽略，因为它们处于 dead_zone 之内。这样的处理可以提高玩家对游戏的控制感，并减少不必要的微小抖动。
,
            // update button states
            // 更新按钮状态
            key.value = key_msg.keys();

            R1.update(key.components.R1);
            L1.update(key.components.L1);
            start.update(key.components.start);
            select.update(key.components.select);
            R2.update(key.components.R2);
            L2.update(key.components.L2);
            F1.update(key.components.F1);
            F2.update(key.components.F2);
            A.update(key.components.A);
            B.update(key.components.B);
            X.update(key.components.X);
            Y.update(key.components.Y);
            up.update(key.components.up);
            right.update(key.components.right);
            down.update(key.components.down);
            left.update(key.components.left);
        }

        float smooth = 0.03;
        // smooth：这是一个介于0到1之间的平滑因子，用来控制新旧摇杆值的混合程度。当 smooth 接近0时，摇杆值变化会比较突兀；当 smooth 接近1时，摇杆值变化会更加平滑。
        float dead_zone = 0.01;
        // dead_zone：这是死区阈值，用于定义一个区间，在这个区间内摇杆被认为没有移动。如果摇杆的值在这个区间内，则将其视为0。

        float lx = 0.;
        float rx = 0.;
        float ly = 0.;
        float ry = 0.;

        Button R1;
        Button L1;
        Button start;
        Button select;
        Button R2;
        Button L2;
        Button F1;
        Button F2;
        Button A;
        Button B;
        Button X;
        Button Y;
        Button up;
        Button right;
        Button down;
        Button left;

    private:
        xKeySwitchUnion key;
    };
} // namespace unitree::common