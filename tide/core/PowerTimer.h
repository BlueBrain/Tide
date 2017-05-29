#ifndef POWERTIMER_H
#define POWERTIMER_H

#include "serialization/includes.h"
#include "types.h"

#include <boost/enable_shared_from_this.hpp>

#include <QObject>
#include <QTimer>

/**
 * Inform user about inactivity timeout and allow to stop the power off process.
 */
class PowerTimer : public QObject,
                   public boost::enable_shared_from_this<PowerTimer>
{
    Q_OBJECT
    Q_DISABLE_COPY(PowerTimer)

public:
    /**
     * Construct a timer used to power off Planar equipment.
     * @param timeout value of the timer in ms.
     */
    PowerTimer(const int timeout);

    /** Default constructor */
    PowerTimer();

    /** Get the duration of countdown needed for transition in qml */
    Q_INVOKABLE int getCountdownTimeout();

    /** Check if timer is active */
    Q_INVOKABLE bool isActive();

    /** Start the timer */
    void start();

    /** Stop the timer */
    void stop();

public slots:
    /** Interrupt the timer and prevent power off of Planar equipment */
    void reset();

signals:
    /** Emitted countdown timer times-out */
    void poweroff();

    /** Emitted when state of timer is modified */
    void updated(PowerTimerPtr);

private:
    friend class boost::serialization::access;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        // clang-format off
        ar & _active;
        ar & _countdownTimeout;

        // clang-format on
    }

    bool _active = false;
    int _countdownTimeout = 15000;
    int _timeout;

    std::unique_ptr<QTimer> _countDownTimer;
    std::unique_ptr<QTimer> _inactivityTimer;
};
#endif
