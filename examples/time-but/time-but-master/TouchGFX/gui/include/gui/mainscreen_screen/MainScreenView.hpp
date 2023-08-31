#ifndef MAINSCREENVIEW_HPP
#define MAINSCREENVIEW_HPP

#include <cstddef>
#include <cstdint>
#include <cstdbool>

#include <map>

#include "FreeRTOS.h"
#include "semphr.h"

#include <gui_generated/mainscreen_screen/MainScreenViewBase.hpp>
#include <gui/mainscreen_screen/MainScreenPresenter.hpp>

class MainScreenView : public MainScreenViewBase
{
	friend class MainScreenFriend;
public:
    MainScreenView();
    virtual ~MainScreenView();
    virtual void setupScreen();
    virtual void tearDownScreen();

    void butConnectClicked() override;
    void butHourUpClicked() override;
    void butHourDownClicked() override;
    void butMinuteUpClicked() override;
    void butMinuteDownClicked() override;
    void butSecondUpClicked() override;
    void butSecondDownClicked() override;
    void butUpdateClicked() override;

    void handleTickEvent() override;

private:
    struct BufInfo
	{
    	const touchgfx::TextArea* textArea_p;
    	touchgfx::Unicode::UnicodeChar* buf_p;
    	size_t bufLen;
	};

    struct Time
    {
    	int8_t hour;
    	int8_t minute;
    	int8_t second;
    };

    bool lock();
    void unlock();

    void callTimeUpdatedHandler();
    bool updateSpinValue(touchgfx::TextArea& textArea, int8_t& value, int8_t increment, int8_t limitMin, int8_t limitMax);

    const BufInfo* getWidgetBuf(const touchgfx::TextArea& textArea) const;

    void setValue(touchgfx::TextArea& textArea, uint32_t value);
    void setValue(touchgfx::TextArea& textArea, const char* value);

    bool timeToString(const Time& time, char* buf_out_p, size_t bufSize) const;

    Time _timeToSend = {0};
    bool _isTimeUpdating = false;

    bool isUpdated = false;
    char _status[64];
    Time _remoteTime = {0};
    uint32_t _clickCount = 0;
    bool _isDown = false;

    BufInfo _widgetBuffers[8];

    static constexpr int8_t _minHour = 0;
    static constexpr int8_t _maxHour = 23;

    static constexpr int8_t _minMinute = 0;
    static constexpr int8_t _maxMinute = 59;

    static constexpr int8_t _minSecond = 0;
    static constexpr int8_t _maxSecond = 59;

    SemaphoreHandle_t _mutex = nullptr;

    static constexpr size_t _tempBufLen = 32;
    Unicode::UnicodeChar _tempBuf[_tempBufLen];
};

#endif // MAINSCREENVIEW_HPP
