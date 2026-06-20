/***************************************************************************
 *
 * Copyright 2015-2021 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/


/**
 * DOC  - Definition of terms.  Various "sleep" and low power consumption states
 * have various names and are sometimes used in conflicting ways.  In the
 * PM logic, we will use the following terminology:
 *
 * NORMAL  - The normal, full power operating mode.
 * IDLE    - This is still basically normal operational mode, the system is,
 *           however, IDLE and some simple simple steps to reduce power
 *           consumption provided that they do not interfere with normal
 *           Operation.  Simply dimming a backlight might be an example
 *           something that would be done when the system is idle.
 * STANDBY - Standby is a lower power consumption mode that may involve more
 *           extensive power management steps such has disabling clocking or
 *           setting the processor into reduced power consumption modes. In
 *           this state, the system should still be able to resume normal
 *           activity almost immediately.
 * SLEEP   - The lowest power consumption mode.  The most drastic power
 *           reduction measures possible should be taken in this state. It
 *           may require some time to get back to normal operation from
 *           SLEEP (some MCUs may even require going through reset).
 *
 * State changes always proceed from higher to lower power usage:
 *
 * NORMAL->IDLE->STANDBY->SLEEP
 *   ^       |      |        |
 *   |       V      V        V
 *   +-------+------+--------+
 */

#ifndef __INCLUDE_POWER_PM_H
#define __INCLUDE_POWER_PM_H

/**
 * Included Files
 */

#ifdef CONFIG_PM

/**
 * Pre-processor Definitions
 */

/**
 *  CONFIG_PM_NDOMAINS - Defines the number of "domains" that activity may be
 * monitored on.  For example, you may want to separately manage the power
 * from the Network domain, shutting down the network when it is not be used,
 * from the UI domain, shutting down the UI when it is not in use.
 */

#ifndef CONFIG_PM_NDOMAINS
#define CONFIG_PM_NDOMAINS 1
#endif

#if CONFIG_PM_NDOMAINS < 1
#error CONFIG_PM_NDOMAINS invalid
#endif

/**
 * Public Types
 */

/**
 * enum pm_state_e  -  This enumeration provides all power management states.  Receipt of the
 * state indication is the state transition event.
 */

enum pm_state_e {
    PM_RESTORE = -1,    /* PM_RESTORE is not a low power state.
                 *
                 * PM_RESTORE is used to notify for restore from low power
                 * state.
                 */
    PM_NORMAL = 0,        /* Normal full power operating mode.  If the driver is in
                 * a reduced power usage mode, it should immediately re-
                 * initialize for normal operatin.
                 *
                 * PM_NORMAL may be followed by PM_IDLE.
                 */
    PM_IDLE,        /* Drivers will receive this state change if it is
                 * appropriate to enter a simple IDLE power state.  This
                 * would include simple things such as reducing display
                 * back-lighting.  The driver should be ready to resume
                 * normal activity instantly.
                 *
                 * PM_IDLE may be followed by PM_STANDBY or PM_NORMAL.
                 */
    PM_STANDBY,        /* The system is entering standby mode. Standby is a lower
                 * power consumption mode that may involve more extensive
                 * power management steps such has disabling clocking or
                 * setting the processor into reduced power consumption
                 * modes. In this state, the system should still be able
                 * to resume normal activity almost immediately.
                 *
                 * PM_STANDBY may be followed PM_SLEEP or by PM_NORMAL
                 */
    PM_SLEEP,        /* The system is entering deep sleep mode.  The most
                 * drastic power reduction measures possible should be
                 * taken in this state. It may require some time to get
                 * back to normal operation from SLEEP (some MCUs may
                 * even require going through reset).
                 *
                 * PM_SLEEP may be following by PM_NORMAL
                 */
    PM_COUNT,
};

struct dq_entry_s {
    FAR struct dq_entry_s *flink;
    FAR struct dq_entry_s *blink;
};

/**
 * struct pm_callback_s - This structure contain pointers callback functions in the driver.
 * These callback functions can be used to provide power management information
 * to the driver.
 */

struct pm_callback_s {
    struct dq_entry_s entry;    /* Supports a doubly linked list */

  /**
   * prepare  -  Request the driver to prepare for a new power state.
   *   @cb      : Returned to the driver.  The driver version of the callback
   *             structure may include additional, driver-specific state
   *             data at the end of the structure.
   *   @domain  : Identifies the activity domain of the state change
   *   @pmstate : Identifies the new PM state
   *
   *   Returs:  0 (OK) - means the event was successfully processed and that the driver
   *   is prepared for the PM state change.
   *   Non-zero  - means that the driver is not prepared to perform the tasks needed
   *   achieve this power setting and will cause the state change to be aborted.
   *
   *   NOTE:  The prepare
   *   method will also be recalled when reverting from lower back to higher
   *   power consumption modes (say because another driver refused a lower
   *   power state change).  Drivers are not permitted to return non-zero
   *   values when reverting back to higher power consumption modes!
   *
   **************************************************************************/

    int (*prepare) (FAR struct pm_callback_s * cb, int domain,
            enum pm_state_e pmstate);

  /**
   * notify  - Notify the driver of new power state
   * @cb      - Returned to the driver.  The driver version of the callback
   *             structure may include additional, driver-specific state
   *             data at the end of the structure.
   * @domain  - Identifies the activity domain of the state change
   * @pmstate - Identifies the new PM state
   *
   */

    void (*notify) (FAR struct pm_callback_s * cb, int domain,
            enum pm_state_e pmstate);
};

/**
 * struct pm_governor_s - An instance of a given PM governor */

struct pm_governor_s {
  /**
   * initialize - Invoked by the PM system during initialization
   *
   *   NOTE: since this will be called from pm_initialize(), the system
   *   is in very early boot state when this callback is invoked. Thus,
   *   only very basic initialization should be performed (e.g. no memory
   *   should be allocated).
   *
   */

    void (*initialize) (void);

  /**
   * statechanged  - Invoked by the PM system when the power state is changed.
   *
   * This can be left to NULL if this notification is not needed by the governor.
   *
   */

    void (*statechanged) (int domain, enum pm_state_e newstate);

  /**
   * checkstate - Invoked by the PM system to obtain the governor's suggestion for the
   *   power level to set. Implementing this callback is mandatory for a
   *   governor since recommending power levels is its main responsibility.
   *
   *   NOTE: the governor should consider the "stay" count in order to hold
   *   off the recommendation of a lower-power level.
   *
   */

    enum pm_state_e (*checkstate) (int domain);

  /**
   * activity  - Invoked by the PM system when a driver reports activity via
   *   pm_activity().
   *
   * This may or may not be useful to the governor to recommend power levels.
   *   It can be left NULL, in which case it will not be invoked.
   *
   */

    void (*activity) (int domain, int count);

    /* Private data for governor's internal use */

    FAR void *priv;
};

/**
 * struct pm_user_governor_state_s - To be used for accessing the user governor via ioctl calls
 */

struct pm_user_governor_state_s {
    int domain;
    enum pm_state_e state;
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifndef __ASSEMBLY__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Public Function Prototypes
 */

/**
 * pm_initialize
 *
 *   This function is called by MCU-specific logic at power-on reset in
 *   order to provide one-time initialization the power management subsystem.
 *   This function must be called *very* early in the initialization sequence
 *   *before* any other device drivers are initialized (since they may
 *   attempt to register with the power management subsystem).
 *
 */

void pm_initialize(void);

/**
 * pm_register - is called by a device driver in order to register to
 *   receive power management event callbacks.
 * @callbacks : An instance of struct pm_callback_s providing the driver
 *               callback functions.
 *
 * Returns: Zero (OK) on success; otherwise a negated errno value is returned.
 *
 */

int pm_register(FAR struct pm_callback_s *callbacks);

/**
 * pm_unregister - is called by a device driver in order to unregister
 *   previously registered power management event callbacks.
 * @callbacks : An instance of struct pm_callback_s providing the driver
 *               callback functions.
 *
 * Returns:  Zero (OK) on success; otherwise a negated errno value is returned.
 *
 */

int pm_unregister(FAR struct pm_callback_s *callbacks);

/**
 * pm_activity   -  is called by a device driver to indicate that it is
 *   performing meaningful activities (non-idle).  This is reported to
 *   the PM governor, which may be used to suggest power states.
 * @domain : The domain of the PM activity
 * @priority : Activity priority, range 0-9.  Larger values correspond to
 *     higher priorities.  Higher priority activity can prevent the system
 *     from entering reduced power states for a longer period of time.
 *
 * As an example, a button press might be higher priority activity
 * because it means that the user is actively interacting with the
 * device.
 *
 * Assumptions:
 *   This function may be called from an interrupt handler (this is the ONLY
 *   PM function that may be called from an interrupt handler!).
 *
 */

void pm_activity(int domain, int priority);

/**
 * pm_stay   -  is called by a device driver to indicate that it is
 *   performing meaningful activities (non-idle), needs the power kept at
 *   least the specified level.
 * @domain - The domain of the PM activity
 * @state - The state want to stay.
 *
 * As an example, media player might stay in normal state during
 * playback.
 *
 * Assumptions:
 *   This function may be called from an interrupt handler.
 *
 */

void pm_stay(int domain, enum pm_state_e state);

/**
 * pm_relax - is called by a device driver to indicate that it is
 *   idle now, could relax the previous requested power level.
 * @domain : The domain of the PM activity
 * @state : The state want to relax.
 *
 * As an example, media player might relax power level after playback.
 *
 * Assumptions:
 *   This function may be called from an interrupt handler.
 *
 */

void pm_relax(int domain, enum pm_state_e state);

/**
 * pm_staycount  - is called to get current stay count.
 * @domain ： The domain of the PM activity
 * @state ： The state want to relax.
 *
 * Returns:  Current pm stay count
 *
 * Assumptions:
 *   This function may be called from an interrupt handler.
 *
 */

uint32_t pm_staycount(int domain, enum pm_state_e state);

/**
 * pm_checkstate -  is called from the MCU-specific IDLE loop to monitor the
 *   the power management conditions
 * @domain - The PM domain to check
 *
 * This function is called from the MCU-specific IDLE loop to monitor the
 *   the power management conditions.  This function returns the
 *   "recommended" power management state based on the PM configuration and
 *   activity reported in the last sampling periods.  The power management
 *   state is not automatically changed, however.  The IDLE loop must call
 *   pm_changestate() in order to make the state change.
 *
 *   These two steps are separated because the platform-specific IDLE loop
 *   may have additional situational information that is not available to
 *   the the PM sub-system.  For example, the IDLE loop may know that the
 *   battery charge level is very low and may force lower power states
 *   even if there is activity.
 *
 *   NOTE: That these two steps are separated in time and, hence, the IDLE
 *   loop could be suspended for a long period of time between calling
 *   pm_checkstate() and pm_changestate().  The IDLE loop may need to make
 *   these calls atomic by either disabling interrupts until the state change
 *   is completed.
 *
 *
 * Returns: The recommended power management state.
 *
 */

enum pm_state_e pm_checkstate(int domain);

/**
 * pm_changestate - is used to platform-specific power management logic.  It
 *   will announce the power management power management state change to all
 *   drivers that have registered for power management event callbacks.
 * @domain : Identifies the domain of the new PM state
 *   newstate - Identifies the new PM state
 *
 * Returns :
 *   0 (OK) means that the callback function for all registered drivers
 *   returned OK (meaning that they accept the state change).  Non-zero
 *   means that one of the drivers refused the state change.  In this case,
 *   the system will revert to the preceding state.
 *
 * Assumptions:
 *   It is assumed that interrupts are disabled when this function is
 *   called.  This function is probably called from the IDLE loop... the
 *   lowest priority task in the system.  Changing driver power management
 *   states may result in renewed system activity and, as a result, can
 *   suspend the IDLE thread before it completes the entire state change
 *   unless interrupts are disabled throughout the state change.
 *
 */

int pm_changestate(int domain, enum pm_state_e newstate);

/**
 * pm_querystate  - returns the current power management state.
 * @domain  : The PM domain to check
 *
 * Returns: The current power management state.
 *
 */

enum pm_state_e pm_querystate(int domain);

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLY__ */

/**
 * Stubs
 */

#else /* CONFIG_PM */

/**
 *  Stubbed out versions of all of PM interface functions that may be used to
 * avoid so much conditional compilation in driver code when PM is disabled:
 */

#define pm_initialize()
#define pm_register(cb)              (0)
#define pm_unregister(cb)            (0)
#define pm_activity(domain,prio)
#define pm_checkstate(domain)        (0)
#define pm_changestate(domain,state) (0)
#define pm_querystate(domain)        (0)

#endif /* CONFIG_PM */



#endif /* __INCLUDE_POWER_PM_H */
