/**
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is part of the mingw-w64 runtime package.
 * No warranty is given; refer to the file DISCLAIMER.PD within this package.
 */

#pragma once

#if __GNUC__ >=3
#pragma GCC system_header
#endif

#include <ksmedia-mingw.h>

typedef enum {
  LAST = KSPROPERTY_RTAUDIO_GETPOSITIONFUNCTION,
/* Additional structs for Windows Vista and later */
  KSPROPERTY_RTAUDIO_BUFFER,
  KSPROPERTY_RTAUDIO_HWLATENCY,
  KSPROPERTY_RTAUDIO_POSITIONREGISTER,
  KSPROPERTY_RTAUDIO_CLOCKREGISTER,
  KSPROPERTY_RTAUDIO_BUFFER_WITH_NOTIFICATION,
  KSPROPERTY_RTAUDIO_REGISTER_NOTIFICATION_EVENT,
  KSPROPERTY_RTAUDIO_UNREGISTER_NOTIFICATION_EVENT
} KSPROPERTY_RTAUDIO_1;

/* Additional structs for Windows Vista and later */
typedef struct _tagKSRTAUDIO_BUFFER_PROPERTY {
    KSPROPERTY  Property;
    PVOID       BaseAddress;
    ULONG       RequestedBufferSize;
} KSRTAUDIO_BUFFER_PROPERTY, *PKSRTAUDIO_BUFFER_PROPERTY;

typedef struct _tagKSRTAUDIO_BUFFER_PROPERTY_WITH_NOTIFICATION {
    KSPROPERTY  Property;
    PVOID       BaseAddress;
    ULONG       RequestedBufferSize;
    ULONG       NotificationCount;
} KSRTAUDIO_BUFFER_PROPERTY_WITH_NOTIFICATION, *PKSRTAUDIO_BUFFER_PROPERTY_WITH_NOTIFICATION;

typedef struct _tagKSRTAUDIO_BUFFER {
    PVOID   BufferAddress;
    ULONG   ActualBufferSize;
    BOOL    CallMemoryBarrier;
} KSRTAUDIO_BUFFER, *PKSRTAUDIO_BUFFER;

typedef struct _tagKSRTAUDIO_HWLATENCY {
    ULONG   FifoSize;
    ULONG   ChipsetDelay;
    ULONG   CodecDelay;
} KSRTAUDIO_HWLATENCY, *PKSRTAUDIO_HWLATENCY;

typedef struct _tagKSRTAUDIO_HWREGISTER_PROPERTY {
    KSPROPERTY  Property;
    PVOID       BaseAddress;
} KSRTAUDIO_HWREGISTER_PROPERTY, *PKSRTAUDIO_HWREGISTER_PROPERTY;

typedef struct _tagKSRTAUDIO_HWREGISTER {
    PVOID       Register;
    ULONG       Width;
    ULONGLONG   Numerator;
    ULONGLONG   Denominator;
    ULONG       Accuracy;
} KSRTAUDIO_HWREGISTER, *PKSRTAUDIO_HWREGISTER;

typedef struct _tagKSRTAUDIO_NOTIFICATION_EVENT_PROPERTY {
    KSPROPERTY  Property;
    HANDLE      NotificationEvent;
} KSRTAUDIO_NOTIFICATION_EVENT_PROPERTY, *PKSRTAUDIO_NOTIFICATION_EVENT_PROPERTY;

/* Added for Vista and later */
#define STATIC_KSCATEGORY_REALTIME \
    0xEB115FFCL, 0x10C8, 0x4964, 0x83, 0x1D, 0x6D, 0xCB, 0x02, 0xE6, 0xF2, 0x3F
DEFINE_GUIDSTRUCT("EB115FFC-10C8-4964-831D-6DCB02E6F23F", KSCATEGORY_REALTIME);
#define KSCATEGORY_REALTIME DEFINE_GUIDNAMED(KSCATEGORY_REALTIME)


