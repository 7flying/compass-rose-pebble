#ifndef COMPASS_ROSE_H
#define COMPASS_ROSE_H

typedef struct {
  int days;
  int hours;
  int minutes;
  int seconds;
} Time;

#define MARGIN 5
#define THICKNESS 3
#define ANIMATION_DELAY 300
#define ANIMATION_DURATION 1000
#define HAND_LENGTH_MIN 80
#define HAND_LENGTH_HOUR (HAND_LENGTH_MIN - 20)

#endif // COMPASS_ROSE_H
