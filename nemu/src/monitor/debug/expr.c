#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256,
  TK_EQ,
  TK_NUM,
  TK_PLUS,
  TK_MINUS,
  TK_MULTIPLY,
  TK_DIV,
  TK_LEFT,
  TK_RIGHT,

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", TK_PLUS},     // plus
  {"\\(", TK_LEFT},
  {"\\*", TK_MULTIPLY},
  {"/", TK_DIV},
  {"-", TK_MINUS},
  {"==", TK_EQ},        // equal
  {"[1-9][0-9]*|0", TK_NUM},
  {"\\)", TK_RIGHT},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;
          case TK_NUM:
            tokens[nr_token].type = TK_NUM;
            strcpy(tokens[nr_token].str, substr_start);
            nr_token++;
            break;
          default:
            tokens[nr_token++].type = rules[i].token_type;
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


bool check_match(int p, int q) {
  int unmatched = 0;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == TK_LEFT)
      unmatched++;
    else if (tokens[i].type == TK_RIGHT)
      unmatched--;
    if (unmatched < 0)
      return false;
  }
  if (!unmatched)
    return true;
  else
    return false;
}

bool check_parentheses(int p, int q, bool* success) {
  if (tokens[p].type == TK_LEFT && tokens[q].type == TK_RIGHT) {
    if (check_match(p, q)) {
      if (check_match(p + 1, q - 1))
        return true;
    } else {
      *success = false;
    }
  }
  return false;
}

int get_priority(int token) {
  switch (token) {
    case TK_PLUS:
    case TK_MINUS:
      return 0;
    case TK_MULTIPLY:
    case TK_DIV:
      return 1;
    default:
      return -1;
  }
}

int find_main_op(int p, int q) {
  int min_index = -1, min_priority = 10;
  for (int i = p; i <= q; i++) {
    if (tokens[i].type == TK_LEFT) {
      while (tokens[i].type != TK_RIGHT)
        i++;
      continue;
    }
    int priority = get_priority(tokens[i].type);
    if (priority >= 0 && priority <= min_priority) {
      min_index = i;
      min_priority = priority;
    }
  }
  return min_index;
}

uint32_t eval(int p, int q, bool* success) {
  if (p > q) {
    *success = false;
    return 0;
  } else if (p == q) {
    if (tokens[p].type == TK_NUM) {
      int num;
      sscanf(tokens[p].str, "%u", &num);
      return num;
    }
    *success = false;
    return 0;
  } else if (check_parentheses(p, q, success) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
      * If that is the case, just throw away the parentheses.
      */
    return eval(p + 1, q - 1, success);
  } else {
    /* We should do more things here. */
    if (!*success)
      return 0;
    int op = find_main_op(p, q);
    uint32_t val1 = eval(p, op - 1, success);
    uint32_t val2 = eval(op + 1, q, success);
    if (op == q) {
      *success = false;
      return 0;
    } else {
      switch (tokens[op].type) {
        case TK_PLUS:
          return val1 + val2;
        case TK_MINUS:
          return val1 - val2;
        case TK_MULTIPLY:
          return val1 * val2;
        case TK_DIV:
          if (val2 == 0) {
            return 0;
          }
          return val1 / val2;
        default:
          assert(0);
      }
    }
  }
  return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  return eval(0, nr_token - 1, success);

  return 0;
}