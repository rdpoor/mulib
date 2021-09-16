#include <stdint.h>
#include <stdio.h>

#define UNITS_PER_PIXEL 16
#define FXP(p, u) ((p)*UNITS_PER_PIXEL + (u))

typedef int16_t distance_t;
typedef int16_t position_t;
typedef int16_t velocity_t;

typedef enum {
  COLLISION_NONE,
  COLLISION_WALL_TOP,
  COLLISION_WALL_BOTTOM,
  COLLISION_WALL_LEFT,
  COLLISION_WALL_RIGHT,
  COLLISION_PADDLE_LEFT,
  COLLISION_PADDLE_RIGHT,
  GRAZE_PADDLE_LEFT,
  GRAZE_PADDLE_RIGHT,
} collision_t;

typedef struct {
  distance_t radius;
  position_t x;
  position_t y;
  velocity_t dxdt;
  velocity_t dydt;
} ball_t;

typedef struct {
  position_t xlo;
  position_t ylo;
  position_t xhi;
  position_t yhi;
} arena_t;

typedef struct {
  position_t x;
  position_t yhi;
  position_t ylo;
} paddle_t;

position_t reflect(position_t p, position_t wall) {
  return 2 * wall - p;
}

collision_t ball_step(ball_t *b, arena_t *a, paddle_t *pl, paddle_t *pr) {
  collision_t collision = COLLISION_NONE;
  position_t bx = b->x + b->dxdt;
  position_t by = b->y + b->dydt;

  if (bx + b->radius >= a->xhi) {
    // reflect off right wall
    b->dxdt = -b->dxdt;
    bx = reflect(bx, a->xhi);
    collision = COLLISION_WALL_RIGHT;
  } else if (bx - b->radius <= a->xlo) {
    // reflect off left wall
    b->dxdt = -b->dxdt;
    bx = reflect(bx, a->xlo);
    collision = COLLISION_WALL_LEFT;
  }

  if (by + b->radius >= a->yhi) {
    // reflect off top wall
    b->dydt = -b->dydt;
    by = reflect(by, a->yhi);
    collision = COLLISION_WALL_TOP;
  } else if (by - b->radius <= a->ylo) {
    // reflect off bottom wall
    b->dydt = -b->dydt;
    by = reflect(by, a->ylo);
    collision = COLLISION_WALL_BOTTOM;
  }

  if ((by + b->radius >= pl->ylo) && (by - b->radius <= pl->yhi)) {
    // ball is same height as left paddle.  Did it cross the paddle?
    if (((bx + b->radius >= pl->x) && (b->x < pl->x)) ||
        ((bx - b->radius <= pl->x) && (b->x > pl->x))) {
      b->dxdt = -b->dxdt;
      bx = reflect(bx, pl->x);
      if ((by < pl->ylo) || (by > pl->yhi)) {
        collision = GRAZE_PADDLE_LEFT;
      } else {
        collision = COLLISION_PADDLE_LEFT;
      }
    }
  }

  if ((by + b->radius >= pr->ylo) && (by - b->radius <= pr->yhi)) {
    // ball is same height as right paddle.  Did it cross the paddle?
    if (((bx + b->radius >= pr->x) && (b->x < pr->x)) ||
        ((bx - b->radius <= pr->x) && (b->x > pr->x))) {
      b->dxdt = -b->dxdt;
      bx = reflect(bx, pl->x);
      if ((by < pl->ylo) || (by > pl->yhi)) {
        collision = GRAZE_PADDLE_RIGHT;
      } else {
        collision = COLLISION_PADDLE_RIGHT;
      }
    }
  }

  // update ball
  b->x = bx;
  b->y = by;
  return collision;
}

void paddle_move(paddle_t *p, position_t dy, arena_t *a) {
  position_t plo = p->ylo + dy;
  position_t phi = p->yhi + dy;

  if (((dy > 0) && (phi < a->yhi)) || ((dy < 0) && (plo > a->ylo))) {
    p->yhi = phi;
    p->ylo = plo;
  }
}

void print_arena(arena_t *a) {
  printf("arena: xlo=%d, ylo=%d, xhi=%d, yhi=%d\n", a->xlo, a->ylo, a->xhi, a->yhi);
}

void print_paddle(paddle_t *p, const char *name) {
  printf("paddle %s: x=%d, ylo=%d, yhi=%d\n", name, p->x, p->ylo, p->yhi);
}

void print_ball(ball_t *b, collision_t c) {
  const char *s;
  switch(c) {
    case COLLISION_NONE: s = "COLLISION_NONE"; break;
    case COLLISION_WALL_TOP: s = "COLLISION_WALL_TOP"; break;
    case COLLISION_WALL_BOTTOM: s = "COLLISION_WALL_BOTTOM"; break;
    case COLLISION_WALL_LEFT: s = "COLLISION_WALL_LEFT"; break;
    case COLLISION_WALL_RIGHT: s = "COLLISION_WALL_RIGHT"; break;
    case COLLISION_PADDLE_LEFT: s = "COLLISION_PADDLE_LEFT"; break;
    case COLLISION_PADDLE_RIGHT: s = "COLLISION_PADDLE_RIGHT"; break;
    case GRAZE_PADDLE_LEFT: s = "GRAZE_PADDLE_LEFT"; break;
    case GRAZE_PADDLE_RIGHT: s = "GRAZE_PADDLE_RIGHT"; break;
    default: s = "COLLISION_UNKNOWN"; break;
  }
  printf("b: x=%d, y=%d, dx=%d, dy=%d: %s\n", b->x, b->y, b->dxdt, b->dydt, s);
}

#define ARENA_WIDTH 61
#define ARENA_HEIGHT 37
#define BALL_RADIUS 5
#define PADDLE_HEIGHT 10

// cc -g -Wall -o physics physics.c

int main() {
  ball_t ball = (ball_t){.radius=FXP(0,0),
                          .x=FXP(ARENA_WIDTH/2, 0),
                          .y=FXP(ARENA_HEIGHT/2, 0),
                          .dxdt=FXP(0,5),
                          .dydt=FXP(0,3)};
  arena_t arena = (arena_t){.xlo=FXP(0,0),
                            .ylo=FXP(0,0),
                            .xhi=FXP(ARENA_WIDTH, 0),
                            .yhi=FXP(ARENA_HEIGHT, 0)};
  paddle_t left = (paddle_t){.x=FXP(ARENA_WIDTH*1/5, 0),
                             .ylo=FXP((ARENA_HEIGHT-PADDLE_HEIGHT)/2, 0),
                             .yhi=FXP((ARENA_HEIGHT+PADDLE_HEIGHT)/2, 0)};
  paddle_t right = (paddle_t){.x=FXP(ARENA_WIDTH*4/5, 0),
                              .ylo=FXP((ARENA_HEIGHT-PADDLE_HEIGHT)/2, 0),
                              .yhi=FXP((ARENA_HEIGHT+PADDLE_HEIGHT)/2, 0)};

  print_arena(&arena);
  print_paddle(&left, "left");
  print_paddle(&right, "right");

  collision_t c = COLLISION_NONE;
  while(1) {
    do {
      c = ball_step(&ball, &arena, &left, &right);
    } while (c == COLLISION_NONE);
    print_ball(&ball, c);
    getchar();
  }

  return 0;
}
