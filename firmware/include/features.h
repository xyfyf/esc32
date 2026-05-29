/**
 * @file features.h
 * @brief Optional P3 feature toggles
 */
#ifndef ESC_FEATURES_H
#define ESC_FEATURES_H

/* Cyphal (UAVCAN v1) — see comm/cyphal/README.md */
#ifndef ESC_FEATURE_CYPHAL
#define ESC_FEATURE_CYPHAL 0
#endif

/* Motor beeps (power-on / link established / loss alarm) */
#ifndef ESC_FEATURE_MOTOR_BEEP
#define ESC_FEATURE_MOTOR_BEEP 1
#endif

#endif
