#include <gui/mainscreen_screen/MainScreenView.hpp>

#include <cassert>
#include <cstdio>
#include <cstring>

#include <utility>

#include "app_main.h"

extern "C" {
	void app_timeToSendUpdated(int8_t h, int8_t m, int8_t s);
	void app_connectButClicked();
	void app_sendTimeFlagChanged(bool isSending);
}

class MainScreenFriend
{
public:
	static void setScreen(MainScreenView* screen_p)
	{
		_screen_p = screen_p;
	}

	static void showStatus(const char* text)
	{
		if (_screen_p == nullptr)
		{
			return;
		}

		if (_screen_p->lock())
		{
			strncpy(_screen_p->_status, text, sizeof(_screen_p->_status));
			_screen_p->isUpdated = true;

			_screen_p->unlock();
		}
	}

	static void showSlaveInfo(int8_t h, int8_t m, int8_t s, bool isButDown, int8_t clickCount)
	{
		if (_screen_p == nullptr)
		{
			return;
		}

		if (_screen_p->lock())
		{
			_screen_p->_remoteTime.hour = h;
			_screen_p->_remoteTime.minute = m;
			_screen_p->_remoteTime.second = s;
			_screen_p->_isDown = isButDown;
			_screen_p->_clickCount = clickCount;

			_screen_p->isUpdated = true;

			_screen_p->unlock();
		}
	}

private:
	static MainScreenView* _screen_p;
};

MainScreenView* MainScreenFriend::_screen_p = nullptr;
static MainScreenFriend screenFriend;

extern "C"
{
	void app_display_showStatus(const char* text)
	{
		screenFriend.showStatus(text);
	}

	void app_display_showSlaveInfo(int8_t h, int8_t m, int8_t s, bool isButDown, int8_t clickCount)
	{
		screenFriend.showSlaveInfo(h, m, s, isButDown, clickCount);
	}
}

MainScreenView::MainScreenView()
{
	screenFriend.setScreen(this);

	size_t i = 0;

	_widgetBuffers[i++] = BufInfo { &labStatus, labStatusBuffer, LABSTATUS_SIZE };
	_widgetBuffers[i++] = BufInfo { &labHour, labHourBuffer,   LABHOUR_SIZE };
	_widgetBuffers[i++] = BufInfo { &labMinute, labMinuteBuffer, LABMINUTE_SIZE };
	_widgetBuffers[i++] = BufInfo { &labSecond, labSecondBuffer, LABSECOND_SIZE };
	_widgetBuffers[i++] = BufInfo { &labButDown, labButDownBuffer, LABBUTDOWN_SIZE };
	_widgetBuffers[i++] = BufInfo { &labClicks, labClicksBuffer,  LABCLICKS_SIZE };
	_widgetBuffers[i++] = BufInfo { &labRemoteTime, labRemoteTimeBuffer, LABREMOTETIME_SIZE };

	_widgetBuffers[i++] = BufInfo { nullptr, nullptr, 0 };

	_mutex = xSemaphoreCreateMutex();
}

MainScreenView::~MainScreenView()
{
	vSemaphoreDelete(_mutex);
}

void MainScreenView::handleTickEvent()
{
	if (lock())
	{
		if (isUpdated)
		{
			setValue(labStatus, _status);
			char timeText[16];
			if (timeToString(_remoteTime, timeText, sizeof(timeText)))
			{
				setValue(labRemoteTime, timeText);
			}
			setValue(labButDown, _isDown ? "True" : "False");
			setValue(labClicks, _clickCount);
			isUpdated = false;
		}
		unlock();
	}
	MainScreenViewBase::handleTickEvent();
}

bool MainScreenView::lock()
{
	const TickType_t delay_ticks = 100 / portTICK_PERIOD_MS;

	const auto result = xSemaphoreTake(_mutex, delay_ticks) == pdTRUE;
	return result;
}

void MainScreenView::unlock()
{
	xSemaphoreGive(_mutex);
}

void MainScreenView::setupScreen()
{
    MainScreenViewBase::setupScreen();
}

void MainScreenView::tearDownScreen()
{
    MainScreenViewBase::tearDownScreen();
}

void MainScreenView::butConnectClicked()
{
	app_connectButClicked();
}

void MainScreenView::butHourUpClicked()
{
	if (updateSpinValue(labHour, _timeToSend.hour, 1, _minHour, _maxHour))
	{
		callTimeUpdatedHandler();
	}
}

void MainScreenView::butHourDownClicked()
{
	if (updateSpinValue(labHour, _timeToSend.hour, -1, _minHour, _maxHour))
	{
		callTimeUpdatedHandler();
	}
}

void MainScreenView::butMinuteUpClicked()
{
	if (updateSpinValue(labMinute, _timeToSend.minute, 1, _minMinute, _maxMinute))
	{
		callTimeUpdatedHandler();
	}
}

void MainScreenView::butMinuteDownClicked()
{
	if (updateSpinValue(labMinute, _timeToSend.minute, -1, _minMinute, _maxMinute))
	{
		callTimeUpdatedHandler();
	}
}

void MainScreenView::butSecondUpClicked()
{
	if (updateSpinValue(labSecond, _timeToSend.second, 1, _minSecond, _maxSecond))
	{
		callTimeUpdatedHandler();
	}
}

void MainScreenView::butSecondDownClicked()
{
	if (updateSpinValue(labSecond, _timeToSend.second, -1, _minSecond, _maxSecond))
	{
		callTimeUpdatedHandler();
	}
}

void MainScreenView::butUpdateClicked()
{
	_isTimeUpdating = !_isTimeUpdating;
	app_sendTimeFlagChanged(_isTimeUpdating);
}

void MainScreenView::setValue(touchgfx::TextArea& textArea, uint32_t value)
{
	const auto bufInfo_p = getWidgetBuf(textArea);
	assert(bufInfo_p != nullptr);

	Unicode::snprintf(bufInfo_p->buf_p, bufInfo_p->bufLen, "%u", value);
	textArea.invalidate();
}

void MainScreenView::setValue(touchgfx::TextArea& textArea, const char* value)
{
	const int len = strlen(value);
	if ((len < 0) || (len > static_cast<int>(_tempBufLen)))
	{
		return;
	}

	const auto bufInfo_p = getWidgetBuf(textArea);
	if (bufInfo_p == nullptr)
	{
		return;
	}

	memset(_tempBuf, 0, sizeof(_tempBuf));

	Unicode::strncpy(_tempBuf, value, len);
	Unicode::snprintf(bufInfo_p->buf_p, bufInfo_p->bufLen, "%s", _tempBuf);

	textArea.invalidate();
}

bool MainScreenView::timeToString(const Time& time, char* buf_out_p, size_t bufSize) const
{
	const auto sz = std::snprintf(buf_out_p, bufSize, "%02d:%02d:%02d", time.hour, time.minute, time.second);
	if ((sz > 0) && (sz < static_cast<int>(bufSize)))
	{
		return true;
	}

	return false;
}

void MainScreenView::callTimeUpdatedHandler()
{
	app_timeToSendUpdated(_timeToSend.hour, _timeToSend.minute, _timeToSend.second);
}

bool MainScreenView::updateSpinValue(touchgfx::TextArea& textArea, int8_t& value, int8_t increment, int8_t limitMin, int8_t limitMax)
{
	auto mayUpdate = true;

	const auto result = static_cast<int32_t>(value) + increment;

	mayUpdate &= (result >= limitMin);
	mayUpdate &= (result <= limitMax);

	if (!mayUpdate)
	{
		return false;
	}

	value += increment;
	setValue(textArea, value);

	return true;
}

const MainScreenView::BufInfo* MainScreenView::getWidgetBuf(const touchgfx::TextArea& textArea) const
{
	for (size_t i = 0;; ++i)
	{
		if (_widgetBuffers[i].textArea_p == nullptr)
		{
			return nullptr;
		}

		if (_widgetBuffers[i].textArea_p == &textArea)
		{
			return &_widgetBuffers[i];
		}
	}
}
