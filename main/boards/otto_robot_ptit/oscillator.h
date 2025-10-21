#ifndef __OSCILLATOR_H__
#define __OSCILLATOR_H__

#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define M_PI 3.14159265358979323846

#ifndef DEG2RAD
#define DEG2RAD(g) ((g) * M_PI) / 180
#endif

#define SERVO_MIN_PULSEWIDTH_US 500           // Độ rộng xung tối thiểu (micro giây)
#define SERVO_MAX_PULSEWIDTH_US 2500          // Độ rộng xung tối đa (micro giây)
#define SERVO_MIN_DEGREE -90                  // Góc tối thiểu
#define SERVO_MAX_DEGREE 90                   // Góc tối đa
#define SERVO_TIMEBASE_RESOLUTION_HZ 1000000  // 1MHz, 1us per tick
#define SERVO_TIMEBASE_PERIOD 20000           // 20000 ticks, 20ms

class Oscillator {
public:
    Oscillator(int trim = 0);
    ~Oscillator();
    void Attach(int pin, bool rev = false);
    void Detach();

    void SetA(unsigned int amplitude) { amplitude_ = amplitude; };
    void SetO(int offset) { offset_ = offset; };
    void SetPh(double Ph) { phase0_ = Ph; };
    void SetT(unsigned int period);
    void SetTrim(int trim) { trim_ = trim; };
    void SetLimiter(int diff_limit) { diff_limit_ = diff_limit; };
    void DisableLimiter() { diff_limit_ = 0; };
    int GetTrim() { return trim_; };
    void SetPosition(int position);
    void Stop() { stop_ = true; };
    void Play() { stop_ = false; };
    void Reset() { phase_ = 0; };
    void Refresh();
    int GetPosition() { return pos_; }

private:
    bool NextSample();
    void Write(int position);
    uint32_t AngleToCompare(int angle);

private:
    bool is_attached_;

    //-- Tham số dao động
    unsigned int amplitude_;  //-- Biên độ (độ)
    int offset_;              //-- Độ lệch (độ)
    unsigned int period_;     //-- Chu kỳ (mili giây)
    double phase0_;           //-- Pha (radian)

    //-- Biến nội bộ
    int pos_;                       //-- Vị trí servo hiện tại
    int pin_;                       //-- Chân mà servo được kết nối
    int trim_;                      //-- Độ lệch hiệu chuẩn
    double phase_;                  //-- Pha hiện tại
    double inc_;                    //-- Tăng pha
    double number_samples_;         //-- Số mẫu
    unsigned int sampling_period_;  //-- Chu kỳ lấy mẫu (ms)

    long previous_millis_;
    long current_millis_;

    //-- Chế độ dao động. Nếu true, servo dừng
    bool stop_;

    //-- Chế độ đảo ngược
    bool rev_;

    int diff_limit_;
    long previous_servo_command_millis_;

    ledc_channel_t ledc_channel_;
    ledc_mode_t ledc_speed_mode_;
};

#endif  // __OSCILLATOR_H__