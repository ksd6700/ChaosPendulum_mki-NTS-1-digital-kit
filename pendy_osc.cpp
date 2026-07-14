/*
  Pendy: double-pendulum Y-axis oscillator for KORG Nu:Tekt NTS-1 mk1.

  Copy this project into logue-sdk/platform/nutekt-digital/pendy_osc
  and build it like a normal nutekt-digital custom oscillator project.
*/

#include <math.h>
#include <stdint.h>

#if defined(PENDY_HOST_TEST) || defined(PENDY_HOST_TEST_MAIN)
struct user_osc_param_t
{
  uint16_t pitch = 60u << 8;
  int32_t shape_lfo = 0;
};

enum
{
  k_user_osc_param_id1 = 0,
  k_user_osc_param_id2,
  k_user_osc_param_id3,
  k_user_osc_param_id4,
  k_user_osc_param_id5,
  k_user_osc_param_id6,
  k_user_osc_param_shape,
  k_user_osc_param_shiftshape
};

static inline float osc_w0f_for_note (uint8_t note, uint8_t fine)
{
  const float midi = (float) note + ((float) fine / 256.0f);
  const float hz = 440.0f * powf (2.0f, (midi - 69.0f) / 12.0f);
  return 6.28318530717958647692f * hz / 48000.0f;
}

static inline int32_t f32_to_q31 (float x)
{
  if (x > 0.999999f)
    x = 0.999999f;

  if (x < -1.0f)
    x = -1.0f;

  return (int32_t) (x * 2147483647.0f);
}

static inline float q31_to_f32 (int32_t x)
{
  return (float) x * (1.0f / 2147483648.0f);
}

#ifndef param_val_to_f32
#define param_val_to_f32(v) ((float) (v) * (1.0f / 1023.0f))
#endif
#else
#include "userosc.h"
#include "utils/fixed_math.h"
#endif

namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = 6.28318530717958647692f;
constexpr float kSampleRate = 48000.0f;

struct Params
{
  float speed = 0.44f;
  float chaos = 0.58f;
  float length2 = 0.64f;
  float gravity = 0.54f;
  float drive = 0.36f;
  float damping = 0.0f;
  float shape = 0.25f;
  float shift_shape = 0.10f;
};

struct State
{
  float theta1 = 0.72f;
  float theta2 = -0.36f;
  float omega1 = 0.0f;
  float omega2 = 0.0f;
};

struct Deriv
{
  float dtheta1 = 0.0f;
  float dtheta2 = 0.0f;
  float domega1 = 0.0f;
  float domega2 = 0.0f;
};

Params g_params;
State g_state;
float g_drive_phase = 0.0f;
float g_last_y = 0.0f;
float g_dc_y1 = 0.0f;
float g_dc_x1 = 0.0f;
float g_output_gain = 0.72f;

static inline float clampf (float x, float low, float high)
{
  return x < low ? low : (x > high ? high : x);
}

static inline float wrap_pi (float x)
{
  while (x > kPi)
    x -= kTwoPi;

  while (x < -kPi)
    x += kTwoPi;

  return x;
}

static inline float soft_clip (float x)
{
  return tanhf (x);
}

static inline float pct_to_norm (uint16_t value)
{
  return clampf ((float) value * 0.01f, 0.0f, 1.0f);
}

Deriv eval_double_pendulum (const State& s, float drive)
{
  const float m1 = 1.0f;
  const float m2 = 1.0f;
  const float l1 = 1.0f;
  const float l2 = 0.30f + g_params.length2 * 1.25f;
  const float gravity = 2.2f + g_params.gravity * 24.0f;
  const float damping = g_params.damping * 0.042f;
  const float chaos = 0.12f + g_params.chaos * 1.45f;

  const float delta = s.theta1 - s.theta2;
  const float den = fmaxf (0.001f, 2.0f * m1 + m2 - m2 * cosf (2.0f * delta));

  float a1 = (-gravity * (2.0f * m1 + m2) * sinf (s.theta1)
            - m2 * gravity * sinf (s.theta1 - 2.0f * s.theta2)
            - 2.0f * sinf (delta) * m2
                * (s.omega2 * s.omega2 * l2 + s.omega1 * s.omega1 * l1 * cosf (delta)))
           / (l1 * den);

  float a2 = (2.0f * sinf (delta)
            * (s.omega1 * s.omega1 * l1 * (m1 + m2)
             + gravity * (m1 + m2) * cosf (s.theta1)
             + s.omega2 * s.omega2 * l2 * m2 * cosf (delta)))
           / (l2 * den);

  a1 += drive * chaos - damping * s.omega1;
  a2 += drive * chaos * 1.31f - damping * s.omega2;

  Deriv d;
  d.dtheta1 = s.omega1;
  d.dtheta2 = s.omega2;
  d.domega1 = a1;
  d.domega2 = a2;
  return d;
}

static inline State add_scaled (const State& s, const Deriv& d, float scale)
{
  State out;
  out.theta1 = s.theta1 + d.dtheta1 * scale;
  out.theta2 = s.theta2 + d.dtheta2 * scale;
  out.omega1 = s.omega1 + d.domega1 * scale;
  out.omega2 = s.omega2 + d.domega2 * scale;
  return out;
}

void integrate_rk2 (float dt, float drive)
{
  const Deriv k1 = eval_double_pendulum (g_state, drive);
  const State mid = add_scaled (g_state, k1, dt * 0.5f);
  const Deriv k2 = eval_double_pendulum (mid, drive);

  g_state.theta1 += k2.dtheta1 * dt;
  g_state.theta2 += k2.dtheta2 * dt;
  g_state.omega1 += k2.domega1 * dt;
  g_state.omega2 += k2.domega2 * dt;

  g_state.theta1 = wrap_pi (g_state.theta1);
  g_state.theta2 = wrap_pi (g_state.theta2);
  g_state.omega1 = clampf (g_state.omega1, -96.0f, 96.0f);
  g_state.omega2 = clampf (g_state.omega2, -96.0f, 96.0f);
}

float pendulum_y()
{
  const float l1 = 1.0f;
  const float l2 = 0.30f + g_params.length2 * 1.25f;
  const float y = l1 * cosf (g_state.theta1) + l2 * cosf (g_state.theta2);
  return (y / (l1 + l2)) * 2.0f - 1.0f;
}

float dc_block (float x)
{
  const float y = x - g_dc_x1 + 0.995f * g_dc_y1;
  g_dc_x1 = x;
  g_dc_y1 = y;
  return y;
}

void reset_state (uint16_t pitch)
{
  const float note_seed = (float) ((pitch >> 8) % 24) / 23.0f;
  g_state.theta1 = 0.42f + note_seed * 0.92f;
  g_state.theta2 = -0.28f - note_seed * 0.54f;
  g_state.omega1 = 0.0f;
  g_state.omega2 = 0.0f;
  g_drive_phase = 0.0f;
  g_last_y = pendulum_y();
  g_dc_y1 = 0.0f;
  g_dc_x1 = g_last_y;
}
}

extern "C" {

void OSC_INIT (uint32_t platform, uint32_t api)
{
  (void) platform;
  (void) api;
  reset_state (60u << 8);
}

void OSC_CYCLE (const user_osc_param_t* const params, int32_t* yn, const uint32_t frames)
{
  const uint16_t pitch = params != 0 ? params->pitch : (60u << 8);
  const float w0 = osc_w0f_for_note ((uint8_t) (pitch >> 8), (uint8_t) (pitch & 0xffu));
  const float lfo = params != 0 ? q31_to_f32 (params->shape_lfo) : 0.0f;
  const float shape = clampf (g_params.shape + lfo * 0.35f, 0.0f, 1.0f);
  const float shift_shape = g_params.shift_shape;

  const float speed = 0.18f + g_params.speed * 3.8f + shape * 2.2f;
  const float drive_amount = 0.015f + g_params.drive * 0.19f + shift_shape * 0.16f;
  const float dt = clampf ((w0 / kTwoPi) * speed, 0.00001f, 0.045f);
  const float drive_inc = w0 * (0.5f + g_params.chaos * 2.5f);

  for (uint32_t i = 0; i < frames; ++i)
  {
    g_drive_phase += drive_inc;

    if (g_drive_phase >= kTwoPi)
      g_drive_phase -= kTwoPi;

    const float drive = sinf (g_drive_phase) * drive_amount;
    integrate_rk2 (dt, drive);

    const float y = pendulum_y();
    const float wave = dc_block (y);
    const float mixed = 0.72f * wave + 0.28f * (y - g_last_y) * 9.0f;
    g_last_y = y;

    const float out = soft_clip (mixed * (1.0f + g_params.drive * 3.5f)) * g_output_gain;
    yn[i] = f32_to_q31 (out);
  }
}

void OSC_NOTEON (const user_osc_param_t* const params)
{
  reset_state (params != 0 ? params->pitch : (60u << 8));
}

void OSC_NOTEOFF (const user_osc_param_t* const params)
{
  (void) params;
}

void OSC_PARAM (uint16_t index, uint16_t value)
{
  switch (index)
  {
    case k_user_osc_param_id1:
      g_params.speed = pct_to_norm (value);
      break;

    case k_user_osc_param_id2:
      g_params.chaos = pct_to_norm (value);
      break;

    case k_user_osc_param_id3:
      g_params.length2 = pct_to_norm (value);
      break;

    case k_user_osc_param_id4:
      g_params.gravity = pct_to_norm (value);
      break;

    case k_user_osc_param_id5:
      g_params.drive = pct_to_norm (value);
      break;

    case k_user_osc_param_id6:
      g_params.damping = pct_to_norm (value);
      break;

    case k_user_osc_param_shape:
      g_params.shape = param_val_to_f32 (value);
      break;

    case k_user_osc_param_shiftshape:
      g_params.shift_shape = param_val_to_f32 (value);
      break;

    default:
      break;
  }
}

}

#if defined(PENDY_HOST_TEST_MAIN)
#include <stdio.h>

int main()
{
  user_osc_param_t params;
  params.pitch = 48u << 8;
  params.shape_lfo = 0;

  OSC_INIT (0, 0);
  OSC_PARAM (0, 52);
  OSC_PARAM (1, 72);
  OSC_PARAM (2, 62);
  OSC_PARAM (3, 48);
  OSC_PARAM (4, 42);
  OSC_PARAM (5, 0);
  OSC_PARAM (k_user_osc_param_shape, 256);
  OSC_PARAM (k_user_osc_param_shiftshape, 96);
  OSC_NOTEON (&params);

  int32_t buffer[64];
  int32_t peak = 0;

  for (int block = 0; block < 240; ++block)
  {
    OSC_CYCLE (&params, buffer, 64);

    for (int i = 0; i < 64; ++i)
    {
      const int32_t abs_sample = buffer[i] < 0 ? -buffer[i] : buffer[i];

      if (abs_sample > peak)
        peak = abs_sample;
    }
  }

  printf ("Pendy host peak q31: %ld\n", (long) peak);
  return peak > 1000000 ? 0 : 1;
}
#endif
