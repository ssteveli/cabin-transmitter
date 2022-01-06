// Thanks to https://gist.github.com/janjongboom/ed2994e754d89aa4f0a6b8a60fdba432 where I found this

#ifndef _DEBOUNCE_INTERRUPT_IN
#define _DEBOUNCE_INTERRUPT_IN

#if defined (DEVICE_INTERRUPTIN) || defined(DOXYGEN_ONLY)

#include "mbed.h"

namespace mbed {

class DebounceInterruptIn : private NonCopyable<DebounceInterruptIn> {
public:
    DebounceInterruptIn(PinName pinName, std::chrono::microseconds aDebounceTime) :
        pin(pinName), debounceTime(aDebounceTime)
    {}

    void fall(Callback<void()> cb) {
        fall_cb = cb;

        set_fall_isr();
    }

    void rise(Callback<void()> cb) {
        rise_cb = cb;

        set_rise_isr();
    }

    void mode(PinMode pull) {
        pin.mode(pull);
    }

    void enable_irq() {
        pin.enable_irq();
    }

    void disable_irq() {
        pin.disable_irq();
    }

    int read() {
        return pin.read();
    }

    operator int () {
        return pin.read();
    }

private:
    void set_rise_isr() {
        pin.rise(callback(this, &DebounceInterruptIn::rise_isr));
    }

    void set_fall_isr() {
        pin.fall(callback(this, &DebounceInterruptIn::fall_isr));
    }

    void rise_isr() {
        pin.rise(NULL);
        rise_timeout.attach(callback(this, &DebounceInterruptIn::set_rise_isr), debounceTime);

        rise_cb();
    }

    void fall_isr() {
        fall_cb();

        pin.fall(NULL);
        fall_timeout.attach(callback(this, &DebounceInterruptIn::set_fall_isr), debounceTime);
    }

    InterruptIn pin;
    std::chrono::microseconds debounceTime;

    Callback<void()> fall_cb;
    Callback<void()> rise_cb;
    Timeout fall_timeout;
    Timeout rise_timeout;
};

} // namespace mbed

#endif // MBED_INTERRUPT_IN

#endif // _DEBOUNCE_INTERRUPT_IN