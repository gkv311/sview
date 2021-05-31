/**
 * Copyright © 2009-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StSlot_h_
#define __StSlot_h_

/**
 * Empty template definition. Templates concretizations are below.
 * This class template provides Slots pointers - receivers of the Signals.
 * Defined as virtual class.
 */
template<typename slotMethod_t>
class StSlot;

/**
 * Concretization for Slots without arguments.
 */
template<>
class StSlot<void ()> {

        public:

    StSlot() {}
    virtual ~StSlot() {}

    /**
     * Virtual method to validate the callback Slot.
     */
    virtual bool isValid() const = 0;

    /**
     * Virtual method to trigger the callback function.
     */
    virtual bool call() const = 0;

    /**
     * Virtual method to compare slots.
     */
    virtual bool isEqual(const StSlot& theOther) const = 0;

};

/**
 * Concretization for Slots with 1 argument.
 */
template<typename arg1_t>
class StSlot<void (arg1_t )> {

        public:

    StSlot() {}
    virtual ~StSlot() {}

    /**
     * Virtual method to validate the callback Slot.
     */
    virtual bool isValid() const = 0;

    /**
     * Virtual method to trigger the callback function.
     */
    virtual bool call(arg1_t arg1) const = 0;

    /**
     * Virtual method to compare slots.
     */
    virtual bool isEqual(const StSlot& theOther) const = 0;

};

/**
 * Concretization for Slots with 2 arguments.
 */
template<typename arg1_t, typename arg2_t>
class StSlot<void (arg1_t , arg2_t )> {

        public:

    StSlot() {}
    virtual ~StSlot() {}

    /**
     * Virtual method to validate the callback Slot.
     */
    virtual bool isValid() const = 0;

    /**
     * Virtual method to trigger the callback function.
     */
    virtual bool call(arg1_t arg1, arg2_t arg2) const = 0;

    /**
     * Virtual method to compare slots.
     */
    virtual bool isEqual(const StSlot& theOther) const = 0;

};

/**
 * Concretization for Slots with 3 arguments.
 */
template<typename arg1_t, typename arg2_t, typename arg3_t>
class StSlot<void (arg1_t , arg2_t , arg3_t )> {

        public:

    StSlot() {}
    virtual ~StSlot() {}

    /**
     * Virtual method to validate the callback Slot.
     */
    virtual bool isValid() const = 0;

    /**
     * Virtual method to trigger the callback function.
     */
    virtual bool call(arg1_t arg1, arg2_t arg2, arg3_t arg3) const = 0;

    /**
     * Virtual method to compare slots.
     */
    virtual bool isEqual(const StSlot& theOther) const = 0;

};

#endif //__StSlot_h_
