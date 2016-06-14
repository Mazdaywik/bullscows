#include <algorithm>
#include <functional>
#include <map>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "number.h"

class Match {
public:
  Match(Number x, Number y)
    : bulls(0), cows(0)
  {
    for (size_t i = 0; i < NUMLEN; ++i) {
      bulls += (x[i] == y[i]);
    }

    for (size_t i = 0; i < NUMLEN; ++i) {
      for (size_t j = 0; j < NUMLEN; ++j) {
        cows += (x[i] == y[j] && i != j);
      }
    }
  }

  Match(int bulls, int cows)
    : bulls(bulls), cows(cows)
  {
    /* пусто */
  }

  Match()
    : bulls(0), cows(0)
  {
    /* пусто */
  }

  bool operator<(const Match& rhs) const {
    return (bulls < rhs.bulls) || ((bulls == rhs.bulls) && (cows < rhs.cows));
  }

  bool operator==(const Match& rhs) const {
    return (bulls == rhs.bulls) && (cows == rhs.cows);
  }

  const char *str(char buffer[]) const {
    sprintf(buffer, "%dA%dB", bulls, cows);
    return buffer;
  }

private:
  int bulls, cows;
};

struct Answer {
  Number number;
  Match match;

  Answer(Number number, Match match)
    : number(number), match(match)
  {
    /* пусто */
  }

  void print() {
    char num_buffer[NUMLEN + 1];
    char match_buffer[10];
    printf("%s<->%s", number.str(num_buffer), match.str(match_buffer));
  }

  typedef Number argument_type;
  typedef bool result_type;

  result_type operator()(argument_type num) const {
    return Match(this->number, num) == this->match;
  }
};

typedef std::map<Match, Numbers> Classes;
typedef std::map<Match, int> ClassCounts;

double log2(double x) {
  return log(x) / log(2);
}

class SilentCalcInfo {
public:
  void add_match(Match, Number) { /* пусто */ }
  void print(Number) { /* пусто */ }
  void print(Match, int) { /* пусто */ }
  void print_info(double) { /* пусто */ }
};

class PrintableCalcInfo {
public:
  void add_match(Match m, Number n) {
    if (m_classes[m].size() < 10) {
      m_classes[m].push_back(n);
    }
  }

  void print(Number n) {
    char num_buf[NUMLEN + 1];
    printf("Number: %s\n", n.str(num_buf));
  }

  void print(Match m, int count) {
    char match_buf[10];
    printf("%s | %4d | ", m.str(match_buf), count);
    const Numbers& class_ = m_classes[m];
    for (size_t i = 0; i < class_.size(); ++i) {
      char num_buf[NUMLEN + 1];
      printf("%s%s", (i == 0 ? "" : ", "), class_[i].str(num_buf));
    }
    puts(class_.size() < (size_t) count ? "..." : "");
  }

  void print_info(double info) {
    printf("Information: %.2f bits\n", info);
  }

private:
  Classes m_classes;
};

template <typename CalcInfoPrinter>
double calc_info(Number sample, const Numbers& candidates) {
  ClassCounts class_counts;
  CalcInfoPrinter pr;

  for (
    Numbers::const_iterator p = candidates.begin();
    p != candidates.end();
    ++p
  ) {
    Match match(sample, *p);
    pr.add_match(match, *p);
    ++class_counts[match];
  }

  pr.print(sample);
  double info = 0;
  const double total = candidates.size();
  for (
    ClassCounts::const_iterator p = class_counts.begin();
    p != class_counts.end();
    ++p
  ) {
    pr.print(p->first, p->second);
    info -= (p->second / total) * log2(p->second / total);
  }
  pr.print_info(info);

  return info;
}

Number max_info_number(const Numbers& candidates, const Numbers& tries) {
  Number result = candidates.at(0);
  if (candidates.size() > 1) {
    double max_info = calc_info<SilentCalcInfo>(result, candidates);

    const Numbers *v = &tries;
    if (candidates.size() < tries.size()) {
      v = &candidates;
    }

    while (v != 0) {
      for (Numbers::const_iterator p = v->begin(); p != v->end(); ++p) {
        double next_info = calc_info<SilentCalcInfo>(*p, candidates);
        if (max_info < next_info) {
          max_info = next_info;
          result = *p;
        }
      }

      v = (v == &tries ? 0 : &tries);
    }
  }
  calc_info<PrintableCalcInfo>(result, candidates);
  return result;
}

Number get_next_number(const Numbers& candidates, const Numbers& tries) {
    printf("numbers = %u\n", (unsigned) candidates.size());
    return max_info_number(candidates, tries);
}

void sieve(Numbers& candidates, Answer ans) {
  size_t before = candidates.size();
  candidates.erase(
    std::remove_if(candidates.begin(), candidates.end(), std::not1(ans)),
    candidates.end()
  );
  double after = candidates.size();
  printf(
    "before %lu, after = %.0f, ratio = %.2f, info = %.2f bits\n",
    (unsigned long) before, after, before/after, log2(before/after)
  );
}

int main() {
  try {
    bool do_exit = false;

    Number next = Number();
    char num_buf[NUMLEN + 1] = { '\0' };
    Numbers candidates = Number::all_numbers();
    Numbers tries = Number::all_numbers();

    while (! do_exit) {
      printf("\nDefault number %s:\n", next.str(num_buf));

      const char MAXLINE = 80;
      char line[MAXLINE+1] = { '\0' };
      int bulls = 0, cows = 0, next_num_num;

      bool perform_sieve = false;

      if (! fgets(line, MAXLINE, stdin)) {
        do_exit = true;
      } else if (sscanf(line, "? %d", &next_num_num) == 1) {
        try {
          next = Number(next_num_num);
          calc_info<PrintableCalcInfo>(next, candidates);
        } catch (BadNumberError) {
          printf("Invalid number: digits is repeated\n");
        }
      } else if (strncmp(line, "cheat", 5) == 0) {
        next = get_next_number(candidates, tries);
      } else if (sscanf(line, "%d %dA%dB", &next_num_num, &bulls, &cows) == 3) {
        try {
          next = Number(next_num_num);
          calc_info<PrintableCalcInfo>(next, candidates);
          perform_sieve = true;
        } catch (BadNumberError) {
          printf("Invalid number: digits is repeated\n");
        }
      } else if (sscanf(line, "%dA%dB", &bulls, &cows) == 2) {
        perform_sieve = true;
      } else if (strncmp(line, "reset", 5) == 0) {
        candidates = Number::all_numbers();
        next = Number();
      } else if (strncmp(line, "exit", 4) == 0) {
        do_exit = true;
      } else {
        puts(
          "Print command:\n"
          "  ? number - query stats for number\n"
          "  cheat - find better number\n"
          "  number xAyB - for the number x bulls and y cows\n"
          "  xAyB - for default number x bulls and y cows\n"
          "  reset - reset game\n"
          "  exit - exit\n"
        );
      }

      if (perform_sieve) {
        Numbers backup = candidates;
        sieve(candidates, Answer(next, Match(bulls, cows)));
        switch (candidates.size()) {
          case 0:
            printf("WARNING: empty candidates list, unroll sieve\n");
            candidates.swap(backup);
            break;

          case 1:
            printf(
              "Thinked number is %s, reset the game.\n",
              candidates[0].str(num_buf)
            );
            candidates = Number::all_numbers();
            next = Number();
            break;

          default:
            /* ничего не делаем */;
            break;
        }
        perform_sieve = false;
      }
    }
  } catch (BadNumberError) {
    printf("uncatched BadNumberError\n");
  } catch (BadIndexError) {
    printf("uncatched BadIndexError\n");
  } catch (...) {
    printf("uncatched unknown error\n");
  }
  return 0;
}
