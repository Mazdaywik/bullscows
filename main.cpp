#include <algorithm>
#include <functional>
#include <map>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

class BadNumberError {};
class BadIndexError {};

const size_t NUMLEN = 4;

class Number;
typedef std::vector<Number> Numbers;

class Number {
public:
  explicit Number(const char *num) {
    init(num);
  }

  explicit Number(unsigned int value) {
    char numbuffer[25];
    char nummaskformat[10];
    sprintf(nummaskformat, "%%0%dd", NUMLEN);
    sprintf(numbuffer, nummaskformat, value);
    init(numbuffer);
  }

  Number() {
    char num[NUMLEN + 1];
    for (size_t i = 0; i < NUMLEN; ++i) {
      num[i] = '0' + i;
    }
    num[NUMLEN] = '\0';
    init(num);
  }

  static Number get_random_number() {
    for ( ; ; ) {
      try {
        return Number(rand() % div());
      } catch (BadNumberError) {
        /* ничего не делаем */
      }
    }
  }

  static const Numbers& all_numbers() {
    static Numbers s_all_numbers;

    if (s_all_numbers.empty()) {
      const unsigned int total = div();
      s_all_numbers.reserve(total);

      for (unsigned int i = 0; i < total; ++i) {
        try {
          s_all_numbers.push_back(Number(i));
        } catch (BadNumberError) {
          /* ничего не делаем */
        }
      }

      Numbers(s_all_numbers).swap(s_all_numbers);
    }

    return s_all_numbers;
  }

  char operator[](size_t i) {
    return i < NUMLEN ? m_digits[i] : throw BadIndexError();
  }

  bool operator<(const Number& rhs) const {
    size_t i = 0;
    while (i < NUMLEN && m_digits[i] == rhs.m_digits[i]) {
      ++i;
    }
    return (i < NUMLEN) && (m_digits[i] < rhs.m_digits[i]);
  }

  const char *str(char (&buffer)[NUMLEN + 1]) const {
    for (size_t i = 0; i < NUMLEN; ++i) {
      buffer[i] = m_digits[i];
    }
    buffer[NUMLEN] = '\0';
    return buffer;
  }

private:
  void init(const char *num) {
    if (strlen(num) != NUMLEN) {
      throw BadNumberError();
    }

    for (size_t i = 0; i < NUMLEN; ++i) {
      char digit = num[i];
      for (size_t j = i + 1; j < NUMLEN; ++j) {
        if (num[j] == digit) {
          throw BadNumberError();
        }
      }
      m_digits[i] = digit;
    }
  }

  static unsigned int div() {
    unsigned int result = 1;
    for (size_t i = 0; i < NUMLEN; ++i) {
      result *= 10;
    }
    return result;
  }

  char m_digits[NUMLEN];
};

class Match {
public:
  Match(Number x, Number y)
    : oxes(0), cows(0)
  {
    for (size_t i = 0; i < NUMLEN; ++i) {
      oxes += (x[i] == y[i]);
    }

    for (size_t i = 0; i < NUMLEN; ++i) {
      for (size_t j = 0; j < NUMLEN; ++j) {
        cows += (x[i] == y[j] && i != j);
      }
    }
  }

  Match(int oxes, int cows)
    : oxes(oxes), cows(cows)
  {
    /* пусто */
  }

  Match()
    : oxes(0), cows(0)
  {
    /* пусто */
  }

  bool operator<(const Match& rhs) const {
    return (oxes < rhs.oxes) || ((oxes == rhs.oxes) && (cows < rhs.cows));
  }

  bool operator==(const Match& rhs) const {
    return (oxes == rhs.oxes) && (cows == rhs.cows);
  }

  const char *str(char buffer[]) const {
    sprintf(buffer, "%dA%dB", oxes, cows);
    return buffer;
  }

private:
  int oxes, cows;
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

      if (v != &tries) {
        v = &tries;
      } else {
        v = 0;
      }
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
      int oxes = 0, cows = 0, next_num_num;

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
      } else if (sscanf(line, "%d %dA%dB", &next_num_num, &oxes, &cows) == 3) {
        try {
          next = Number(next_num_num);
          calc_info<PrintableCalcInfo>(next, candidates);
          perform_sieve = true;
        } catch (BadNumberError) {
          printf("Invalid number: digits is repeated\n");
        }
      } else if (sscanf(line, "%dA%dB", &oxes, &cows) == 2) {
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
        sieve(candidates, Answer(next, Match(oxes, cows)));
        switch (candidates.size()) {
          case 0:
            printf("WARNING: empty candidates list, unroll sieve\n");
            candidates.swap(backup);
            break;

          case 1:
            printf("Only one number");
            next = candidates[0];
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
