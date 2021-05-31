/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StSlotTypes_h_
#define __StSlotTypes_h_

#include <stTypes.h>

/**
 * Empty template definition. Templates concretizations are below.
 * This structure template provides general types definitions used in slots.
 */
template<typename class_t, typename slotMethod_t>
struct StSlotTypes;

/**
 * Concretization for slots without arguments.
 */
template<typename class_t>
struct StSlotTypes<class_t, void ()> {

    typedef stNoType arg1_t;  //!< dummy type definition
    typedef stNoType arg2_t;  //!< dummy type definition
    typedef stNoType arg3_t;  //!< dummy type definition
    typedef void (*func_t)();               //!< Slot-function type definition
    typedef void (class_t::*method_t)();    //!< Slot-method type definition
    typedef void (*methodUnsafe_t)(void* ); //!< Unsafe Slot-method type definition

};

/**
 * Concretization for slots with 1 argument.
 */
template<typename class_t, typename arg1Type>
struct StSlotTypes<class_t, void (arg1Type)> {

    typedef arg1Type arg1_t; //!< type definition for the 1st argument in callback Slot
    typedef stNoType arg2_t; //!< dummy type definition
    typedef stNoType arg3_t; //!< dummy type definition
    typedef void (*func_t)(arg1_t);                //!< Slot-function type definition
    typedef void (class_t::*method_t)(arg1_t);     //!< Slot-method type definition
    typedef void (*methodUnsafe_t)(void*, arg1_t); //!< Unsafe Slot-method type definition

};

/**
 * Concretization for slots with 2 arguments.
 */
template<typename class_t, typename arg1Type, typename arg2Type>
struct StSlotTypes<class_t, void (arg1Type, arg2Type)> {

    typedef arg1Type arg1_t; //!< type definition for the 1st argument in callback Slot
    typedef arg2Type arg2_t; //!< type definition for the 2nd argument in callback Slot
    typedef stNoType arg3_t; //!< dummy type definition
    typedef void (*func_t)(arg1_t, arg2_t);                //!< Slot-function type definition
    typedef void (class_t::*method_t)(arg1_t, arg2_t);     //!< Slot-method type definition
    typedef void (*methodUnsafe_t)(void*, arg1_t, arg2_t); //!< Unsafe Slot-method type definition

};

/**
 * Concretization for slots with 3 arguments.
 */
template<typename class_t, typename arg1Type, typename arg2Type, typename arg3Type>
struct StSlotTypes<class_t, void (arg1Type, arg2Type, arg3Type)> {

    typedef arg1Type arg1_t; //!< type definition for the 1st argument in callback Slot
    typedef arg2Type arg2_t; //!< type definition for the 2nd argument in callback Slot
    typedef arg3Type arg3_t; //!< type definition for the 3rd argument in callback Slot
    typedef void (*func_t)(arg1_t, arg2_t, arg3_t);                //!< Slot-function type definition
    typedef void (class_t::*method_t)(arg1_t, arg2_t, arg3_t);     //!< Slot-method type definition
    typedef void (*methodUnsafe_t)(void*, arg1_t, arg2_t, arg3_t); //!< Unsafe Slot-method type definition

};

#endif //__StSlotTypes_h_
