/**
 * @file features.h
 * @brief P3 可选特性开关
 */
#ifndef ESC_FEATURES_H
#define ESC_FEATURES_H

/* Cyphal (UAVCAN v1) — 见 comm/cyphal/README.md */
#ifndef ESC_FEATURE_CYPHAL
#define ESC_FEATURE_CYPHAL 0
#endif

/* 电机提示音（上电 / 联机 / 丢失报警） */
#ifndef ESC_FEATURE_MOTOR_BEEP
#define ESC_FEATURE_MOTOR_BEEP 1
#endif

#endif
