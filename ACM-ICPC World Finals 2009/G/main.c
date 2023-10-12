/**
 * G. House of Cards
 */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define M 13

#define MAX_RESERVED_STATES (M * 2 * 4)

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

typedef struct state {
  int cards[8];
  int holds[2];
  int score;
  struct state *next;
} state_t;

state_t reserved_states[MAX_RESERVED_STATES];

state_t *free_list;

void init_states(void) {
  free_list = reserved_states;
  for (int i = 1; i < MAX_RESERVED_STATES; i++) {
    reserved_states[i - 1].next = reserved_states + i;
  }
  reserved_states[MAX_RESERVED_STATES - 1].next = NULL;
}

int compute_score(int a, int b, int c) {
  int x = 0;

  if (a < 0) {
    a = -a;
    x++;
  }

  if (b < 0) {
    b = -b;
    x++;
  }

  if (c < 0) {
    c = -c;
    x++;
  }

  return (a + b + c) * (x <= 1 ? 1 : -1);
}

state_t *state_alloc(void) {
  assert(free_list != NULL);
  state_t *state = free_list;
  free_list = state->next;
  state->next = NULL;
  return state;
}

void state_free(state_t *state) {
  state->next = free_list;
  free_list = state;
}

state_t *state_transition_to(const state_t *from, int player, int i, int a, int b, int hold) {
  state_t *to = state_alloc();

  memcpy(to->cards, from->cards, sizeof(int) * 8);
  memcpy(to->holds, from->holds, sizeof(int) * 2);

  if (i > 0) {
    if (a == b) {
      to->cards[i] = a << 2;
      to->cards[i + 1] = b << 2;
      to->score = from->score + compute_score(a, from->cards[i + 1] >> 2, from->cards[i] >> 2);
    } else {
      to->cards[i] = (a << 2) | 1;
      to->cards[i + 1] = (b << 2) | 2;
      to->score = from->score + compute_score(from->cards[i] >> 2, a, b);
    }
  } else {
    to->score = from->score;
  }

  to->holds[player] = hold;

  return to;
}

state_t *state_expand(state_t *from, int player, int deck[], int k) {
  state_t *state;
  state_t *next_states = NULL;

  if (from->holds[player] == 0) {
    state = state_transition_to(from, player, -1, 0, 0, deck[k - 1]);
    state->next = next_states;
    next_states = state;
  }

  for (int i = 1; i < 7; i++) {
    if (from->cards[i] == from->cards[i + 1]) {
      if (from->holds[player] != 0) {
        state = state_transition_to(from, player, i, from->holds[player], deck[k - 1], 0);
        state->next = next_states;
        next_states = state;

        state = state_transition_to(from, player, i, deck[k - 1], from->holds[player], 0);
        state->next = next_states;
        next_states = state;
      }
    } else if ((from->cards[i] & 3) == 2 && (from->cards[i + 1] & 3) == 1) {
      if (from->holds[player] != 0) {
        state = state_transition_to(from, player, i, from->holds[player], from->holds[player], deck[k - 1]);
        state->next = next_states;
        next_states = state;
      }

      state = state_transition_to(from, player, i, deck[k - 1], deck[k - 1], from->holds[player]);
      state->next = next_states;
      next_states = state;
    }
  }

  return next_states;
}

int minimax_search_internal(state_t *state, int player, int deck[], int k, int alpha, int beta) {  
  if (k == 0) {
    return state->score + state->holds[0] + state->holds[1];
  }

  state_t *next_states = state_expand(state, player, deck, k);

  for (state_t *iter = next_states; iter != NULL; iter = iter->next) {
    int score = minimax_search_internal(iter, player ^ 1, deck, k - 1, alpha, beta);

    if (player == 0) {
      alpha = MAX(score, alpha);
    } else {
      beta  = MIN(score, beta);
    }

    /* alpha-beta pruning */
    if (beta <= alpha) {
      break;
    }
  }

  for (state_t *curr = next_states, *next; curr != NULL; curr = next) {
    next = curr->next;
    state_free(curr);
  }

  return player == 0 ? alpha : beta;
}

int minimax_search(int deck[], int m, int first_player) {
  init_states();

  state_t *state = state_alloc();

  for (int i = 0; i < 8; i++) {
    state->cards[i] = (deck[m - i - 1] << 2) | ((i & 1) + 1);
  }

  state->holds[0] = 0;
  state->holds[1] = 0;
  state->score = 0;

  int score = minimax_search_internal(state, first_player, deck, m - 8, INT_MIN, INT_MAX);

  state_free(state);

  return score;
}

int main(void) {
  int t = 0;

  char name[7];
  char card[4];

  int m;
  int deck[M * 2];

  while (1) {
    scanf("%s", name);
    if (name[0] == 'E') {
      break;
    }

    scanf("%d", &m);
    m *= 2;

    for (int i = m - 1; i >= 0; i--) {
      scanf("%s", card);

      int card_id;
      sscanf(card, "%d", &card_id);
      if (card[strlen(card) - 1] == 'B') {
        card_id = -card_id;
      }

      deck[i] = card_id;
    }

    int score = minimax_search(deck, m, deck[m - 1] > 0 ? 0 : 1);

    if (score > 0) {
      if (name[0] == 'A') {
        printf("Case %d: Axel wins %d\n", ++t, score);
      } else {
        printf("Case %d: Birgit loses %d\n", ++t, score);
      }
    } else if (score < 0) {
      if (name[0] == 'A') {
        printf("Case %d: Axel loses %d\n", ++t, -score);
      } else {
        printf("Case %d: Birgit wins %d\n", ++t, -score);
      }
    } else {
      printf("Case %d: Axel and Birgit tie\n", ++t);
    }
  }

  return 0;
}
